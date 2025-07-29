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
    configureEQNodes();
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

void EQUI::resized()
{
    // Layout constants
    const int margin = 10;
    const int sliderWidth = 200;
    const int sliderHeight = 30;
    const int spacing = 8;

    // Lay out sliders in a vertical column on the left
    auto area = getLocalBounds().reduced(margin);
    auto sliderArea = area.removeFromLeft(sliderWidth);

    int y = 0;
    for (auto& band : eqNodes)
    {
        band.freqSlider.setBounds(sliderArea.withY(y).withHeight(sliderHeight));
        y += sliderHeight + spacing;

        if (band.bandIndex >= EQProcessor::Peak1 && band.bandIndex <= EQProcessor::Peak4)
        {
            band.gainSlider.setBounds(sliderArea.withY(y).withHeight(sliderHeight));
            y += sliderHeight + spacing;
        }

        band.qSlider.setBounds(sliderArea.withY(y).withHeight(sliderHeight));
        y += sliderHeight + spacing * 2;
    }

    // The remaining area is for the EQ graph and draggable nodes
    auto eqGraphBounds = area.reduced(10, 20);

    // Update node positions based on freq/gain values
    for (auto& band : eqNodes)
    {
        float x = freqToX(band.freq, eqGraphBounds);
        float y = gainToY(band.gain, eqGraphBounds);
        band.position = { x, y };
    }
}

//================= Helper functions ====================================//

// Drawing Code
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

    for (int i = 0; i < magnitudes.size(); ++i)
    {
        double freq = Constants::minFreq * std::pow(Constants::maxFreq / Constants::minFreq, (double)i / (magnitudes.size() - 1));
        int x = freqToX(freq, bounds);
        float dB = juce::jlimit(Constants::minDb, Constants::maxDb, (float)magnitudes[i]);
        float y = juce::jmap(dB, Constants::minDb, Constants::maxDb, (float)(bounds.getBottom()), (float)(bounds.getY()));

        if (i == 0)
            responsePath.startNewSubPath((float)x, y);
        else
            responsePath.lineTo((float)x, y);
    }

    g.setColour(juce::Colours::lime);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));

    for (auto& node : eqNodes)
    {
        node.position = { freqToX(node.freq, bounds), gainToY(node.gain, bounds) };
        g.setColour((&node - &eqNodes[0]) == nodeUnderMouse ? juce::Colours::yellow : juce::Colours::orange);
        g.fillEllipse(node.position.x - 5, node.position.y - 5, 10, 10);
    }
}

// Position to DSP sync
float EQUI::freqToX(float freq, juce::Rectangle<int> bounds) const
{
    double normX = std::log10(freq / Constants::minFreq) / std::log10(Constants::maxFreq / Constants::minFreq);
    return bounds.getX() + static_cast<float>(normX * bounds.getWidth());
}

float EQUI::xToFreq(float x, juce::Rectangle<int> bounds) const
{
    float norm = (x - bounds.getX()) / bounds.getWidth();
    return Constants::minFreq * std::pow(Constants::maxFreq / Constants::minFreq, norm);
}

float EQUI::gainToY(float dB, juce::Rectangle<int> bounds) const
{
    return juce::jmap(dB, Constants::minDb, Constants::maxDb,
        static_cast<float>(bounds.getBottom()), static_cast<float>(bounds.getY()));
}

float EQUI::yToGain(float y, juce::Rectangle<int> bounds) const
{
    return juce::jmap(y, static_cast<float>(bounds.getBottom()), static_cast<float>(bounds.getY()),
        Constants::minDb, Constants::maxDb);
}

