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
    configureEQUI();
    magnitudes.resize(512); // points across the frequency range
    startTimerHz(30); // refresh at 30 fps
}

void EQUI::timerCallback()
{
    repaint(); // trigger paint at regular intervals
}

void EQUI::paint(juce::Graphics& g)
{
    // Make background black
    g.fillAll(juce::Colour::fromRGB(50, 50, 50));

    // Get bounds
    auto graphBounds = getGraphBounds();
    auto sliderBounds = getSliderBounds();

    // Draw graph setup
    drawGraphSetup(g, graphBounds);

    // Draw Frequency response
    drawFrequencyResponse(g, graphBounds);

    // Draw nodes
    drawNodes(g, graphBounds);

    // Draw slider labels
    drawLabels(g, sliderBounds);
}

void EQUI::resized()
{
    // Only need to resize components (not graph)
    auto sliderBounds = getSliderBounds();

    // Divide into 6 equal columns (one per band)
    int bandWidth = (sliderBounds.getWidth() / Constants::numBands);

    for (int i = 0; i < Constants::numBands; ++i)
    {
        auto& band = eqNodes[i];

        // Each band's vertical strip
        juce::Rectangle<int> bandArea = sliderBounds.removeFromLeft(bandWidth).reduced(4, 0);

        // Top Y
        int y = bandArea.getY();

        // Gain control or color rectangle
        int gainHeight = bandArea.getHeight();

        // Width of band
        int gainWidth = 0.7f * bandArea.getWidth();

        // Set bounds
        band.gainSlider.setBounds(bandArea.getCentreX() - (gainWidth / 2), y, gainWidth, gainHeight);

        // Bounds for toggle button
        int toggleButtonHeight = 10;
        int toggleButtonWidth = gainWidth;

        // Padding for toggle button
        y += gainHeight + toggleButtonHeight;

        // Set bounds
        band.enableToggle.setBounds(
            bandArea.getCentreX() - (toggleButtonWidth / 2),
            y,
            toggleButtonWidth,
            toggleButtonHeight);

        // Space for rotary knobs
        y += toggleButtonHeight + 5;

        // Frequency knob
        int rotarySize = 37;
        band.freqSlider.setBounds(bandArea.getCentreX() - rotarySize / 2, y, rotarySize, rotarySize);

        y += rotarySize;
        // Q knob
        band.qSlider.setBounds(bandArea.getCentreX() - rotarySize / 2, y, rotarySize, rotarySize);
    }
}


//================= Helper functions ====================================//

// Drawing Code
juce::Rectangle<int> EQUI::getGraphBounds() const
{
    auto bounds = getLocalBounds();

    // Remove the right panel (slider column)
    int sliderColumnWidth = 275;
    bounds.removeFromRight(sliderColumnWidth);

    // Custom padding: left, top, right, bottom
    bounds.removeFromLeft(50);
    bounds.removeFromTop(50);
    /*bounds.removeFromRight(20);*/
    bounds.removeFromBottom(100);

    return bounds;
}

juce::Rectangle<int> EQUI::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    int sliderColumnWidth = 275;
    auto sliderBounds = bounds.removeFromRight(sliderColumnWidth);
    
    sliderBounds.removeFromLeft(20);
    sliderBounds.removeFromTop(50);
    sliderBounds.removeFromRight(10);
    sliderBounds.removeFromBottom(100);

    return sliderBounds;
}

