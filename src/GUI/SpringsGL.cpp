#include "SpringsGL.h"
#include "BinaryData.h"
#include "SpringsSection.h"

namespace aether
{

static constexpr auto refreshTimeMs = 40;

juce::String glslColour(juce::Colour colour)
{
    float r = colour.getFloatRed(), g = colour.getFloatGreen(),
          b           = colour.getFloatBlue();
    juce::String glsl = "vec3(" + juce::String(r) + "," + juce::String(g) +
                        "," + juce::String(b) + ")";
    return glsl;
}

SpringsGL::SpringsGL(const PluginProcessor &processor)
{
    setOpaque(true);
    //// Sets the OpenGL version to 3.2
    // openGLContext.setOpenGLVersionRequired(
    //     juce::OpenGLContext::OpenGLVersion::openGL3_2);

    // Attach the OpenGL context but do not start [ see start() ]
    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false);

    setRMS(processor.getRMSStack(), processor.getRMSStackPos());
}

SpringsGL::~SpringsGL()
{
    // Turn off OpenGL
    openGLContext.detach();
}

void SpringsGL::timerCallback() { repaint(); }

void SpringsGL::newOpenGLContextCreated()
{
    // Setup Shaders
    createShaders();

    // Setup Buffer Objects
    openGLContext.extensions.glGenBuffers(1, &VBO); // Vertex Buffer Object
    openGLContext.extensions.glGenBuffers(1, &EBO); // Element Buffer Object

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
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Enable Alpha Blending
    juce::gl::glEnable(juce::gl::GL_BLEND);
    juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA,
                          juce::gl::GL_ONE_MINUS_SRC_ALPHA);

    // Use Shader Program that's been defined
    shader->use();

    // Setup the Uniforms for use in the Shader

    if (uniforms->resolution != nullptr)
        uniforms->resolution->set((GLfloat)renderingScale * getWidth(),
                                  (GLfloat)renderingScale * getHeight());

    if (uniforms->rms != nullptr)
        uniforms->rms->set((GLfloat *)&rms[0][0], RMSStackSize * N);

    if (uniforms->rmspos != nullptr) uniforms->rmspos->set((GLint)*rmspos);

    // Define Vertices for a Square (the view plane)
    GLfloat vertices[] = {
        1.0f,  1.0f,  0.0f, // Top Right
        1.0f,  -1.0f, 0.0f, // Bottom Right
        -1.0f, -1.0f, 0.0f, // Bottom Left
        -1.0f, 1.0f,  0.0f  // Top Left
    };
    // Define Which Vertex Indexes Make the Square
    GLuint indices[] = {
        // Note that we start from 0!
        0, 1, 3, // First Triangle
        1, 2, 3  // Second Triangle
    };

    // Vertex Array Object stuff for later
    // openGLContext.extensions.glGenVertexArrays(1, &VAO);
    // openGLContext.extensions.glBindVertexArray(VAO);

    // VBO (Vertex Buffer Object) - Bind and Write to Buffer
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, VBO);
    openGLContext.extensions.glBufferData(juce::gl::GL_ARRAY_BUFFER,
                                          sizeof(vertices), vertices,
                                          juce::gl::GL_STREAM_DRAW);
    // GL_DYNAMIC_DRAW or GL_STREAM_DRAW
    // Don't we want GL_DYNAMIC_DRAW since this
    // vertex data will be changing alot??
    // test this

    // EBO (Element Buffer Object) - Bind and Write to Buffer
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER,
                                          EBO);
    openGLContext.extensions.glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER,
                                          sizeof(indices), indices,
                                          juce::gl::GL_STREAM_DRAW);
    // GL_DYNAMIC_DRAW or GL_STREAM_DRAW
    // Don't we want GL_DYNAMIC_DRAW since this
    // vertex data will be changing alot??
    // test this

    // Setup Vertex Attributes
    openGLContext.extensions.glVertexAttribPointer(
        0, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 3 * sizeof(GLfloat),
        (GLvoid *)nullptr);
    openGLContext.extensions.glEnableVertexAttribArray(0);

    // Draw Vertices
    // glDrawArrays (GL_TRIANGLES, 0, 6); // For just VBO's (Vertex Buffer
    // Objects)
    juce::gl::glDrawElements(
        juce::gl::GL_TRIANGLES, 6, juce::gl::GL_UNSIGNED_INT,
        nullptr); // For EBO's (Element Buffer Objects) (Indices)

    // Reset the element buffers so child Components draw correctly
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    // openGLContext.extensions.glBindVertexArray(0);
}

void SpringsGL::createShaders()
{
    constexpr char vertexShader[] = "attribute vec3 position;\n"
                                    "\n"
                                    "void main()\n"
                                    "{\n"
                                    "    gl_Position = vec4(position, 1.0);\n"
                                    "}\n";

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgramAttempt =
        std::make_unique<juce::OpenGLShaderProgram>(openGLContext);

    auto springBackgroundColour =
        findColour(SpringsSection::backgroundColourId);
    auto backgroundColour =
        findColour(juce::ResizableWindow::backgroundColourId);

    // Sets up pipeline of shaders and compiles the program
    if (shaderProgramAttempt->addVertexShader(
            juce::OpenGLHelpers::translateVertexShaderToV3(vertexShader)) &&
        shaderProgramAttempt->addFragmentShader(
            juce::OpenGLHelpers::translateFragmentShaderToV3(
                "\n#define RMS_BUFFER_SIZE " + juce::String(RMSStackSize) +
                "\n#define N " + juce::String(N) + "\n#define BORDER_COLOR " +
                glslColour(springBackgroundColour) +
                "\n#define BACKGROUND_COLOR " +
                glslColour(backgroundColour.darker(0.25f)) + "\n" +
                juce::String(BinaryData::spring_shader))) &&
        shaderProgramAttempt->link()) {
        uniforms.reset();
        shader = std::move(shaderProgramAttempt);
        uniforms.reset(new Uniforms(openGLContext, *shader));
    } else {
    }
}

} // namespace aether
