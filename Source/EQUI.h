/*
  ==============================================================================

    EQUI.h
    Created: 28 Jul 2025 3:06:20pm
    Author:  thoma

  ==============================================================================
*/

#pragma once

#include "Constants.h"
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

        // Make sliders hoverable
        template <typename ComponentType>
        struct Hoverable : public ComponentType
        {
            std::function<void(bool)> onHoverChanged;

            void mouseEnter(const juce::MouseEvent& e) override
            {
                if (onHoverChanged) onHoverChanged(true);
                ComponentType::mouseEnter(e); // preserve default behavior
            }

            void mouseExit(const juce::MouseEvent& e) override
            {
                if (onHoverChanged) onHoverChanged(false);
                ComponentType::mouseExit(e); // preserve default behavior
            }
        };

        // Custom Look and Feel for Gain
        struct GainLook : public juce::LookAndFeel_V4
        {
            juce::Colour colour;
            float phase;
            bool isInit = false;
            bool isHovered = false;

            GainLook(juce::Colour c) : colour(c) {}

            void setHovered(bool hovered)
            {
                isHovered = hovered;
            }

            void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                float sliderPos,
                float minSliderPos,
                float maxSliderPos,
                const juce::Slider::SliderStyle style,
                juce::Slider& slider) override
            {
                auto rectangle = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);
                float heightPulse = 0.175f * height;
                float radius = 2.0f;

                // One-time random init
                if (!isInit)
                {
                    phase = juce::Random::getSystemRandom().nextFloat() * (height - heightPulse);
                    isInit = true;
                }

                // Filled area (sliderPos is Y in vertical slider)
                float fillTop = sliderPos;
                float fillBottom = rectangle.getBottom();
                float filledHeight = fillBottom - fillTop;

                float barWidth = rectangle.getWidth();
                float barX = rectangle.getCentreX() - (barWidth / 2.0f);

                // Draw the filled gain bar
                g.setColour(colour.withAlpha(isHovered ? 1.0f : 0.8f));
                g.fillRoundedRectangle(
                    juce::Rectangle<float>(barX, fillTop, barWidth, filledHeight),
                    radius);

                // Calculate the pulse position
                float pulseTop = fillBottom - heightPulse - phase;
                float pulseBottom = fillBottom - phase;

                // If pulse is fully above filled area, reset
                if (pulseBottom < fillTop)
                    phase = -heightPulse;

                // Compute visible portion of the pulse
                float visibleTop = std::max(pulseTop, fillTop);
                float visibleBottom = std::min(pulseBottom, fillBottom);
                float visibleHeight = visibleBottom - visibleTop;

                if (visibleHeight > 0.0f)
                {
                    juce::ColourGradient grad(
                        colour.brighter(0.7f).withAlpha(isHovered ? 0.7f : 0.5f),
                        rectangle.getCentreX(), visibleTop,
                        juce::Colours::transparentWhite,
                        rectangle.getCentreX(), visibleBottom,
                        false);

                    g.setGradientFill(grad);
                    g.fillRoundedRectangle(
                        juce::Rectangle<float>(barX, visibleTop, barWidth, visibleHeight),
                        radius);
                }

                phase += 5.0f;

                g.setColour(colour.withAlpha(isHovered ? 0.7f : 0.5f));
                g.drawRoundedRectangle(rectangle, radius, 2.0f);
            }
        };

        struct ToggleLook : public juce::LookAndFeel_V4
        {
            juce::Colour colour;
            float phase;
            bool isInit = false;
            bool isHovered = false;

            ToggleLook(juce::Colour c) : colour(c) {}

            void setHovered(bool hovered)
            {
                isHovered = hovered;
            }

            void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
            {
                auto bounds = button.getLocalBounds().toFloat();
                float radius = 2.0f;

                if (button.getToggleState()) // button on
                {
                    g.setColour(colour.withAlpha(isHovered ? 1.0f : 0.8f));
                    g.fillRoundedRectangle(bounds, radius);
                    
                }
                else // outline only
                {
                    g.setColour(colour.withAlpha(isHovered ? 1.0f : 0.8f));
                    g.drawRoundedRectangle(bounds, radius, 2.0f);
                }
            }
        };

        struct RotaryLook : public juce::LookAndFeel_V4
        {
            juce::Colour colour;
            float phase;
            bool isInit = false;
            bool isHovered = false;

            RotaryLook(juce::Colour c) : colour(c) {}

            void setHovered(bool hovered)
            {
                isHovered = hovered;
            }

            void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                float sliderPosProportional,
                float rotaryStartAngle, float rotaryEndAngle,
                juce::Slider&) override
            {
                auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(6.0f);
                auto center = bounds.getCentre();
                float radius = bounds.getWidth() / 2.0f;

                float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

                // Background arc
                juce::Path backgroundArc;
                backgroundArc.addCentredArc(center.x, center.y, radius, radius,
                    0.0f, rotaryStartAngle, rotaryEndAngle, true);
                g.setColour(juce::Colours::darkgrey.withAlpha(0.6f));
                g.strokePath(backgroundArc, juce::PathStrokeType(4.0f));

                // Solid value arc (main foreground arc)
                juce::Path valueArc;
                valueArc.addCentredArc(center.x, center.y, radius, radius,
                    0.0f, rotaryStartAngle, angle, true);
                g.setColour(isHovered ? colour : colour.withAlpha(0.8f));
                g.strokePath(valueArc, juce::PathStrokeType(4.0f));

                // Glow trail (overlay gradient arc effect)
                const int numSteps = 30; // resolution of glow trail
                float step = (angle - rotaryStartAngle) / (float)numSteps;

                for (int i = 0; i < numSteps; ++i)
                {
                    float start = rotaryStartAngle + i * step;
                    float end = start + step;

                    float alpha = 0.6f * (1.0f - (float)i / (float)numSteps); // fade out
                    juce::Colour glow = colour.brighter(0.7f).withAlpha(alpha);

                    juce::Path glowArc;
                    glowArc.addCentredArc(center.x, center.y, radius, radius,
                        0.0f, start, end, true);

                    g.setColour(glow);
                    g.strokePath(glowArc, juce::PathStrokeType(2.5f));
                }
            }

        };

        // Structure for an individual node
        struct EQNode
        {
            float freq;
            float gain;
            float Q;
            juce::Point<float> position;
            bool isEnabled;
            Hoverable<juce::Slider> freqSlider;
            Hoverable<juce::Slider> gainSlider;
            Hoverable<juce::Slider> qSlider;
            Hoverable<juce::ToggleButton> enableToggle;

            std::unique_ptr<GainLook> gainLook;
            std::unique_ptr<ToggleLook> toggleLook;
            std::unique_ptr<RotaryLook> frequencyLook;
            std::unique_ptr<RotaryLook> qLook;
        };

        // array of 6 Band controls
        std::array<EQNode, Constants::numBands> eqNodes;

        EQProcessor& eq;
        std::vector<double> magnitudes;

        int nodeUnderMouse = -1; // for highlighting
        int nodeBeingDragged = -1; // current Node
        int hoveredBand = -1; // for component hovering logic

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
        void configureEQUI();

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
