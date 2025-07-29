#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{

    addAndMakeVisible(eqUI);
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlockExpected);
    spec.numChannels = 1;

    eq.prepare(spec);

    // Sync the EQ bands with the current slider values
    for (int i = 0; i < eqUI.eqNodes.size(); ++i)
        eqUI.handleSliderChange(i);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::AudioBuffer<float> sineWave = generateSineWave(bufferToFill.numSamples, 2);
    eq.process(sineWave);
    for (int i = 0; i < bufferToFill.numSamples; ++i)
    {
        float leftSample = sineWave.getSample(0, i);
        float rightSample = sineWave.getSample(1, i);
        bufferToFill.buffer->setSample(0, i, leftSample);
        bufferToFill.buffer->setSample(1, i, rightSample);
        
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    eqUI.setBounds(getLocalBounds());
}

// ============== Helper functions ============== //

juce::AudioBuffer<float> MainComponent::generateSineWave(int numSamples, int numChannels)
{
    static float phase = 0.0f;
    const float frequency = 440.0f;
    const float sampleRate = eq.getSampleRate();
    juce::AudioBuffer<float> tempBuffer(numChannels, numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = std::sin(phase);
        phase += 2.0f * juce::MathConstants<float>::pi * frequency / sampleRate;
        if (phase >= 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;

        tempBuffer.setSample(0, i, sample);
        tempBuffer.setSample(1, i, sample);
    }

    return tempBuffer;
}