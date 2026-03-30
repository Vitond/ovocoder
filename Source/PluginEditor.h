/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class OvocoderAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    OvocoderAudioProcessorEditor (OvocoderAudioProcessor&);
    ~OvocoderAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    OvocoderAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OvocoderAudioProcessorEditor)

    float bandEnvelopes[2][OvocoderAudioProcessor::maxBands];
    float mainBandEnvelopes[2][OvocoderAudioProcessor::maxBands];
    float outputBandEnvelopes[2][OvocoderAudioProcessor::maxBands];

    void timerCallback() override;

    juce::Slider 
      attackSlider,
      releaseSlider,
      filterQualitySlider,
      filterOrderSlider,
      outputGainSlider,
      mixSlider,
      numBandsSlider,
      minFreqSlider,
      maxFreqSlider,
      processedGainSlider;

    juce::ToggleButton correlationEnabledButton;

    juce::Colour mainColour = juce::Colour(200, 200, 66);
    juce::Colour sidechainColour = juce::Colour(58, 165, 170);
    juce::Colour outputColour = juce::Colour(58, 165, 70);

    juce::AudioProcessorValueTreeState::SliderAttachment 
      attackSliderAttachment,
      releaseSliderAttachment,
      filterQualitySliderAttachment,
      filterOrderSliderAttachment,
      outputGainSliderAttachment,
      mixSliderAttachment,
      numBandsSliderAttachment,
      minFreqSliderAttachment,
      maxFreqSliderAttachment,
      processedGainSliderAttachment;

    juce::AudioProcessorValueTreeState::ButtonAttachment correlationEnabledButtonAttachment;

    juce::Label 
      attackLabel,
      releaseLabel,
      filterQualityLabel,
      filterOrderLabel,
      outputGainLabel,
      correlationEnabledButtonLabel,
      mixLabel,
      numBandsLabel,
      minFreqLabel,
      maxFreqLabel,
      processedGainLabel;

    int displayedChannel = 0;

    juce::TextButton displayedChannelButton;

    int legendRectSize = 20;
    int legendRectTextGap = 5;
    int legendTextWidth = 100;
};