void EQUI::drawGraphSetup(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Draw bounding box
    g.setColour(juce::Colours::white);
    g.drawRect(bounds);

    // Implement vignette
    juce::ColourGradient vignette(
        juce::Colours::darkgrey,
        bounds.getCentreX(), bounds.getCentreY(),
        juce::Colours::black,
        bounds.getX(), bounds.getY(),
        true // radial
    );

    g.setGradientFill(vignette);
    g.fillRect(bounds);

    g.setFont(16.0f);
    g.setColour(juce::Colours::white.withAlpha(0.9f));

    // Draw Frequency ticks
    for (int i = 0; i < Constants::numFrequencyLabels; i++)
    {
        double freq = Constants::frequencyGraphLabels[i];
        double normX = std::log10(freq / Constants::minFreq) / std::log10(Constants::maxFreq / Constants::minFreq);
        int x = bounds.getX() + static_cast<int>(normX * bounds.getWidth());

        // Tick mark for all except first and last
        if (i > 0 && i < Constants::numFrequencyLabels - 1)
            g.drawLine((float)x, (float)(bounds.getBottom() - 8), (float)x, (float)(bounds.getBottom()), 1.0f);

        // Label
        juce::String labelText = (freq >= 1000.0)
            ? juce::String(freq / 1000.0, 0) + "k"
            : juce::String((int)freq);

        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawFittedText(labelText, x - 20, bounds.getBottom() + 2, 40, 16, juce::Justification::centred, 1);
    }

    // Draw X axis label
    juce::String hzLabel = "Hz";
    int labelWidth = 40;
    int labelHeight = 16;
    int centerX = bounds.getCentreX();
    int yPos = bounds.getBottom() + 30;

    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.drawFittedText(hzLabel,
        centerX - labelWidth / 2, yPos,
        labelWidth, labelHeight,
        juce::Justification::centred,
        1);

    // Draw Decibel ticks
    for (float dB = Constants::minDb; dB <= Constants::maxDb; dB += 6.0f)
    {
        float y = juce::jmap(dB, Constants::minDb, Constants::maxDb, (float)bounds.getBottom(), (float)bounds.getY());
        g.setColour(dB == 0.0 ? juce::Colours::white.withAlpha(0.9f) : juce::Colours::white.withAlpha(0.5f));

        if (dB > Constants::minDb && dB < Constants::maxDb)
            g.drawHorizontalLine((int)y, (float)bounds.getX(), (float)bounds.getRight());

        juce::String label = dB == 0 ? juce::String(dB, 0) + " dB" : juce::String(dB, 0);
        g.drawFittedText(label, bounds.getX() - 48, (int)y - 7, 35, 14, juce::Justification::centredRight, 1);
    }

}

void EQUI::drawFrequencyResponse(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const int numPoints = magnitudes.size();
    juce::Path responsePath;

    // Get decibel values
    for (int i = 0; i < numPoints; ++i)
    {
        double freq = Constants::minFreq * std::pow(Constants::maxFreq / Constants::minFreq,
            (double)i / (numPoints - 1));
        auto magnitude = eq.getMagnitudeForFrequency(freq, eq.getSampleRate());

        // Clamp to min dB
        magnitudes[i] = std::max(juce::Decibels::gainToDecibels(magnitude), Constants::minDb);
    }

    // Build path (continuous across all points)
    for (int i = 0; i < numPoints; ++i)
    {
        double freq = Constants::minFreq * std::pow(Constants::maxFreq / Constants::minFreq,
            (double)i / (numPoints - 1));
        float x = freqToX(freq, bounds);
        float dB = (float)magnitudes[i];

        float y = juce::jmap(dB, Constants::minDb, Constants::maxDb,
            (float)(bounds.getBottom()), (float)(bounds.getY()));

        if (i == 0)
            responsePath.startNewSubPath(x, y);
        else
            responsePath.lineTo(x, y);
    }

    int focusedBand = (nodeBeingDragged >= 0) ? nodeBeingDragged :
        (nodeUnderMouse >= 0) ? nodeUnderMouse :
        hoveredBand;

    // Fill area under the curve
    if (focusedBand >= 0 && focusedBand < Constants::numBands)
    {
        juce::Path filledPath = responsePath;
        filledPath.lineTo(freqToX(Constants::maxFreq, bounds), (float)bounds.getBottom());
        filledPath.lineTo(freqToX(Constants::minFreq, bounds), (float)bounds.getBottom());
        filledPath.closeSubPath();
        g.setColour(Constants::bandColours[focusedBand].withAlpha(0.05f));
        g.fillPath(filledPath);
    }

    // Draw frequency response line
    g.setColour(juce::Colours::white);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));
}

