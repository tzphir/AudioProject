/*
  ==============================================================================

    EQUI.h
    Created: 28 Jul 2025 3:06:20pm
    Author:  thoma

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EQProcessor.h"

class EQUI : public juce::Component,
    private juce::Timer
{
    public:
        EQUI(EQProcessor& processor);
        ~EQUI() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override {}

    private:
        void timerCallback() override;
        void drawSetup(juce::Graphics& g, juce::Rectangle<int> bounds);
        void drawFrequencyResponse(juce::Graphics& g, juce::Rectangle<int> bounds);

        EQProcessor& eq;
        std::vector<double> magnitudes;

        // Structure for band controls
        struct BandControls
        {
            juce::Slider freqSlider;
            juce::Slider gainSlider;
            juce::Slider qSlider;
        };

        // array of 6 Band controls
        std::array<BandControls, 6> bandControls;

        // ============= Helper functions ============== //

        void configureEQSlider(juce::Slider& slider, double min, double max, double step,
            const juce::String& suffix, double defaultValue);

        void configureAllEQSliders();

        void updateBandFromSliders();

};
