/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OvocoderAudioProcessor::OvocoderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

OvocoderAudioProcessor::~OvocoderAudioProcessor()
{
}

//==============================================================================
const juce::String OvocoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OvocoderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OvocoderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OvocoderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OvocoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OvocoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OvocoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OvocoderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OvocoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void OvocoderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OvocoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    float releaseInMs = 100.0f;
    float attackInMs = 20.0f;
    float releaseInSamples = releaseInMs * sampleRate / 1000;
    float attackInSamples = attackInMs * sampleRate / 1000;

    releaseCoeff = std::exp(-1 / releaseInSamples);
    attackCoeff = std::exp(-1 / attackInSamples);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    float minCenterFreq = 80.0;
    float maxCenterFreq = 7800.0;

    for (int channel = 0; channel < numChannels; channel++) {
        for (int i = 0; i < numBands; i++)  {
            sidechainFilters[channel][i].prepare(spec);
            mainFilters[channel][i].prepare(spec);
            float ratio = static_cast<float>(i) / (numBands - 1);
            float centerFreq = minCenterFreq * std::pow(maxCenterFreq / minCenterFreq, ratio);
            Coefficients::Ptr coefficients = Coefficients::makeBandPass(sampleRate, centerFreq);
            sidechainFilters[channel][i].coefficients = coefficients;
            mainFilters[channel][i].coefficients = coefficients;
        }
    }

    processBuffer.setSize(numChannels, samplesPerBlock);
    outputBuffer.setSize(numChannels, samplesPerBlock);
}

void OvocoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OvocoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void OvocoderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::AudioBuffer<float> sidechainBuffer = getBusBuffer(buffer, true, 1);
    juce::AudioBuffer<float> mainBuffer = getBusBuffer(buffer, true, 0);
    int numChannels = mainBuffer.getNumChannels();
    int numSamples = mainBuffer.getNumSamples();

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* sidechainChannelData = sidechainBuffer.getWritePointer(channel);
        float* mainChannelData = mainBuffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; sample++) {

            float sumMainSample = 0.0f;

            for (int band = 0; band < numBands; band++) {
                float processedSidechainSample = sidechainFilters[channel][band].processSample(sidechainChannelData[sample]);
                float absoluteProcessedSidechainValue = std::abs(processedSidechainSample);
                float envelopeState = envelopeStates[channel][band];

                if (absoluteProcessedSidechainValue > envelopeState) {
                    envelopeStates[channel][band] += (absoluteProcessedSidechainValue - envelopeState) * attackCoeff;
                } else {
                    envelopeStates[channel][band] -= (envelopeState - absoluteProcessedSidechainValue) * releaseCoeff;
                }

                float processedMainSample = mainFilters[channel][band].processSample(mainChannelData[sample]);
                sumMainSample += processedMainSample * envelopeStates[channel][band];
            }

            mainChannelData[sample] = sumMainSample;

        }

        for (int band = 0; band < numBands; band++) {
            envelopeValues[channel][band].store(envelopeStates[channel][band]);
        }
    }

}

//==============================================================================
bool OvocoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OvocoderAudioProcessor::createEditor()
{
    return new OvocoderAudioProcessorEditor (*this);
}

//==============================================================================
void OvocoderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OvocoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OvocoderAudioProcessor();
}