void EQUI::drawNodes(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    int focusedBand = -1;

    for (int i = 0; i < Constants::numBands; i++)
    {
        auto& node = eqNodes[i];
        node.position = { freqToX(node.freq, bounds), gainToY(node.gain, bounds) };

        // fill ellipse with transparent background of appropriate colour
        float alpha = (nodeUnderMouse == i || hoveredBand == i) ? 0.4f : 0.2f;
        g.setColour(Constants::bandColours[i].withAlpha(alpha));
        g.fillEllipse(node.position.x - 12, node.position.y - 12, 24, 24);

        // black outline
        g.setColour(juce::Colours::black);
        g.drawEllipse(node.position.x - 12, node.position.y - 12, 24, 24, 2);

        // draw black outline around the text by offsetting it in all directions
        g.setFont(20.0f);
        g.setColour(juce::Colours::black);
        juce::String label = juce::String(i + 1);

        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                if (dx != 0 || dy != 0)
                {
                    g.drawText(label,
                        node.position.x - 12 + dx,
                        node.position.y - 12 + dy,
                        24, 24,
                        juce::Justification::centred, false);
                }
            }
        }

        // Then draw the number of the band (index + 1)
        g.setColour(Constants::bandColours[i].interpolatedWith(juce::Colours::white, 0.75f));
        g.drawText(label,
            node.position.x - 12, node.position.y - 12, 24, 24,
            juce::Justification::centred, false);

        // Add some rings to the outside
        float qNorm = juce::jmap(node.Q, Constants::minQ, Constants::maxQ, 0.0f, 1.0f);
        float arcSpanRadians = juce::jmap(
            qNorm,
            0.0f,
            1.0f,
            0.0f,
            juce::MathConstants<float>::halfPi);

        float radius = 12.0f;
        juce::Path upperLeftArc, upperRightArc, lowerLeftArc, lowerRightArc;

        upperLeftArc.addCentredArc(node.position.x, node.position.y,
            radius, radius, 0.0f, 0.0f, -1 * arcSpanRadians, true);
        upperRightArc.addCentredArc(node.position.x, node.position.y,
            radius, radius, 0.0f, 0.0f, arcSpanRadians, true);
        lowerLeftArc.addCentredArc(node.position.x, node.position.y,
            radius, radius, 0.0f, juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi + arcSpanRadians, true);
        lowerRightArc.addCentredArc(node.position.x, node.position.y,
            radius, radius, 0.0f, juce::MathConstants<float>::pi,
            juce::MathConstants<float>::pi - arcSpanRadians, true);

        g.setColour(Constants::bandColours[i]);
        g.strokePath(upperLeftArc, juce::PathStrokeType(2.0f));
        g.strokePath(upperRightArc, juce::PathStrokeType(2.0f));
        g.strokePath(lowerLeftArc, juce::PathStrokeType(2.0f));
        g.strokePath(lowerRightArc, juce::PathStrokeType(2.0f));

        float contourRadius = 14.0f;
        g.setColour(juce::Colours::black);
        g.drawEllipse(node.position.x - contourRadius,
            node.position.y - contourRadius,
            contourRadius * 2.0f,
            contourRadius * 2.0f,
            2.0f);

        if (i == nodeUnderMouse || i == nodeBeingDragged || i == hoveredBand)
            focusedBand = i;
    }

    // Draw focused band path last
    if (focusedBand >= 0)
    {
        auto& node = eqNodes[focusedBand];
        juce::Path bandPath;
        bool drawing = false;

        for (int j = 0; j < magnitudes.size(); ++j)
        {
            double freq = Constants::minFreq * std::pow(Constants::maxFreq / Constants::minFreq,
                (double)j / (magnitudes.size() - 1));
            float magnitude = eq.getMagnitudeForBand(focusedBand, freq, eq.getSampleRate());
            float dB = juce::Decibels::gainToDecibels(magnitude);

            if (dB < Constants::minDb)
            {
                drawing = false;
                continue;
            }

            float y = juce::jmap(dB, Constants::minDb, Constants::maxDb,
                (float)bounds.getBottom(), (float)bounds.getY());
            float x = freqToX(freq, bounds);

            if (!drawing)
            {
                bandPath.startNewSubPath(x, y);
                drawing = true;
            }
            else
            {
                bandPath.lineTo(x, y);
            }
        }

        // If band is enabled, draw frequency response normally. Otherwise make it dashed
        g.setColour(Constants::bandColours[focusedBand].withAlpha(0.9f));
        if (node.isEnabled)
            g.strokePath(bandPath, juce::PathStrokeType(2.0f));
        
        else
        {
            juce::Path dashedPath;
            juce::PathStrokeType stroke(1.0f);
            float dashLengths[] = { 4.0f, 4.0f }; // 4px on, 4px off

            stroke.createDashedStroke(dashedPath, bandPath, dashLengths, 2);
            g.strokePath(dashedPath, stroke);
        }

        // Label text
        juce::String freqText = (node.freq >= 1000.0f)
            ? juce::String(node.freq / 1000.0f, 1) + " kHz"
            : juce::String((int)node.freq) + " Hz";

        bool isPeak = (focusedBand >= EQProcessor::Peak1 && focusedBand <= EQProcessor::Peak4);
        juce::String gainText = isPeak ? juce::String(juce::roundToInt(node.gain)) + " dB" : juce::String();

        float qPercent = juce::jlimit(0.0f, 100.0f,
            juce::jmap(node.Q, Constants::minQ, Constants::maxQ, 0.0f, 100.0f));
        juce::String bwText = juce::String((int)qPercent) + "%";

        juce::String line1 = gainText.isNotEmpty()
            ? freqText + " | " + gainText
            : freqText;
        juce::String line2 = bwText;

        g.setFont(14.0f);
        g.setColour(Constants::bandColours[focusedBand].withAlpha(0.95f));

        int textWidth = 100;
        int textHeight = 16;
        int x = static_cast<int>(node.position.x) - textWidth / 2;
        bool showAbove = node.gain >= 0.0f;
        int y1 = static_cast<int>(node.position.y) + (showAbove ? -52 : 18);
        int y2 = y1 + textHeight + 2;

        g.drawFittedText(line1, x, y1, textWidth, textHeight, juce::Justification::centred, 1);
        g.drawFittedText(line2, x, y2, textWidth, textHeight, juce::Justification::centred, 1);
    }

}

