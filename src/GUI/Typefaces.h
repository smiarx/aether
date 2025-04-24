#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class Typefaces : juce::DeletedAtShutdown
{
  public:
    Typefaces();
    ~Typefaces() override { clearSingletonInstance(); }
    const juce::Typeface::Ptr title;
    const juce::Typeface::Ptr symbols;
    const juce::Typeface::Ptr dfault;
    const juce::Typeface::Ptr defaultMono;

    JUCE_DECLARE_SINGLETON(Typefaces, false)
};
