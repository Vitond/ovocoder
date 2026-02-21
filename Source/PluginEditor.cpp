/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OvocoderAudioProcessorEditor::OvocoderAudioProcessorEditor (OvocoderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    attackSliderAttachment(audioProcessor.apvts, "attack", attackSlider),
    releaseSliderAttachment(audioProcessor.apvts, "release", releaseSlider),
    filterQualitySliderAttachment(audioProcessor.apvts, "q", filterQualitySlider),
    filterOrderSliderAttachment(audioProcessor.apvts, "order", filterOrderSlider),
    outputGainSliderAttachment(audioProcessor.apvts, "gain", outputGainSlider)
{

    for (int channel = 0; channel < OvocoderAudioProcessor::numChannels; channel++) {
      for (int band = 0; band < OvocoderAudioProcessor::numBands; band++) {
        bandEnvelopes[channel][band] = 0.0f;
      }
    }
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 400);
    startTimer(16);

    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(filterQualitySlider);
    addAndMakeVisible(filterOrderSlider);
    addAndMakeVisible(outputGainSlider);

    attackSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    releaseSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    filterQualitySlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    filterOrderSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);

    attackSlider.setNumDecimalPlacesToDisplay(2);
    releaseSlider.setNumDecimalPlacesToDisplay(2);
    filterQualitySlider.setNumDecimalPlacesToDisplay(2);
    outputGainSlider.setNumDecimalPlacesToDisplay(2);

    attackSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    releaseSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    filterQualitySlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    filterOrderSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    outputGainSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);

    attackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    releaseSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    filterQualitySlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    filterOrderSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);

    attackSlider.setTextValueSuffix("ms");
    releaseSlider.setTextValueSuffix("ms");
    outputGainSlider.setTextValueSuffix("db");

    attackSlider.setBounds(0, 40, 80, 80);
    releaseSlider.setBounds(100, 40, 80, 80);
    filterQualitySlider.setBounds(200, 40, 80, 80);
    filterOrderSlider.setBounds(300, 40, 80, 80);
    outputGainSlider.setBounds(400, 40, 80, 80);


    attackLabel.setText("Attack", juce::NotificationType::dontSendNotification);
    releaseLabel.setText("Release", juce::NotificationType::dontSendNotification);
    filterQualityLabel.setText("Q", juce::NotificationType::dontSendNotification);
    filterOrderLabel.setText("Order", juce::NotificationType::dontSendNotification);
    outputGainLabel.setText("Output gain", juce::NotificationType::dontSendNotification);

    attackLabel.attachToComponent(&attackSlider, false);
    releaseLabel.attachToComponent(&releaseSlider, false);
    filterQualityLabel.attachToComponent(&filterQualitySlider, false);
    filterOrderLabel.attachToComponent(&filterOrderSlider, false);
    outputGainLabel.attachToComponent(&outputGainSlider, false);

    addAndMakeVisible(attackLabel);
    addAndMakeVisible(releaseLabel);
    addAndMakeVisible(filterQualityLabel);
    addAndMakeVisible(filterOrderLabel);
    addAndMakeVisible(outputGainLabel);
}

OvocoderAudioProcessorEditor::~OvocoderAudioProcessorEditor()
{
}

void OvocoderAudioProcessorEditor::timerCallback() {
  for (int channel = 0; channel < OvocoderAudioProcessor::numChannels; channel++) {
    for (int band = 0; band < OvocoderAudioProcessor::numBands; band++) {
      bandEnvelopes[channel][band] = audioProcessor.getEnvelopeValue(channel, band);
    }
  }

  repaint();
}

//==============================================================================
void OvocoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(mainColour);

    juce::Rectangle<int> bounds = getLocalBounds();
    int gap = 10;
    int barWidth = (bounds.getWidth() - (OvocoderAudioProcessor::numBands - 1) * gap) / OvocoderAudioProcessor::numBands;
    for (int i = 0; i < OvocoderAudioProcessor::numBands; i++) {
      int height = 500 * bandEnvelopes[0][i];
      g.fillRect(i * barWidth + (i > 0 ? i : 0) * gap, bounds.getHeight() - height, barWidth, height);
    }
}

void OvocoderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
