#pragma once

#include <juce_opengl/juce_opengl.h>

#include "../PluginProcessor.h"
#include "juce_audio_processors/juce_audio_processors.h"

namespace aether
{

class SpringsGL : public juce::Component,
                  public juce::SettableTooltipClient,
                  public juce::OpenGLRenderer,
                  public juce::Timer,
                  public juce::AudioProcessorParameter::Listener
{

  public:
    SpringsGL(PluginProcessor &processor);
    ~SpringsGL() override;

    static constexpr auto kN             = processors::Springs::N;
    static constexpr auto kRmsStackSize  = processors::Springs::kRmsStackSize;
    static constexpr float kDamp2Density = 4500.f;

    void timerCallback() override;

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
        Uniforms(juce::OpenGLShaderProgram &tShaderProgram)
        {
            coils.reset(createUniform(tShaderProgram, "u_coils"));
            radius.reset(createUniform(tShaderProgram, "u_radius"));
            shape.reset(createUniform(tShaderProgram, "u_shape"));
            resolution.reset(createUniform(tShaderProgram, "u_resolution"));
            rms.reset(createUniform(tShaderProgram, "u_rms"));
            rmspos.reset(createUniform(tShaderProgram, "u_rmspos"));
            time.reset(createUniform(tShaderProgram, "u_time"));
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> coils, radius,
            shape, resolution, rms, rmspos, time;

      private:
        static juce::OpenGLShaderProgram::Uniform *
        createUniform(juce::OpenGLShaderProgram &shaderProgram,
                      const char *uniformName)
        {
            if (juce::OpenGLExtensionFunctions::glGetUniformLocation(
                    shaderProgram.getProgramID(), uniformName) < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Uniform(shaderProgram,
                                                          uniformName);
        }
    };

    // OpenGL Variables
    juce::OpenGLContext openGlContext_;
    GLuint vbo_, ebo_;

    std::unique_ptr<juce::OpenGLShaderProgram> shader_;
    std::unique_ptr<Uniforms> uniforms_;

    /** DEV NOTE
        If I wanted to optionally have an interchangeable shader system,
        this would be fairly easy to add. Chack JUCE Demo -> OpenGLDemo.cpp for
        an implementation example of this. For now, we'll just allow these
        shader files to be static instead of interchangeable and dynamic.
        String newVertexShader, newFragmentShader;
     */

    // AudioProcessorParameter::Listener functions
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int /*parameterIndex*/,
                                 bool /*gestureIsStarting*/) override
    {
    }

    // click functions
    void mouseDown(const juce::MouseEvent & /*event*/) override
    {
        shake_->store(true);
    }

    PluginProcessor &processor_;

    float time_{};
    const dsp::fSample<kN> *rms_;
    const std::atomic<int> *rmspos_;
    float coils_ = 0.f, radius_ = 0.f, shape_ = 0.5f;
    std::atomic<bool> *shake_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpringsGL)
};

} // namespace aether
