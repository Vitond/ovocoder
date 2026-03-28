/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MAX_ORDER 8
#define AUTOCORRELATION_DOWNSAMPLE 8
#define MAX_BANDS 64
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
    float getCorrelationValue(int channel) const {return correlationValues[channel].load();}
    float getMainInputEnvelopeValue(int channel, int band) const { return mainInputEnvelopeValues[channel][band].load(); }
    float getOutputEnvelopeValue(int channel, int band) const { return outputEnvelopeValues[channel][band].load(); }
    int getNumBands() const {return numBands; }

    static constexpr int numChannels = 2;
    static constexpr int maxBands = MAX_BANDS;

    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OvocoderAudioProcessor)
    float envelopeStates[2][MAX_BANDS] = {0.0f, 0.0f};
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float correlationAttackCoeff = 0.0f;
    float correlationReleaseCoeff = 0.0f;
    std::atomic<float> envelopeValues[2][MAX_BANDS] = {0.0f, 0.0f};

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    Filter sidechainFilters[numChannels][MAX_BANDS][MAX_ORDER];
    Filter mainFilters[numChannels][MAX_BANDS][MAX_ORDER];

    juce::AudioBuffer<float> processBuffer;
    juce::AudioBuffer<float> outputBuffer;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void setAttackCoeff(float attackInMs);
    void setReleaseCoeff(float releaseInMs);
    void setFilterQualityFactor(float Q);
    void setFilterOrder(int order);
    void setOutputGain(float gainInDb);
    void setCorrelationEnabled(bool enabled);
    void setNumBands(int _numBands);
    void setMix(float mix);

    void updateFilterCoefficients();

    int sampleRate = 48000;

    void parameterChanged(const juce::String & parameterId, float newValue) override;

    float qualityFactor = 0.7071f;

    float minCenterFreq = 20.0;
    float maxCenterFreq = 20000.0;

    int order = 2;

    float gain = 1.0;

    // Autocorrelation
    float minFundamentalFreq = 60.0; 
    float maxFundamentalFreq = 400.0;

    juce::AudioBuffer<float> correlationBuffer;
    int correlationBufferPointers[numChannels] = {0, 0};
    float currentWindowEnergyLevels[numChannels] = {0.0f, 0.0f};
    juce::AudioBuffer<float> correlationLevels;
    juce::AudioBuffer<float> lagEnergyLevels;

    int minLag, maxLag, correlationBufferSize;
    std::atomic<float> correlationValues[2] = {0.0f, 0.0f};
    float lastCorrelation[2] = {0.0f, 0.0f};

    float correlationReleaseInMs = 5.0f;
    float correlationAttackInMs = 5.0f;

    Filter correlationDownsampleFilters[2];

    bool correlationEnabled = false;

    float mainInputEnvelopeStates[2][MAX_BANDS] = {0.0f, 0.0f};
    std::atomic<float> mainInputEnvelopeValues[2][MAX_BANDS] = {0.0f, 0.0f};

    float outputEnvelopeStates[2][MAX_BANDS] = {0.0f, 0.0f};
    std::atomic<float> outputEnvelopeValues[2][MAX_BANDS] = {0.0f, 0.0f};

    float mix = 1.0f;

    int numBands = 8;
};
