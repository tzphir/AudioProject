/*
  ==============================================================================

    EQProcessor.h
    Created: 28 Jul 2025 3:06:48pm
    Author:  thoma

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class EQProcessor
{
    public:

        enum Band
        {
            HighPass = 0,
            Peak1,
            Peak2,
            Peak3,
            Peak4,
            LowPass
        };

        EQProcessor();

        void prepare(const juce::dsp::ProcessSpec& spec);
        void process(juce::AudioBuffer<float>& buffer);
        float getSampleRate() const { return sampleRate; }

        void updateEQ(int bandIndex, float freq, float gainDb, float Q);

        float getMagnitudeForFrequency(double frequency, double sampleRate) const;

    private:

        using Filter = juce::dsp::IIR::Filter<float>;

        using CoeffsPtr = Filter::CoefficientsPtr;

        using Coeffs = juce::dsp::IIR::Coefficients<float>;

        // Mono chain
        using FilterChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter, Filter>;

        // Use two chains
        FilterChain leftChannel, rightChannel;

        // fall back sample rate
        float sampleRate = 44100.0f;

        // DSP -- Change bands
        void EQProcessor::updateCoefficients(CoeffsPtr& old, CoeffsPtr& replacements) { *old = *replacements; }
        

};