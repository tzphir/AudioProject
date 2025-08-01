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

    auto accumulateMag = [&](const Filter& filter)
        {
            const Coeffs* coeffs = filter.coefficients.get();
            if (coeffs != nullptr)
                result *= coeffs->getMagnitudeForFrequency(frequency, sampleRate);
        };

    // assume symmetry, can use left channel only
    accumulateMag(leftChannel.get<HighPass>());
    accumulateMag(leftChannel.get<Peak1>());
    accumulateMag(leftChannel.get<Peak2>());
    accumulateMag(leftChannel.get<Peak3>());
    accumulateMag(leftChannel.get<Peak4>());
    accumulateMag(leftChannel.get<LowPass>());

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