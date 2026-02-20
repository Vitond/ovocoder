/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MAX_ORDER 8
//==============================================================================
/**
*/
class OvocoderAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    OvocoderAudioProcessor();
    ~OvocoderAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    float getEnvelopeValue(int channel, int band) const { return envelopeValues[channel][band].load(); }

    static constexpr int numBands = 12;
    static constexpr int numChannels = 2;

    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OvocoderAudioProcessor)
    float envelopeStates[2][numBands] = {0.0f, 0.0f};
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    std::atomic<float> envelopeValues[2][numBands] = {0.0f, 0.0f};

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    Filter sidechainFilters[numChannels][numBands][MAX_ORDER];
    Filter mainFilters[numChannels][numBands][MAX_ORDER];

    juce::AudioBuffer<float> processBuffer;
    juce::AudioBuffer<float> outputBuffer;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void setAttackCoeff(float attackInMs);
    void setReleaseCoeff(float releaseInMs);
    void setFilterQualityFactor(float Q);

    void updateFilterCoefficients();

    int sampleRate = 48000;

    void parameterChanged(const juce::String & parameterId, float newValue) override;

    float qualityFactor = 0.7071f;

    float minCenterFreq = 20.0;
    float maxCenterFreq = 20000.0;

    int order = 2;
};
