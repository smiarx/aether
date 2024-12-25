#pragma once

#include <juce_opengl/juce_opengl.h>

extern "C" {
#include "springreverb.h"
}

class SpringsGL : public juce::Component,
                  public juce::OpenGLRenderer,
                  public juce::Timer
{

  public:
    SpringsGL();
    ~SpringsGL();

    static const springsfloat *rms;
    static const int *rmspos;
    static const springsfloat *length;
    static const springsfloat *density;

    static void setUniforms(const springsfloat t_rms[RMS_BUFFER_SIZE],
                            const int *t_rmspos, const springsfloat *t_length,
                            const springsfloat *t_density)
    {
        rms     = t_rms;
        rmspos  = t_rmspos;
        length  = t_length;
        density = t_density;
    }

    virtual void timerCallback() override;

    //==========================================================================
    // OpenGL Callbacks

    /** Called before rendering OpenGL, after an OpenGLContext has been
       associated with this OpenGLRenderer (this component is a OpenGLRenderer).
        Sets up GL objects that are needed for rendering.
     */
    void newOpenGLContextCreated() override;

    /** Called when done rendering OpenGL, as an OpenGLContext object is
       closing. Frees any GL objects created during rendering.
     */
    void openGLContextClosing() override;

    /** The OpenGL rendering callback.
     */
    void renderOpenGL() override;

    //==========================================================================
    // JUCE Callbacks

    void paint(juce::Graphics &g) override { (void)g; }

    void resized() override {}

  private:
    //==========================================================================
    // OpenGL Functions

    /** Loads the OpenGL Shaders and sets up the whole ShaderProgram
     */
    void createShaders();

    //==============================================================================
    // This class just manages the uniform values that the fragment shader uses.
    struct Uniforms {
        Uniforms(juce::OpenGLContext &openGLContext,
                 juce::OpenGLShaderProgram &shaderProgram)
        {
            resolution.reset(
                createUniform(openGLContext, shaderProgram, "u_resolution"));
            length.reset(
                createUniform(openGLContext, shaderProgram, "u_length"));
            density.reset(
                createUniform(openGLContext, shaderProgram, "u_density"));
            rms.reset(createUniform(openGLContext, shaderProgram, "u_rms"));
            rmspos.reset(
                createUniform(openGLContext, shaderProgram, "u_rmspos"));
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolution, length,
            density, rms, rmspos;

      private:
        static juce::OpenGLShaderProgram::Uniform *
        createUniform(juce::OpenGLContext &openGLContext,
                      juce::OpenGLShaderProgram &shaderProgram,
                      const char *uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation(
                    shaderProgram.getProgramID(), uniformName) < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Uniform(shaderProgram,
                                                          uniformName);
        }
    };

    // OpenGL Variables
    juce::OpenGLContext openGLContext;
    GLuint VBO, EBO;

    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    std::unique_ptr<Uniforms> uniforms;

    /** DEV NOTE
        If I wanted to optionally have an interchangeable shader system,
        this would be fairly easy to add. Chack JUCE Demo -> OpenGLDemo.cpp for
        an implementation example of this. For now, we'll just allow these
        shader files to be static instead of interchangeable and dynamic.
        String newVertexShader, newFragmentShader;
     */

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpringsGL)
};
