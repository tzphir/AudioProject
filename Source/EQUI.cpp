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
    auto bounds = getGraphBounds();
    drawSetup(g, bounds);
    drawFrequencyResponse(g, bounds);
}

void EQUI::resized()
{
    // Graph node positions
    auto graphArea = getGraphBounds();
    for (auto& band : eqNodes)
    {
        band.position = {
            freqToX(band.freq, graphArea),
            gainToY(band.gain, graphArea)
        };
    }
    auto bounds = getLocalBounds();
    int columnWidth = static_cast<int>(bounds.getWidth() * 0.28f);
    auto sliderArea = bounds.removeFromRight(columnWidth);
    sliderArea = sliderArea.reduced(50, 50);

    // Use sliderArea for laying out sliders
    // Use bounds (the remainder) for graph

    // Example: assign slider bounds...
    int x = sliderArea.getX();
    int y = sliderArea.getY();
    const int h = 30;
    const int spacing = 8;

    for (auto& band : eqNodes)
    {
        band.freqSlider.setBounds(x, y, sliderArea.getWidth(), h);
        y += h + spacing;

        if (band.bandIndex >= EQProcessor::Peak1 && band.bandIndex <= EQProcessor::Peak4)
        {
            band.gainSlider.setBounds(x, y, sliderArea.getWidth(), h);
            y += h + spacing;
        }

        band.qSlider.setBounds(x, y, sliderArea.getWidth(), h);
        y += h + spacing * 2;
    }

    
}


//================= Helper functions ====================================//

// Drawing Code
juce::Rectangle<int> EQUI::getGraphBounds() const
{
    auto bounds = getLocalBounds();
    int sliderColumnWidth = static_cast<int>(bounds.getWidth() * 0.28f); // match layout %
    bounds.removeFromRight(sliderColumnWidth);
    return bounds.reduced(50, 50); // match visual margin
}

void EQUI::drawSetup(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Make background black
    g.fillAll(juce::Colours::black);

    // Draw bounding box
    g.setColour(juce::Colours::white);
    g.drawRect(bounds);

    g.setFont(16.0f);

    // Draw Frequency ticks
    for (int i = 0; i < Constants::numFrequencyLabels; i++)
    {
        double freq = Constants::frequencyGraphLabels[i];
        double normX = std::log10(freq / Constants::minFreq) / std::log10(Constants::maxFreq / Constants::minFreq);
        int x = bounds.getX() + static_cast<int>(normX * bounds.getWidth());

        // Tick mark for all except first and last
        if (i > 0 && i < Constants::numFrequencyLabels - 1)
            g.drawLine((float)x, (float)(bounds.getBottom() - 4), (float)x, (float)(bounds.getBottom()), 1.0f);

        // Label
        juce::String labelText = (freq >= 1000.0)
            ? juce::String(freq / 1000.0, 1) + "k"
            : juce::String((int)freq);

        g.drawFittedText(labelText, x - 20, bounds.getBottom() + 2, 40, 16, juce::Justification::centred, 1);
    }

    // Draw X axis label
    juce::String hzLabel = "Hz";
    int labelWidth = 40;
    int labelHeight = 16;
    int centerX = bounds.getCentreX();
    int yPos = bounds.getBottom() + 30;

    g.drawFittedText(hzLabel,
        centerX - labelWidth / 2, yPos,
        labelWidth, labelHeight,
        juce::Justification::centred,
        1);

    // Draw Decibel ticks
    for (float dB = Constants::minDb; dB <= Constants::maxDb; dB += 6.0f)
    {
        float y = juce::jmap(dB, Constants::minDb, Constants::maxDb, (float)bounds.getBottom(), (float)bounds.getY());
        g.setColour(dB == 0.0 ? juce::Colours::whitesmoke : juce::Colours::darkgrey);

        if (dB > Constants::minDb && dB < Constants::maxDb)
            g.drawHorizontalLine((int)y, (float)bounds.getX(), (float)bounds.getRight());

        juce::String label = dB == 0 ? juce::String(dB, 0) + " dB" : juce::String(dB, 0);
        g.drawFittedText(label, bounds.getX() - 48, (int)y - 7, 35, 14, juce::Justification::centredRight, 1);
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

    g.setColour(juce::Colours::white);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));

   
    for (int i = 0; i < eqNodes.size(); i++)
    {
        auto& node = eqNodes[i];
        node.position = { freqToX(node.freq, bounds), gainToY(node.gain, bounds) };

        juce::Colour colour = Constants::bandColours[i];
        if (node.bandIndex == nodeUnderMouse)
            colour = colour.withAlpha(0.8f);

        g.setColour(colour);
        g.drawEllipse(node.position.x - 12, node.position.y - 12, 24, 24, 2.0f);

        g.setColour(juce::Colours::white);
        g.setFont(20.0f);

        juce::String label = juce::String(i + 1);
        g.drawText(label,
            node.position.x - 12, node.position.y - 12, 24, 24,  // same rect as ellipse
            juce::Justification::centred, false);
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
    auto bounds = getGraphBounds();
    node.freq = juce::jlimit<float>(Constants::minFreq, Constants::maxFreq, xToFreq(e.position.x, bounds));

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
        node.Q = juce::jlimit(Constants::minQ, Constants::maxQ, node.Q + wheel.deltaY);
        
        handleNodeChange(node.bandIndex);
    }
}