void EQUI::drawLabels(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    int bandWidth = bounds.getWidth() / Constants::numBands;
    int y = bounds.getY() - 25;

    for (int i = 0; i < Constants::numBands; ++i)
    {
        auto& node = eqNodes[i];
        int centerX = bounds.getX() + i * bandWidth + bandWidth / 2;

        juce::Point<float> center((float)centerX, (float)y);

        float radius = 16.0f;

        // Colored transparent fill
        g.setColour(Constants::bandColours[i].withAlpha(0.7f));
        g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);

        // Black outline
        g.setColour(juce::Colours::black);
        g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 3.0f);

        // Label text
        juce::String label = juce::String(i + 1);
        g.setFont(26.0f); // Slightly bigger font to match size

        // Draw black outline pass
        g.setColour(juce::Colours::black);
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                if (dx != 0 || dy != 0)
                {
                    g.drawText(label,
                        center.x - radius + dx, center.y - radius + dy,
                        radius * 2, radius * 2,
                        juce::Justification::centred, false);
                }
            }
        }

        // Draw foreground label
        g.setColour(Constants::bandColours[i].interpolatedWith(juce::Colours::white, 0.7f));
        g.drawText(label,
            center.x - radius, center.y - radius,
            radius * 2, radius * 2,
            juce::Justification::centred, false);
    }
    // Write Freq and BW
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.setFont(12.0f);

    int rotarySize = 37; // same as in resized()
    int toggleHeight = 10;
    int gap = 5;

    // Starting from top of gain slider
    int gainHeight = bounds.getHeight();

    int freqY = bounds.getY() + gainHeight + toggleHeight + gap + (rotarySize / 2) + 2;
    int bwY = freqY + rotarySize;

    int labelX = bounds.getX() - 25; // Left margin for text
    int labelWidth = 40;
    int labelHeight = 16;

    g.drawFittedText("Freq", labelX, freqY, labelWidth, labelHeight, juce::Justification::left, 1);
    g.drawFittedText("BW", labelX + 5, bwY, labelWidth, labelHeight, juce::Justification::left, 1);

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
void EQUI::configureEQSlider(juce::Slider& slider, juce::Slider::SliderStyle sliderStyle, 
    double min, double max, double step,
    const juce::String& suffix, double defaultValue,
    juce::Colour colour)
{
    slider.setSliderStyle(sliderStyle);
    slider.setRange(min, max, step);

    if (suffix == " Hz")
        slider.setSkewFactorFromMidPoint(1000.0);

    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setValue(defaultValue);
    slider.setColour(juce::Slider::thumbColourId, colour);
    slider.setColour(juce::Slider::trackColourId, colour.withAlpha(0.7f));
    slider.setColour(juce::Slider::rotarySliderFillColourId, colour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, colour.withAlpha(0.4f));
    addAndMakeVisible(slider);
}

