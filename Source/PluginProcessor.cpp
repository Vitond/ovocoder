/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout OvocoderAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout parameterLayout(
        std::make_unique<juce::AudioParameterFloat>
        (
            "attack", 
            "Attack", 
            juce::NormalisableRange(0.1f, 100.0f, 0.1f, 0.2f),
            5.0f,
            juce::RangedAudioParameterAttributes<juce::AudioParameterFloatAttributes, float>().withLabel("ms")
        ),
        std::make_unique<juce::AudioParameterFloat>
        (
            "release", 
            "Release", 
            juce::NormalisableRange(1.0f, 500.0f, 0.1f, 0.2f),
            20.0f,
            juce::RangedAudioParameterAttributes<juce::AudioParameterFloatAttributes, float>().withLabel("ms")
        ),
        std::make_unique<juce::AudioParameterFloat>
        (
            "q", 
            "Q", 
            juce::NormalisableRange(0.7071f, 20.0f, 0.01f, 0.2f),
            0.7071f
        ),
        std::make_unique<juce::AudioParameterInt>
        (
            "order", 
            "Order", 
            1,
            8,
            2
        ),
        std::make_unique<juce::AudioParameterFloat>
        (
            "num_bands",
            "Bands",
            juce::NormalisableRange<float>(3.0f, 64.0f, 1.0f, 0.449f),
            8.0f
        ),
        std::make_unique<juce::AudioParameterFloat>
        (
            "gain", 
            "Output gain", 
            juce::NormalisableRange(0.0f, 40.0f, 0.01f, 0.2f),
            0.0f
        ),
        std::make_unique<juce::AudioParameterBool>
        (
            "correlation_enabled",
            "Correlation enabled",
            false
        ),
        std::make_unique<juce::AudioParameterFloat>
        (
            "mix", 
            "Mix", 
            juce::NormalisableRange(0.0f, 1.0f, 0.01f),
            1.0f
        ),
        std::make_unique<juce::AudioParameterFloat>
        (
            "min_freq", 
            "Min frequency", 
            juce::NormalisableRange(20.0f, 20000.0f, 0.1f, 0.2f),
            20.0f
        ),
        std::make_unique<juce::AudioParameterFloat>
        (
            "max_freq", 
            "Max frequency", 
            juce::NormalisableRange(20.0f, 20000.0f, 0.1f, 0.2f),
            20000.0f
        )
    );
    return parameterLayout;
}
//==============================================================================
OvocoderAudioProcessor::OvocoderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withInput  ("UnvoicedInput", juce::AudioChannelSet::stereo(), true)
                       .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
    :
#endif
        apvts(*this, nullptr, "parameters", createParameterLayout())
{
    apvts.addParameterListener("attack", this);
    apvts.addParameterListener("release", this);
    apvts.addParameterListener("q", this);
    apvts.addParameterListener("order", this);
    apvts.addParameterListener("gain", this);
    apvts.addParameterListener("correlation_enabled", this);
    apvts.addParameterListener("mix", this);
    apvts.addParameterListener("num_bands", this);
    apvts.addParameterListener("min_freq", this);
    apvts.addParameterListener("max_freq", this);
}

