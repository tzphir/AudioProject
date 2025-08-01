/*
  ==============================================================================

    Constants.h
    Created: 28 Jul 2025 3:07:25pm
    Author:  thoma

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Constants
{
    // ============ EQ ================ //

    // Number of bands.
    constexpr int numBands = 6;

    // Colors of bands
    inline const juce::Colour bandColours[numBands] = {
        juce::Colour::fromRGB(177, 73, 238),
        juce::Colour::fromRGB(0, 162, 255),
        juce::Colour::fromRGB(0, 255, 0),
        juce::Colour::fromRGB(255, 255, 0),
        juce::Colour::fromRGB(255, 127, 0),
        juce::Colour::fromRGB(255, 0, 0)
    };

    // Default parameters for each band (index-based)
    constexpr float defaultFrequencies[numBands] = { 33.0f, 100.0f, 350.0f, 1350.0f, 5000.0f, 16000.0f };
    constexpr float defaultGain = 0;
    constexpr float defaultQs[numBands] = { 0.707f, 1.0f, 1.0f,   1.0f,   1.0f,    0.707f };

    // Frequency labels for graph (EQ curve and FFT)
    constexpr float frequencyGraphLabels[10] = {
        20.0f, 50.0f, 100.0f, 200.0f, 500.0f,
        1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f
    };

    // Number of labels
    constexpr int numFrequencyLabels = 10;

    // Min and Max frequencies
    constexpr double minFreq = 20.0;
    constexpr double maxFreq = 20000.0;

    // Min and Max decibel values (for eq curve)
    constexpr float minDb = -18.0f;
    constexpr float maxDb = 18.0f;

    // Min and Max bandwidth values
    constexpr float minQ = 0.1f;
    constexpr float maxQ = 6.0f;
}