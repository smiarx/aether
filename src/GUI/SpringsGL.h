#pragma once

#include <juce_opengl/juce_opengl.h>

#include "../PluginProcessor.h"

class SpringsGL : public juce::Component,
                  public juce::OpenGLRenderer,
                  public juce::Timer
{

  public:
    SpringsGL(const PluginProcessor &processor);
    ~SpringsGL() override;

    static constexpr auto N             = processors::Springs::N;
    static constexpr auto RMSStackSize  = processors::Springs::RMSStackSize;
    static constexpr float Damp2Density = 4500.f;

    void setRMS(const dsp::fSample<N> t_rms[RMSStackSize],
                const std::atomic<int> *t_rmspos)
    {
        rms    = t_rms;
        rmspos = t_rmspos;
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
        Uniforms(juce::OpenGLContext &t_openGLContext,
                 juce::OpenGLShaderProgram &t_shaderProgram)
        {
            resolution.reset(createUniform(t_openGLContext, t_shaderProgram,
                                           "u_resolution"));
            rms.reset(createUniform(t_openGLContext, t_shaderProgram, "u_rms"));
            rmspos.reset(
                createUniform(t_openGLContext, t_shaderProgram, "u_rmspos"));
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolution, rms,
            rmspos;

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

    const dsp::fSample<N> *rms;
    const std::atomic<int> *rmspos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpringsGL)
};
