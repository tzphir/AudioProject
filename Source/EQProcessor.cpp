/*
  ==============================================================================

    EQProcessor.cpp
    Created: 28 Jul 2025 3:06:48pm
    Author:  thoma

  ==============================================================================
*/

#include "EQProcessor.h"

EQProcessor::EQProcessor() {}

void EQProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    // Get the sample rate of the spec.
    sampleRate = spec.sampleRate;
    leftChannel.prepare(spec);
    rightChannel.prepare(spec);
}

void EQProcessor::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> leftContext(block.getSingleChannelBlock(0));
    juce::dsp::ProcessContextReplacing<float> rightContext(block.getSingleChannelBlock(1));

    leftChannel.process(leftContext);
    rightChannel.process(rightContext);
}

void EQProcessor::updateEQ(int bandIndex, float freq, float gainDb, float Q)
{
    switch (bandIndex)
    {
        case HighPass:
        {
            auto coeffs = Coeffs::makeHighPass(sampleRate, freq, Q);
            updateCoefficients(leftChannel.get<HighPass>().coefficients, coeffs);
            updateCoefficients(rightChannel.get<HighPass>().coefficients, coeffs);
            break;
        }

        case Peak1:
        {
            auto coeffs = Coeffs::makePeakFilter(sampleRate, freq, Q, juce::Decibels::decibelsToGain(gainDb));
            updateCoefficients(leftChannel.get<Peak1>().coefficients, coeffs);
            updateCoefficients(rightChannel.get<Peak1>().coefficients, coeffs);
            break;
        }

        case Peak2:
        {
            auto coeffs = Coeffs::makePeakFilter(sampleRate, freq, Q, juce::Decibels::decibelsToGain(gainDb));
            updateCoefficients(leftChannel.get<Peak2>().coefficients, coeffs);
            updateCoefficients(rightChannel.get<Peak2>().coefficients, coeffs);
            break;
        }

        case Peak3:
        {
            auto coeffs = Coeffs::makePeakFilter(sampleRate, freq, Q, juce::Decibels::decibelsToGain(gainDb));
            updateCoefficients(leftChannel.get<Peak3>().coefficients, coeffs);
            updateCoefficients(rightChannel.get<Peak3>().coefficients, coeffs);
            break;
        }

        case Peak4:
        {
            auto coeffs = Coeffs::makePeakFilter(sampleRate, freq, Q, juce::Decibels::decibelsToGain(gainDb));
            updateCoefficients(leftChannel.get<Peak4>().coefficients, coeffs);
            updateCoefficients(rightChannel.get<Peak4>().coefficients, coeffs);
            break;
        }

        case LowPass:
        {
            auto coeffs = Coeffs::makeLowPass(sampleRate, freq, Q);
            updateCoefficients(leftChannel.get<LowPass>().coefficients, coeffs);
            updateCoefficients(rightChannel.get<LowPass>().coefficients, coeffs);
            break;
        }

        default:
        {
            jassertfalse;
            break;
        }
    }
}

float EQProcessor::getMagnitudeForFrequency(double frequency, double sampleRate) const
{
    std::complex<double> result(1.0, 0.0);

    auto accumulateMag = [&](const Filter& filter, bool isBypassed)
    {
        if (isBypassed) return;

        const Coeffs* coeffs = filter.coefficients.get();
        if (coeffs != nullptr)
            result *= coeffs->getMagnitudeForFrequency(frequency, sampleRate);
    };

    accumulateMag(leftChannel.get<HighPass>(), leftChannel.isBypassed<HighPass>());
    accumulateMag(leftChannel.get<Peak1>(), leftChannel.isBypassed<Peak1>());
    accumulateMag(leftChannel.get<Peak2>(), leftChannel.isBypassed<Peak2>());
    accumulateMag(leftChannel.get<Peak3>(), leftChannel.isBypassed<Peak3>());
    accumulateMag(leftChannel.get<Peak4>(), leftChannel.isBypassed<Peak4>());
    accumulateMag(leftChannel.get<LowPass>(), leftChannel.isBypassed<LowPass>());

    return static_cast<float>(std::abs(result));
}

float EQProcessor::getMagnitudeForBand(int bandIndex, double frequency, double sampleRate) const
{
    const Filter* filter = nullptr;

    switch (bandIndex)
    {
        case HighPass: filter = &leftChannel.get<HighPass>(); break;
        case Peak1:    filter = &leftChannel.get<Peak1>();    break;
        case Peak2:    filter = &leftChannel.get<Peak2>();    break;
        case Peak3:    filter = &leftChannel.get<Peak3>();    break;
        case Peak4:    filter = &leftChannel.get<Peak4>();    break;
        case LowPass:  filter = &leftChannel.get<LowPass>();  break;
        default: jassertfalse; return 1.0f;
    }

    const Coeffs* coeffs = filter->coefficients.get();

    if (coeffs != nullptr)
        return static_cast<float>(std::abs(coeffs->getMagnitudeForFrequency(frequency, sampleRate)));

    return 1.0f;
}

void EQProcessor::setBandBypass(int bandIndex, bool isEnabled)
{
    switch (bandIndex)
    {
        case HighPass:
            leftChannel.setBypassed<HighPass>(!isEnabled);
            rightChannel.setBypassed<HighPass>(!isEnabled);
            break;

        case Peak1:
            leftChannel.setBypassed<Peak1>(!isEnabled);
            rightChannel.setBypassed<Peak1>(!isEnabled);
            break;

        case Peak2:
            leftChannel.setBypassed<Peak2>(!isEnabled);
            rightChannel.setBypassed<Peak2>(!isEnabled);
            break;

        case Peak3:
            leftChannel.setBypassed<Peak3>(!isEnabled);
            rightChannel.setBypassed<Peak3>(!isEnabled);
            break;

        case Peak4:
            leftChannel.setBypassed<Peak4>(!isEnabled);
            rightChannel.setBypassed<Peak4>(!isEnabled);
            break;

        case LowPass:
            leftChannel.setBypassed<LowPass>(!isEnabled);
            rightChannel.setBypassed<LowPass>(!isEnabled);
            break;

        default:
            jassertfalse; // invalid band index
            break;
        }
}

