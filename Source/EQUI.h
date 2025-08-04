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

    private:
        void timerCallback() override;

        // Hoverable Slider
        struct HoverableSlider : public juce::Slider
        {
            std::function<void(bool)> onHoverChanged;

            void mouseEnter(const juce::MouseEvent&) override
            {
                if (onHoverChanged) onHoverChanged(true);
            }

            void mouseExit(const juce::MouseEvent&) override
            {
                if (onHoverChanged) onHoverChanged(false);
            }
        };

        // Custom Look and Feel
        struct ToggleLook : public juce::LookAndFeel_V4
        {
            juce::Colour colour;

            ToggleLook(juce::Colour c) : colour(c) {}

            void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
            {
                auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
                g.setColour(colour);

                if (button.getToggleState()) // filled
                {
                    g.fillRect(bounds);
                }
                else // outline only
                {
                    g.drawRect(bounds, 2.0f);
                }
            }
        };

        // Structure for an individual node
        struct EQNode
        {
            int bandIndex;
            float freq;
            float gain;
            float Q;
            HoverableSlider freqSlider;
            HoverableSlider gainSlider;
            HoverableSlider qSlider;
            juce::Point<float> position;
            juce::ToggleButton enableToggle;
            bool isEnabled;

            std::unique_ptr<ToggleLook> toggleLook;
        };

        // array of 6 Band controls
        std::array<EQNode, 6> eqNodes;

        EQProcessor& eq;
        std::vector<double> magnitudes;

        int nodeUnderMouse = -1; // for highlighting
        int nodeBeingDragged = -1; // current Node
        int hoveredSliderBand = -1; // for slider hovering logic

        //================= Helper functions ====================================//

        // Drawing Code
        juce::Rectangle<int> getGraphBounds() const;
        juce::Rectangle<int> getSliderBounds() const;
        void drawGraphSetup(juce::Graphics& g, juce::Rectangle<int> bounds);
        void drawFrequencyResponse(juce::Graphics& g, juce::Rectangle<int> bounds);
        void drawNodes(juce::Graphics& g, juce::Rectangle<int> bounds);
        void drawLabels(juce::Graphics& g, juce::Rectangle<int> bounds);

        // Position to DSP sync
        float freqToX(float freq, juce::Rectangle<int> bounds) const;
        float xToFreq(float x, juce::Rectangle<int> bounds) const;
        float gainToY(float dB, juce::Rectangle<int> bounds) const;
        float yToGain(float y, juce::Rectangle<int> bounds) const;

        // Basic UI Code
        void configureEQSlider(juce::Slider& slider, juce::Slider::SliderStyle sliderStyle, 
            double min, double max, double step,
            const juce::String& suffix, double defaultValue,
            juce::Colour colour);
        void configureEQ();

        // Callback functions
        void handleSliderChange(int bandIndex);
        void handleNodeChange(int bandIndex);
        void handleToggleButton(int bandIndex);

        // Mouse Events
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
};
