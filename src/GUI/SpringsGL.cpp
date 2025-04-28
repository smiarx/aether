#include "SpringsGL.h"

#include <memory>

#include "../PluginProcessor.h"
#include "Assets.h"
#include "SpringsSection.h"
#include "juce_core/juce_core.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_opengl/juce_opengl.h"
#include "juce_opengl/opengl/juce_gl.h"

namespace aether
{

static constexpr auto kRefreshTimeMs = 17;

constexpr PluginProcessor::ParamId kListenIds[]{
    PluginProcessor::ParamId::kSpringsDecay,
    PluginProcessor::ParamId::kSpringsDamp,
    PluginProcessor::ParamId::kSpringsShape,
};

static juce::String glslColour(juce::Colour colour)
{
    float r = colour.getFloatRed(), g = colour.getFloatGreen(),
          b           = colour.getFloatBlue();
    juce::String glsl = "vec3(" + juce::String(r) + "," + juce::String(g) +
                        "," + juce::String(b) + ")";
    return glsl;
}

SpringsGL::SpringsGL(PluginProcessor &processor) :
    processor_(processor), rms_(processor.getRMSStack()),
    rmspos_(processor.getRMSStackPos()), shake_(processor.getShakeAtomic())
{
    setOpaque(true);
    // Sets the OpenGL version to 3.2
    openGlContext_.setOpenGLVersionRequired(
        juce::OpenGLContext::OpenGLVersion::openGL3_2);

    // Attach the OpenGL context but do not start [ see start() ]
    openGlContext_.setComponentPaintingEnabled(true);
    openGlContext_.setRenderer(this);
    openGlContext_.attachTo(*this);
    openGlContext_.setContinuousRepainting(false);

    // add springgl to listeners
    auto &params = processor.getParameters();
    for (auto id : kListenIds) {
        int iid     = static_cast<int>(id);
        auto &param = params.getReference(iid);
        param->addListener(this);
        parameterValueChanged(iid, param->getValue());
    }
}

SpringsGL::~SpringsGL()
{
    // Turn off OpenGL
    openGlContext_.detach();

    // remove springgl from listeners
    auto &params = processor_.getParameters();
    for (auto id : kListenIds) {
        int iid     = static_cast<int>(id);
        auto &param = params.getReference(iid);
        param->removeListener(this);
    }
}

void SpringsGL::timerCallback()
{
    openGlContext_.triggerRepaint();
    time_ += 0.001f * kRefreshTimeMs;
}

void SpringsGL::newOpenGLContextCreated()
{
    // Setup Buffer Objects
    juce::gl::glGenBuffers(1, &vbo_); // Vertex Buffer Object
    juce::gl::glGenBuffers(1, &ebo_); // Element Buffer Object
                                      //
    // Define Vertices for a Square (the view plane)
    constexpr GLfloat kVertices[] = {
        1.0f,  1.0f,  // Top Right
        1.0f,  -1.0f, // Bottom Right
        -1.0f, -1.0f, // Bottom Left
        -1.0f, 1.0f,  // Top Left
    };
    // Define Which Vertex Indexes Make the Square
    constexpr GLuint kIndices[] = {
        // Note that we start from 0!
        0, 1, 3, // First Triangle
        1, 2, 3  // Second Triangle
    };

    // VBO (Vertex Buffer Object) - Bind and Write to Buffer
    juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo_);
    juce::gl::glBufferData(juce::gl::GL_ARRAY_BUFFER, sizeof(kVertices),
                           kVertices, juce::gl::GL_STATIC_DRAW);

    // EBO (Element Buffer Object) - Bind and Write to Buffer
    juce::gl::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ebo_);
    juce::gl::glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices),
                           kIndices, juce::gl::GL_STATIC_DRAW);

    juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
    juce::gl::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);

    // Setup Shaders
    createShaders();

    startTimer(kRefreshTimeMs);
}

void SpringsGL::openGLContextClosing()
{
    shader_.reset();
    uniforms_.reset();
}