OvocoderAudioProcessor::~OvocoderAudioProcessor()
{
    apvts.removeParameterListener("attack", this);
    apvts.removeParameterListener("release", this);
    apvts.removeParameterListener("q", this);
    apvts.removeParameterListener("order", this);
    apvts.removeParameterListener("gain", this);
    apvts.removeParameterListener("correlation_enabled", this);
    apvts.removeParameterListener("mix", this);
    apvts.removeParameterListener("num_bands", this);
    apvts.removeParameterListener("min_freq", this);
    apvts.removeParameterListener("max_freq", this);
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

void OvocoderAudioProcessor::setNumBands(int _numBands) {
    numBands = _numBands;
    updateFilterCoefficients();
}

void OvocoderAudioProcessor::setMinFreq(float _minFreq) {
    minCenterFreq = _minFreq;
    updateFilterCoefficients();
}

void OvocoderAudioProcessor::setMaxFreq(float _maxFreq) {
    maxCenterFreq = _maxFreq;
    updateFilterCoefficients();
}

void OvocoderAudioProcessor::setAttackCoeff(float attackInMs) {
    float attackInSamples = attackInMs * sampleRate / 1000;
    attackCoeff = std::exp(-1 / attackInSamples);
}

void OvocoderAudioProcessor::setReleaseCoeff(float releaseInMs) {
    float releaseInSamples = releaseInMs * sampleRate / 1000;
    releaseCoeff = std::exp(-1 / releaseInSamples);
}

void OvocoderAudioProcessor::setFilterOrder(int _order) {
    order = _order;
}

void OvocoderAudioProcessor::setCorrelationEnabled(bool _enabled) {
    correlationEnabled = _enabled;
}

void OvocoderAudioProcessor::setFilterQualityFactor(float Q) {
    qualityFactor = Q;
    updateFilterCoefficients();
}

void OvocoderAudioProcessor::setOutputGain(float gainInDb) {
    gain = std::pow(2, gainInDb / 10.0f);
}

void OvocoderAudioProcessor::setMix(float _mix) {
    mix = _mix;
}

void OvocoderAudioProcessor::updateFilterCoefficients() {
    for (int channel = 0; channel < numChannels; channel++) {
        for (int i = 0; i < numBands; i++)  {
            for (int o = 0; o < MAX_ORDER; o++) {
                float ratio;
                if (numBands == 1) {
                   ratio = 0.0f;
                } else {
                   ratio = static_cast<float>(i) / (numBands - 1);
                }
                float centerFreq = minCenterFreq * std::pow(maxCenterFreq / minCenterFreq, ratio);
                Coefficients::Ptr coefficients = Coefficients::makeBandPass(sampleRate, centerFreq, qualityFactor);
                sidechainFilters[channel][i][o].coefficients = coefficients;
                mainFilters[channel][i][o].coefficients = coefficients;
            }
        }
        Coefficients::Ptr coefficients = Coefficients::makeLowPass(sampleRate, maxFundamentalFreq);
        correlationDownsampleFilters[channel].coefficients = coefficients;
    }
}

void OvocoderAudioProcessor::parameterChanged(const juce::String & parameterID,float newValue) {
    if (parameterID == "attack") {
        setAttackCoeff(newValue);
    } else if (parameterID == "release") {
        setReleaseCoeff(newValue);
    } else if (parameterID == "q") {
        setFilterQualityFactor(newValue);
    } else if (parameterID == "order") {
        setFilterOrder((int)newValue);
    } else if (parameterID == "gain") {
        setOutputGain(newValue);
    } else if (parameterID == "correlation_enabled") {
        setCorrelationEnabled((bool)newValue);
    } else if (parameterID == "mix") {
        setMix(newValue);
    } else if (parameterID == "num_bands") {
        setNumBands((int)newValue);
    } else if (parameterID == "min_freq") {
        setMinFreq(newValue);
    } else if (parameterID == "max_freq") {
        setMaxFreq(newValue);
    }
}
//==============================================================================
void OvocoderAudioProcessor::prepareToPlay (double _sampleRate, int samplesPerBlock)
{
    sampleRate = _sampleRate;

    setReleaseCoeff(apvts.getRawParameterValue("release")->load());
    setAttackCoeff(apvts.getRawParameterValue("attack")->load());
    setFilterQualityFactor(apvts.getRawParameterValue("q")->load());
    setFilterOrder((int)apvts.getRawParameterValue("order")->load());
    setOutputGain(apvts.getRawParameterValue("gain")->load());
    setCorrelationEnabled(apvts.getRawParameterValue("correlation_enabled")->load());
    setMix(apvts.getRawParameterValue("mix")->load());
    setNumBands((int)apvts.getRawParameterValue("num_bands")->load());
    setMinFreq(apvts.getRawParameterValue("min_freq")->load());
    setMaxFreq(apvts.getRawParameterValue("max_freq")->load());

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    for (int channel = 0; channel < numChannels; channel++) {
        for (int i = 0; i < MAX_BANDS; i++)  {
            for (int j = 0; j < MAX_ORDER; j++) {
                sidechainFilters[channel][i][j].prepare(spec);
                mainFilters[channel][i][j].prepare(spec);
            }
        }
        correlationDownsampleFilters[channel].prepare(spec);
    }    

    maxLag = static_cast<int>(sampleRate / (minFundamentalFreq * AUTOCORRELATION_DOWNSAMPLE));
    minLag = static_cast<int>(sampleRate / (maxFundamentalFreq * AUTOCORRELATION_DOWNSAMPLE));
    correlationBufferSize = 2 * maxLag;

    correlationBuffer = juce::AudioBuffer<float>(numChannels, correlationBufferSize);
    correlationLevels = juce::AudioBuffer<float>(numChannels, maxLag - minLag + 1);
    lagEnergyLevels = juce::AudioBuffer<float>(numChannels, maxLag - minLag + 1);
    correlationBuffer.clear();
    correlationLevels.clear();
    lagEnergyLevels.clear();

    updateFilterCoefficients();

    processBuffer.setSize(numChannels, samplesPerBlock);
    outputBuffer.setSize(numChannels, samplesPerBlock);

    float correlationReleaseInSamples = correlationReleaseInMs * sampleRate / 1000;
    correlationReleaseCoeff = std::exp(-1 / correlationReleaseInSamples);

    float correlationAttackInSamples = correlationAttackInMs * sampleRate / 1000;
    correlationAttackCoeff = std::exp(-1 / correlationAttackInSamples);
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

    juce::AudioBuffer<float> sidechainBuffer = getBusBuffer(buffer, true, 2);
    juce::AudioBuffer<float> mainBuffer = getBusBuffer(buffer, true, 0);
    juce::AudioBuffer<float> unvoicedBuffer = getBusBuffer(buffer, true, 1);
    int numChannels = mainBuffer.getNumChannels();
    int numSamples = mainBuffer.getNumSamples();

    if (sidechainBuffer.getNumChannels() != numChannels) {
        return;
    }

    bool unvoicedBufferActive = unvoicedBuffer.getNumChannels() == numChannels;

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
        float* unvoicedChannelData = unvoicedBufferActive ? unvoicedBuffer.getWritePointer(channel) : nullptr;
        float* correlationLevelsData = correlationLevels.getWritePointer(channel);
        float* correlationBufferData = correlationBuffer.getWritePointer(channel);
        float* lagEnergyData = lagEnergyLevels.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; sample++) {

            float correlation = 0;
            float filteredSample = correlationDownsampleFilters[channel].processSample(sidechainChannelData[sample]);
            if (correlationEnabled && (sample % AUTOCORRELATION_DOWNSAMPLE == 0)) {
                int correlationBufferPointer = correlationBufferPointers[channel];
                int currentWindowEndSample = (correlationBufferPointer - (maxLag - minLag) + correlationBufferSize) % correlationBufferSize;
                currentWindowEnergyLevels[channel] += filteredSample * filteredSample;
                currentWindowEnergyLevels[channel] -= correlationBufferData[currentWindowEndSample] * correlationBufferData[currentWindowEndSample];

                float maxCorrelation = 0;
                for (int lag = minLag; lag <= maxLag; lag++) {
                    int lagSample = (correlationBufferPointer - lag + correlationBufferSize) % correlationBufferSize;
                    int currentWindowEndLagSample = (correlationBufferPointer - (maxLag - minLag) - lag + correlationBufferSize) % correlationBufferSize;
                    correlationLevelsData[lag - minLag] += filteredSample * correlationBufferData[lagSample];
                    correlationLevelsData[lag - minLag] -= correlationBufferData[currentWindowEndSample] * correlationBufferData[currentWindowEndLagSample];
                    lagEnergyData[lag - minLag] += correlationBufferData[lagSample] * correlationBufferData[lagSample];
                    lagEnergyData[lag - minLag] -= correlationBufferData[currentWindowEndLagSample] * correlationBufferData[currentWindowEndLagSample];
                    float energy = currentWindowEnergyLevels[channel] * lagEnergyData[lag - minLag];
                    float lagCorrelation;
                    if (energy > 1e-10f) {
                        lagCorrelation = std::abs(correlationLevelsData[lag - minLag]) / std::sqrt(energy);
                    } else {
                        lagCorrelation = 0.0f;
                    }
                    if (lagCorrelation > maxCorrelation) {
                        maxCorrelation = lagCorrelation;
                    }
                }

                correlationBufferData[correlationBufferPointer] = filteredSample;
                correlationBufferPointers[channel] = ((correlationBufferPointer + 1) % correlationBufferSize);

                correlation = std::pow(juce::jlimit(0.0f, 1.0f, (maxCorrelation - 0.5f) * 2), 2.0f);

                if (correlation > lastCorrelation[channel]) {
                    lastCorrelation[channel] += (correlation - lastCorrelation[channel]) * (1 - correlationAttackCoeff);
                } else {
                    lastCorrelation[channel] += (correlation - lastCorrelation[channel]) * (1 - correlationReleaseCoeff);
                }
                
                correlationValues[channel].store(lastCorrelation[channel]);
            }

            float voicedGain = 1.0f, unvoicedGain = 0.0f;
            if (correlationEnabled && unvoicedBufferActive) {
                float angle = lastCorrelation[channel] * juce::MathConstants<float>::halfPi;
                voicedGain = std::sin(angle);
                unvoicedGain = std::cos(angle);
            }

            float sumMainSample = 0.0f;

            for (int band = 0; band < numBands; band++) {
                float processedSidechainSample = sidechainChannelData[sample];
                for (int o = 0; o < order; o++) {
                    processedSidechainSample = sidechainFilters[channel][band][o].processSample(processedSidechainSample);
                }
                float absoluteProcessedSidechainValue = std::abs(processedSidechainSample);
                float envelopeState = envelopeStates[channel][band];

                if (absoluteProcessedSidechainValue > envelopeState) {
                    envelopeStates[channel][band] += (absoluteProcessedSidechainValue - envelopeState) * (1.0f - attackCoeff);
                } else {
                    envelopeStates[channel][band] -= (envelopeState - absoluteProcessedSidechainValue) * (1.0f - releaseCoeff);
                }

                float processedSample = mainChannelData[sample] * voicedGain + (unvoicedChannelData != nullptr ? unvoicedChannelData[sample] * unvoicedGain : 0.0f);
                for (int o = 0; o < order; o++) {
                    processedSample = mainFilters[channel][band][o].processSample(processedSample);
                }

                float absoluteProcessedSample = std::abs(processedSample);
                float mainEnvelopeState = mainInputEnvelopeStates[channel][band];
                if (absoluteProcessedSample > mainEnvelopeState) {
                    mainInputEnvelopeStates[channel][band] += (absoluteProcessedSample - mainEnvelopeState) * (1.0f - attackCoeff);
                } else {
                    mainInputEnvelopeStates[channel][band] -= (mainEnvelopeState - absoluteProcessedSample) * (1.0f - releaseCoeff);
                }

                float appliedEnvelopeSample = processedSample * envelopeStates[channel][band];
                float absoluteAppliedEnvelopeSample = std::abs(appliedEnvelopeSample);

                float outputEnvelopeState = outputEnvelopeStates[channel][band];
                if (absoluteAppliedEnvelopeSample > outputEnvelopeState) {
                    outputEnvelopeStates[channel][band] += (absoluteAppliedEnvelopeSample - outputEnvelopeState) * (1.0f - attackCoeff);
                } else {
                    outputEnvelopeStates[channel][band] -= (outputEnvelopeState - absoluteAppliedEnvelopeSample) * (1.0f - releaseCoeff);
                }

                sumMainSample += appliedEnvelopeSample;
            }

            mainChannelData[sample] = (std::sin(mix * juce::MathConstants<float>::halfPi) * sumMainSample + (std::cos(mix * juce::MathConstants<float>::halfPi)) * mainChannelData[sample]) * gain;
        }

        for (int band = 0; band < numBands; band++) {
            envelopeValues[channel][band].store(envelopeStates[channel][band]);
            mainInputEnvelopeValues[channel][band].store(mainInputEnvelopeStates[channel][band]);
            outputEnvelopeValues[channel][band].store(outputEnvelopeStates[channel][band]);
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
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OvocoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OvocoderAudioProcessor();
}
