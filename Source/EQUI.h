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
        void resized() override;

        // Structure for an individual node
        struct EQNode
        {
            int bandIndex;
            float freq;
            float gain;
            float Q;
            juce::Slider freqSlider;
            juce::Slider gainSlider;
            juce::Slider qSlider;
            juce::Point<float> position;
        };

        // array of 6 Band controls
        std::array<EQNode, 6> eqNodes;

        void handleSliderChange(int bandIndex);
        void handleNodeChange(int bandIndex);

    private:
        void timerCallback() override;
   
        EQProcessor& eq;
        std::vector<double> magnitudes;

        int nodeUnderMouse = -1; // for highlighting
        int nodeBeingDragged = -1; // current Node

        //================= Helper functions ====================================//

        // Drawing Code
        juce::Rectangle<int> getGraphBounds() const;
        void drawSetup(juce::Graphics& g, juce::Rectangle<int> bounds);
        void drawFrequencyResponse(juce::Graphics& g, juce::Rectangle<int> bounds);

        // Position to DSP sync
        float freqToX(float freq, juce::Rectangle<int> bounds) const;
        float xToFreq(float x, juce::Rectangle<int> bounds) const;
        float gainToY(float dB, juce::Rectangle<int> bounds) const;
        float yToGain(float y, juce::Rectangle<int> bounds) const;

        // Basic UI Code
        void configureEQSlider(juce::Slider& slider, double min, double max, double step,
            const juce::String& suffix, double defaultValue);
        void configureEQNodes();

        // Mouse Events
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
};