// Basic UI Code
void EQUI::configureEQSlider(juce::Slider& slider, double min, double max, double step,
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

void EQUI::configureEQNodes()
{
    // Set up nodes
    for (int i = 0; i < eqNodes.size(); ++i)
    {
        auto& controls = eqNodes[i];
        controls.bandIndex = i;
        bool isPeak = (i >= EQProcessor::Peak1 && i <= EQProcessor::Peak4);

        configureEQSlider(controls.freqSlider, Constants::minFreq, Constants::maxFreq, 1.0, " Hz", Constants::defaultFrequencies[i]);
        controls.freq = controls.freqSlider.getValue();
        configureEQSlider(controls.qSlider, Constants::minQ, Constants::maxQ, 0.1, "", Constants::defaultQs[i]);
        controls.Q = controls.qSlider.getValue();

        // High-pass and Low-pass should not have a gain slider, only peaking
        if (isPeak)
        {
            configureEQSlider(controls.gainSlider, Constants::minDb, Constants::maxDb, 0.1, " dB", Constants::defaultGain);
            controls.gain = controls.gainSlider.getValue();
        }

        // Callback functions
        controls.freqSlider.onValueChange = [this, i]() { handleSliderChange(i); };
        controls.qSlider.onValueChange = [this, i]() { handleSliderChange(i); };
        if (isPeak)
            controls.gainSlider.onValueChange = [this, i]() { handleSliderChange(i); };
            
    }
}

void EQUI::handleSliderChange(int bandIndex)
{
    auto& c = eqNodes[bandIndex];

    c.freq = c.freqSlider.getValue();
    c.Q = c.qSlider.getValue();
    c.gain = (bandIndex >= EQProcessor::Peak1 && bandIndex <= EQProcessor::Peak4)
        ? c.gainSlider.getValue()
        : 0.0f;

    eq.updateEQ(bandIndex, c.freq, c.gain, c.Q);

    // Adjust graphic nodes.
    repaint();
}

void EQUI::handleNodeChange(int bandIndex)
{
    auto& c = eqNodes[bandIndex];

    // Update DSP
    eq.updateEQ(c.bandIndex, c.freq, c.gain, c.Q);

    // Sync sliders (without triggering callbacks)
    c.freqSlider.setValue(c.freq, juce::dontSendNotification);
    c.qSlider.setValue(c.Q, juce::dontSendNotification);
    if (c.bandIndex >= EQProcessor::Peak1 && c.bandIndex <= EQProcessor::Peak4)
        c.gainSlider.setValue(c.gain, juce::dontSendNotification);

    // Redraw curve and node position
    repaint();
}

// Mouse Events
void EQUI::mouseDown(const juce::MouseEvent& e)
{
    for (size_t i = 0; i < eqNodes.size(); ++i)
    {
        if (eqNodes[i].position.getDistanceFrom(e.position) < 10.0f)
        {
            nodeBeingDragged = (int)i;
            break;
        }
    }
}

void EQUI::mouseUp(const juce::MouseEvent&) { nodeBeingDragged = -1; }

void EQUI::mouseMove(const juce::MouseEvent& e)
{
    nodeUnderMouse = -1;
    for (size_t i = 0; i < eqNodes.size(); ++i)
    {
        if (eqNodes[i].position.getDistanceFrom(e.position) < 10.0f)
        {
            nodeUnderMouse = (int)i;
            break;
        }
    }
    repaint();
}

void EQUI::mouseDrag(const juce::MouseEvent& e)
{
    if (nodeBeingDragged < 0 || nodeBeingDragged >= (int)eqNodes.size())
        return;

    auto& node = eqNodes[nodeBeingDragged];
    auto bounds = getLocalBounds().reduced(10, 20);

    node.freq = juce::jlimit<float>((float)Constants::minFreq, (float)Constants::maxFreq, xToFreq(e.position.x, bounds));

    // Only allow vertical dragging (gain) for peaking filters
    if (node.bandIndex >= EQProcessor::Peak1 && node.bandIndex <= EQProcessor::Peak4)
        node.gain = juce::jlimit(Constants::minDb, Constants::maxDb, yToGain(e.position.y, bounds));
    
    handleNodeChange(node.bandIndex);
}

void EQUI::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (nodeUnderMouse >= 0)
    {
        auto& node = eqNodes[nodeUnderMouse];
        node.Q = juce::jlimit(Constants::minQ, Constants::maxQ, node.Q + wheel.deltaY * 0.1f);
        
        handleNodeChange(node.bandIndex);
    }
}