void EQUI::configureEQUI()
{
    // Set up nodes
    for (int i = 0; i < Constants::numBands; ++i)
    {
        auto& controls = eqNodes[i];
        bool isPeak = (i >= EQProcessor::Peak1 && i <= EQProcessor::Peak4);

        // GAIN

        // 1. Set up Look and Feel
        controls.gainLook = std::make_unique<GainLook>(Constants::bandColours[i]);
        controls.gainSlider.setLookAndFeel(controls.gainLook.get());

        // 2. Set up EQ Slider
        configureEQSlider(controls.gainSlider, juce::Slider::SliderStyle::LinearBarVertical,
            Constants::minDb, Constants::maxDb, 0.1, " dB", Constants::defaultGain,
            Constants::bandColours[i]);
        controls.gain = controls.gainSlider.getValue();

        // 3. Handle callbacks
        if (isPeak)
            controls.gainSlider.onValueChange = [this, i]() { handleSliderChange(i); };

        controls.gainSlider.onHoverChanged = [this, i](bool isHovered)
            {
                hoveredBand = isHovered ? i : -1;
                eqNodes[i].gainLook->setHovered(isHovered);
            };


        // TOGGLE BUTTON

        // 1. Set up Look and Feel
        controls.toggleLook = std::make_unique<ToggleLook>(Constants::bandColours[i]);
        controls.enableToggle.setLookAndFeel(controls.toggleLook.get());

        // 2. Set up Toggle Button
        controls.isEnabled = true;
        controls.enableToggle.setToggleState(true, juce::dontSendNotification);
        
        // 3. Handle callbacks (and make visible)
        controls.enableToggle.onClick = [this, i]() { handleToggleButton(i); };
        controls.enableToggle.onHoverChanged = [this, i](bool isHovered)
            {
                hoveredBand = isHovered ? i : -1;
                eqNodes[i].toggleLook->setHovered(isHovered);
            };
        addAndMakeVisible(controls.enableToggle);


        // FREQUENCY

        // 1. Set up Look and Feel
        controls.frequencyLook = std::make_unique<RotaryLook>(Constants::bandColours[i]);
        controls.freqSlider.setLookAndFeel(controls.frequencyLook.get());

        // 2. Set up EQ Slider
        configureEQSlider(controls.freqSlider, juce::Slider::SliderStyle::Rotary, 
            Constants::minFreq, Constants::maxFreq, 1.0, " Hz", Constants::defaultFrequencies[i],
            Constants::bandColours[i]);
        controls.freq = controls.freqSlider.getValue();

        // 3. Handle callbacks
        controls.freqSlider.onValueChange = [this, i]() { handleSliderChange(i); };
        controls.freqSlider.onHoverChanged = [this, i](bool isHovered)
            {
                hoveredBand = isHovered ? i : -1;
                eqNodes[i].frequencyLook->setHovered(isHovered);
            };


        // BANDWIDTH
        
        // 1. Set up Look and Feel
        controls.qLook = std::make_unique<RotaryLook>(Constants::bandColours[i]);
        controls.qSlider.setLookAndFeel(controls.qLook.get());

        // 2. Set up EQ Slider
        configureEQSlider(controls.qSlider, juce::Slider::SliderStyle::Rotary, 
            Constants::minQ, Constants::maxQ, 0.1, "", Constants::defaultQs[i],
            Constants::bandColours[i]);
        controls.Q = controls.qSlider.getValue();

        // 3. Handle callbacks
        controls.qSlider.onValueChange = [this, i]() { handleSliderChange(i); };
        controls.qSlider.onHoverChanged = [this, i](bool isHovered)
            {
                hoveredBand = isHovered ? i : -1;
                eqNodes[i].qLook->setHovered(isHovered);
            };
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
}

void EQUI::handleNodeChange(int bandIndex)
{
    auto& c = eqNodes[bandIndex];

    // Sync sliders (without triggering callbacks)
    c.freqSlider.setValue(c.freq, juce::dontSendNotification);
    c.qSlider.setValue(c.Q, juce::dontSendNotification);
    if (bandIndex >= EQProcessor::Peak1 && bandIndex <= EQProcessor::Peak4)
        c.gainSlider.setValue(c.gain, juce::dontSendNotification);

    // Update DSP (mouse events already change the node's values)
    eq.updateEQ(bandIndex, c.freq, c.gain, c.Q);
}

void EQUI::handleToggleButton(int bandIndex)
{
    // update UI
    eqNodes[bandIndex].isEnabled = !eqNodes[bandIndex].isEnabled;
    eq.setBandBypass(bandIndex, eqNodes[bandIndex].isEnabled);

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
    hoveredBand = -1;

    // 1. Check draggable nodes
    for (size_t i = 0; i < eqNodes.size(); ++i)
    {
        if (eqNodes[i].position.getDistanceFrom(e.position) < 10.0f)
        {
            nodeUnderMouse = (int)i;
            return;
        }
    }

    // 2. Check label hover
    auto sliderBounds = getSliderBounds();
    int bandWidth = sliderBounds.getWidth() / Constants::numBands;

    for (int i = 0; i < Constants::numBands; ++i)
    {
        int centerX = sliderBounds.getX() + i * bandWidth + bandWidth / 2;
        int y = getLocalBounds().getY() + 20;

        juce::Point<float> center((float)centerX, (float)y);
        float radius = 18.0f;

        if (e.position.getDistanceFrom(center) < radius)
        {
            hoveredBand = i;
            return;
        }
    }
}

void EQUI::mouseDrag(const juce::MouseEvent& e)
{
    if (nodeBeingDragged < 0 || nodeBeingDragged >= (int)eqNodes.size())
        return;

    auto& node = eqNodes[nodeBeingDragged];
    auto bounds = getGraphBounds();
    node.freq = juce::jlimit<float>(Constants::minFreq, Constants::maxFreq, xToFreq(e.position.x, bounds));

    // Only allow vertical dragging (gain) for peaking filters
    if (nodeBeingDragged >= EQProcessor::Peak1 && nodeBeingDragged <= EQProcessor::Peak4)
        node.gain = juce::jlimit(Constants::minDb, Constants::maxDb, yToGain(e.position.y, bounds));
    
    handleNodeChange(nodeBeingDragged);
}

void EQUI::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (nodeUnderMouse >= 0)
    {
        auto& node = eqNodes[nodeUnderMouse];
        node.Q = juce::jlimit(Constants::minQ, Constants::maxQ, node.Q + wheel.deltaY);
        
        handleNodeChange(nodeUnderMouse);
    }
}