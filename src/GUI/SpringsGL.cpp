#include "SpringsGL.h"

#include "Assets.h"
#include "SpringsSection.h"

namespace aether
{

static constexpr auto refreshTimeMs = 17;

constexpr PluginProcessor::ParamId listenIds[]{
    PluginProcessor::ParamId::SpringsDecay,
    PluginProcessor::ParamId::SpringsDamp,
    PluginProcessor::ParamId::SpringsShape,
};

juce::String glslColour(juce::Colour colour)
{
    float r = colour.getFloatRed(), g = colour.getFloatGreen(),
          b           = colour.getFloatBlue();
    juce::String glsl = "vec3(" + juce::String(r) + "," + juce::String(g) +
                        "," + juce::String(b) + ")";
    return glsl;
}

SpringsGL::SpringsGL(PluginProcessor &processor) :
    m_processor(processor), rms(processor.getRMSStack()),
    rmspos(processor.getRMSStackPos()), shake(processor.getShakeAtomic())
{
    setOpaque(true);
    //// Sets the OpenGL version to 3.2
    // openGLContext.setOpenGLVersionRequired(
    //     juce::OpenGLContext::OpenGLVersion::openGL3_2);

    // Attach the OpenGL context but do not start [ see start() ]
    openGLContext.setComponentPaintingEnabled(true);
    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false);

    // add springgl to listeners
    auto &params = processor.getParameters();
    for (auto id : listenIds) {
        int iid     = static_cast<int>(id);
        auto &param = params.getReference(iid);
        param->addListener(this);
        parameterValueChanged(iid, param->getValue());
    }
}

SpringsGL::~SpringsGL()
{
    // Turn off OpenGL
    openGLContext.detach();

    // remove springgl from listeners
    auto &params = m_processor.getParameters();
    for (auto id : listenIds) {
        int iid     = static_cast<int>(id);
        auto &param = params.getReference(iid);
        param->removeListener(this);
    }
}

void SpringsGL::timerCallback()
{
    openGLContext.triggerRepaint();
    time += 0.001f * refreshTimeMs;
}

void SpringsGL::newOpenGLContextCreated()
{
    // Setup Buffer Objects
    juce::gl::glGenBuffers(1, &VBO); // Vertex Buffer Object
    juce::gl::glGenBuffers(1, &EBO); // Element Buffer Object
                                     //
    // Define Vertices for a Square (the view plane)
    constexpr GLfloat vertices[] = {
        1.0f,  1.0f,  // Top Right
        1.0f,  -1.0f, // Bottom Right
        -1.0f, -1.0f, // Bottom Left
        -1.0f, 1.0f,  // Top Left
    };
    // Define Which Vertex Indexes Make the Square
    constexpr GLuint indices[] = {
        // Note that we start from 0!
        0, 1, 3, // First Triangle
        1, 2, 3  // Second Triangle
    };

    // VBO (Vertex Buffer Object) - Bind and Write to Buffer
    juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, VBO);
    juce::gl::glBufferData(juce::gl::GL_ARRAY_BUFFER, sizeof(vertices),
                           vertices, juce::gl::GL_STATIC_DRAW);

    // EBO (Element Buffer Object) - Bind and Write to Buffer
    juce::gl::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, EBO);
    juce::gl::glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
                           indices, juce::gl::GL_STATIC_DRAW);

    juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
    juce::gl::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);

    // Setup Shaders
    createShaders();

    startTimer(refreshTimeMs);
}

void SpringsGL::openGLContextClosing()
{
    shader.reset();
    uniforms.reset();
}

void SpringsGL::renderOpenGL()
{
    jassert(juce::OpenGLHelpers::isContextActive());

    // Setup Viewport
    const float renderingScale = (float)openGLContext.getRenderingScale();
    juce::gl::glViewport(0, 0, juce::roundToInt(renderingScale * getWidth()),
                         juce::roundToInt(renderingScale * getHeight()));

    // Set background Color
    juce::OpenGLHelpers::clear(
        getLookAndFeel().findColour(SpringsSection::backgroundColourId));

    // Enable Alpha Blending
    // juce::gl::glEnable(juce::gl::GL_BLEND);
    // juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA,
    //                      juce::gl::GL_ONE_MINUS_SRC_ALPHA);

    // Use Shader Program that's been defined
    shader->use();

    // Setup the Uniforms for use in the Shader

    if (uniforms->resolution != nullptr)
        uniforms->resolution->set((GLfloat)renderingScale * getWidth(),
                                  (GLfloat)renderingScale * getHeight());

    if (uniforms->rms != nullptr)
        uniforms->rms->set((GLfloat *)&rms[0][0], RMSStackSize * N);

    if (uniforms->rmspos != nullptr) uniforms->rmspos->set((GLint)*rmspos);

    if (uniforms->coils != nullptr) uniforms->coils->set((GLfloat *)&coils, 1);
    if (uniforms->radius != nullptr)
        uniforms->radius->set((GLfloat *)&radius, 1);
    if (uniforms->shape != nullptr) uniforms->shape->set((GLfloat *)&shape, 1);
    if (uniforms->time != nullptr) {
        uniforms->time->set((GLfloat *)&time, 1);
    }

    juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, VBO);
    juce::gl::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, EBO);

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
    constexpr char vertexShader[] =
        "attribute vec2 position;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgramAttempt =
        std::make_unique<juce::OpenGLShaderProgram>(openGLContext);

    auto springBackgroundColour =
        findColour(SpringsSection::backgroundColourId);

    // Sets up pipeline of shaders and compiles the program
    if (shaderProgramAttempt->addVertexShader(
            juce::OpenGLHelpers::translateVertexShaderToV3(vertexShader)) &&
        shaderProgramAttempt->addFragmentShader(
            juce::OpenGLHelpers::translateFragmentShaderToV3(
                "\n#define RMS_BUFFER_SIZE " + juce::String(RMSStackSize) +
                "\n#define N " + juce::String(N) +
                "\n#define BACKGROUND_COLOR " +
                glslColour(springBackgroundColour) + "\n" +
                juce::String(Assets::springs_shader))) &&
        shaderProgramAttempt->link()) {
        uniforms.reset();
        shader = std::move(shaderProgramAttempt);
        uniforms.reset(new Uniforms(openGLContext, *shader));
    } else {
    }
}

void SpringsGL::parameterValueChanged(int parameterIndex, float newValue)
{
    auto id = static_cast<PluginProcessor::ParamId>(parameterIndex);
    if (id == PluginProcessor::ParamId::SpringsDecay) {
        coils = 1.f - newValue;
    } else if (id == PluginProcessor::ParamId::SpringsDamp) {
        radius = newValue;
    } else if (id == PluginProcessor::ParamId::SpringsShape) {
        if (newValue < 0.5f) shape = 1.5f;
        else if (newValue < 0.81f)
            shape = 0.5f; // we should get the exact value
        else
            shape = 1.0f;
    }
}

} // namespace aether
