/*
  ==============================================================================

    EQUI.cpp
    Created: 28 Jul 2025 3:06:20pm
    Author:  thoma

  ==============================================================================
*/

#include "EQUI.h"
#include "Constants.h"

EQUI::EQUI(EQProcessor& processor)
    : eq(processor)
{
    startTimerHz(30); // refresh at 30 fps
    magnitudes.resize(512); // points across the frequency range
}

void EQUI::timerCallback()
{
    repaint(); // trigger paint at regular intervals
}

void EQUI::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(10, 20);
    drawSetup(g, bounds);
    drawFrequencyResponse(g, bounds);
}

//================= Helper functions ====================================//

void EQUI::drawSetup(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Make background black
    g.fillAll(juce::Colours::black);

    // Draw bounding box
    g.setColour(juce::Colours::white);
    g.drawRect(bounds);

    g.setFont(16.0f);

    for (int i = 0; i < Constants::numFrequencyLabels; i++)
    {
        double freq = Constants::frequencyGraphLabels[i];
        double normX = std::log10(freq / Constants::minFreq) / std::log10(Constants::maxFreq / Constants::minFreq);
        int x = bounds.getX() + static_cast<int>(normX * bounds.getWidth());

        // Tick mark
        g.setColour(juce::Colours::white);
        g.drawLine((float)x, (float)(bounds.getBottom() - 4), (float)x, (float)(bounds.getBottom()), 1.0f);

        // Label
        juce::String labelText = (freq >= 1000.0)
            ? juce::String(freq / 1000.0, 1) + "k"
            : juce::String((int)freq);

        g.drawFittedText(labelText, x - 20, bounds.getBottom() + 2, 40, 16, juce::Justification::centred, 1);
    }

    for (float dB = Constants::minDb; dB <= Constants::maxDb; dB += 6.0f)
    {
        float y = juce::jmap(dB, Constants::minDb, Constants::maxDb, (float)bounds.getBottom(), (float)bounds.getY());
        g.setColour(dB == 0.0f ? juce::Colours::white : juce::Colours::darkgrey);
        g.drawHorizontalLine((int)y, (float)bounds.getX(), (float)bounds.getRight());

        juce::String label = juce::String(dB, 0) + " dB";
        g.drawFittedText(label, bounds.getX() - 40, (int)y - 7, 35, 14, juce::Justification::centredRight, 1);
    }

}

void EQUI::drawFrequencyResponse(juce::Graphics& g, juce::Rectangle<int> bounds)
{

    const int width = bounds.getWidth();

    juce::Path responsePath;

    for (int i = 0; i < magnitudes.size(); ++i)
    {
        double freq = Constants::minFreq * std::pow(Constants::maxFreq / Constants::minFreq, (double)i / (magnitudes.size() - 1));
        auto magnitude = eq.getMagnitudeForFrequency(freq, eq.getSampleRate());
        magnitudes[i] = juce::Decibels::gainToDecibels(magnitude);
    }

    auto freqToX = [&](double freq) -> int {
        double normX = std::log10(freq / Constants::minFreq) / std::log10(Constants::maxFreq / Constants::minFreq);
        return bounds.getX() + static_cast<int>(normX * width);
        };

    for (int i = 0; i < magnitudes.size(); ++i)
    {
        double freq = Constants::minFreq * std::pow(Constants::maxFreq / Constants::minFreq, (double)i / (magnitudes.size() - 1));
        int x = freqToX(freq);
        float dB = juce::jlimit(Constants::minDb, Constants::maxDb, (float)magnitudes[i]);
        float y = juce::jmap(dB, Constants::minDb, Constants::maxDb, (float)(bounds.getBottom()), (float)(bounds.getY()));

        if (i == 0)
            responsePath.startNewSubPath((float)x, y);
        else
            responsePath.lineTo((float)x, y);
    }

    g.setColour(juce::Colours::lime);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));
}

void EQUI::configureAllSliders()
{
    // Set up sliders (for frequency, bandwidth and gain)
    for (int i = 0; i < bandControls.size(); ++i)
    {
        auto& controls = bandControls[i];
        bool isPeak = (i >= EQProcessor::Peak1 && EQProcessor::Peak4);

        configureSlider(controls.freqSlider, Constants::minFreq, Constants::maxFreq, 1.0, " Hz", Constants::defaultFrequencies[i]);
        configureSlider(controls.qSlider, Constants::minQ, Constants::maxQ, 0.1, "", Constants::defaultQs[i]);

        // High-pass and Low-pass should not have a gain slider, only peaking
        if (isPeak)
            configureSlider(controls.gainSlider, Constants::minDb, Constants::maxDb, 0.1, " dB", Constants::defaultGain);


        // callback functions
        controls.freqSlider.onValueChange = [this, i]() { updateBandFromSliders(i); };
        controls.qSlider.onValueChange = [this, i]() { updateBandFromSliders(i); };
        if (isPeak)
            controls.gainSlider.onValueChange = [this, i]() { updateBandFromSliders(i); };
    }
}

void EQUI::configureSlider(juce::Slider& slider, double min, double max, double step,
    const juce::String& suffix, double defaultValue)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    slider.setRange(min, max, step);
    slider.setTextValueSuffix(suffix);

    if (suffix == " Hz")
        slider.setSkewFactorFromMidPoint(1000.0);

    slider.setValue(defaultValue);
    addAndMakeVisible(slider);

}


void EQUI::updateBandFromSliders(int bandIndex)
{
    const auto& c = bandControls[bandIndex];

    float freq = c.freqSlider.getValue();
    float Q = c.qSlider.getValue();
    float gain = 0.0f;

    if (bandIndex >= EQProcessor::Peak1 && bandIndex <= EQProcessor::Peak4)
        gain = c.gainSlider.getValue();

    eq.updateBandParameters(bandIndex, freq, gain, Q);
}


