#include "Typefaces.h"
#include "Fonts.h"

Typefaces::Typefaces() :
    title{juce::Typeface::createSystemTypefaceFor(
        Fonts::NunitoSans320_ttf, Fonts::NunitoSans320_ttfSize)},
    symbols{juce::Typeface::createSystemTypefaceFor(Fonts::Symbols_ttf,
                                                    Fonts::Symbols_ttfSize)},
    dfault{juce::Typeface::createSystemTypefaceFor(Fonts::Lexend300_ttf,
                                                   Fonts::Lexend300_ttfSize)},
    defaultMono{juce::Typeface::createSystemTypefaceFor(
        Fonts::Roboto425_ttf, Fonts::Roboto425_ttfSize)}
{
}
JUCE_IMPLEMENT_SINGLETON(Typefaces)