void SpringsGL::renderOpenGL()
{
    jassert(juce::OpenGLHelpers::isContextActive());

    // Setup Viewport
    const auto renderingScale = (float)openGlContext_.getRenderingScale();
    juce::gl::glViewport(0, 0, juce::roundToInt(renderingScale * getWidth()),
                         juce::roundToInt(renderingScale * getHeight()));

    // Set background Color
    juce::OpenGLHelpers::clear(
        getLookAndFeel().findColour(SpringsSection::kBackgroundColourId));

    // Enable Alpha Blending
    // juce::gl::glEnable(juce::gl::GL_BLEND);
    // juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA,
    //                      juce::gl::GL_ONE_MINUS_SRC_ALPHA);

    // Use Shader Program that's been defined
    if (shader_ != nullptr) shader_->use();

    // Setup the Uniforms for use in the Shader

    if (uniforms_ != nullptr) {
        if (uniforms_->resolution != nullptr)
            uniforms_->resolution->set((GLfloat)renderingScale * getWidth(),
                                       (GLfloat)renderingScale * getHeight());

        if (uniforms_->rms != nullptr)
            uniforms_->rms->set((GLfloat *)&rms_[0][0], kRmsStackSize * kN);

        if (uniforms_->rmspos != nullptr)
            uniforms_->rmspos->set((GLint)*rmspos_);

        if (uniforms_->coils != nullptr)
            uniforms_->coils->set((GLfloat *)&coils_, 1);
        if (uniforms_->radius != nullptr)
            uniforms_->radius->set((GLfloat *)&radius_, 1);
        if (uniforms_->shape != nullptr)
            uniforms_->shape->set((GLfloat *)&shape_, 1);
        if (uniforms_->time != nullptr) {
            uniforms_->time->set((GLfloat *)&time_, 1);
        }
    }

    juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo_);
    juce::gl::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // Setup Vertex Attributes
    juce::gl::glVertexAttribPointer(0, 2, juce::gl::GL_FLOAT,
                                    juce::gl::GL_FALSE, 2 * sizeof(GLfloat),
                                    (GLvoid *)nullptr);
    juce::gl::glEnableVertexAttribArray(0);

    // Draw Vertices
    juce::gl::glDrawElements(
        juce::gl::GL_TRIANGLES, 6, juce::gl::GL_UNSIGNED_INT,
        nullptr); // For EBO's (Element Buffer Objects) (Indices)

    // Reset the element buffers so child Components draw correctly
    juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
    juce::gl::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SpringsGL::createShaders()
{
    constexpr char kVertexShader[] =
        "attribute vec2 position;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgramAttempt =
        std::make_unique<juce::OpenGLShaderProgram>(openGlContext_);

    auto springBackgroundColour =
        findColour(SpringsSection::kBackgroundColourId);

    // Sets up pipeline of shaders and compiles the program
    if (shaderProgramAttempt->addVertexShader(
            juce::OpenGLHelpers::translateVertexShaderToV3(kVertexShader)) &&
        shaderProgramAttempt->addFragmentShader(
            juce::OpenGLHelpers::translateFragmentShaderToV3(
                "\n#define RMS_BUFFER_SIZE " + juce::String(kRmsStackSize) +
                "\n#define N " + juce::String(kN) +
                "\n#define BACKGROUND_COLOR " +
                glslColour(springBackgroundColour) + "\n" +
                juce::String(Assets::springs_shader))) &&
        shaderProgramAttempt->link()) {
        uniforms_.reset();
        shader_   = std::move(shaderProgramAttempt);
        uniforms_ = std::make_unique<Uniforms>(*shader_);
    } else {
    }
}

void SpringsGL::parameterValueChanged(int parameterIndex, float newValue)
{
    auto id = static_cast<PluginProcessor::ParamId>(parameterIndex);
    if (id == PluginProcessor::ParamId::kSpringsDecay) {
        coils_ = 1.f - newValue;
    } else if (id == PluginProcessor::ParamId::kSpringsDamp) {
        radius_ = newValue;
    } else if (id == PluginProcessor::ParamId::kSpringsShape) {
        if (newValue < 0.5f) shape_ = 1.5f;
        else if (newValue < 0.81f)
            shape_ = 0.5f; // we should get the exact value
        else
            shape_ = 1.0f;
    }
}

} // namespace aether
