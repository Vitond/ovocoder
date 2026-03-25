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
    outputGainSliderAttachment(audioProcessor.apvts, "gain", outputGainSlider),
    correlationEnabledButtonAttachment(audioProcessor.apvts, "correlation_enabled", correlationEnabledButton)
{

    for (int channel = 0; channel < OvocoderAudioProcessor::numChannels; channel++) {
      for (int band = 0; band < OvocoderAudioProcessor::numBands; band++) {
        bandEnvelopes[channel][band] = 0.0f;
      }
    }
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    startTimer(32);

    displayedChannelButton.setButtonText("R");
    displayedChannelButton.onClick = [this] {
      displayedChannel = 1 - displayedChannel;
      displayedChannelButton.setButtonText( displayedChannel == 0 ? "L" : "R");
    };

    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(filterQualitySlider);
    addAndMakeVisible(filterOrderSlider);
    addAndMakeVisible(outputGainSlider);
    addAndMakeVisible(correlationEnabledButton);
    addAndMakeVisible(displayedChannelButton);

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
    correlationEnabledButton.setBounds(200, 165, 200, 30);
    displayedChannelButton.setBounds(getLocalBounds().removeFromRight(40).removeFromTop(40));

    attackLabel.setText("Attack", juce::NotificationType::dontSendNotification);
    releaseLabel.setText("Release", juce::NotificationType::dontSendNotification);
    filterQualityLabel.setText("Q", juce::NotificationType::dontSendNotification);
    filterOrderLabel.setText("Order", juce::NotificationType::dontSendNotification);
    outputGainLabel.setText("Output gain", juce::NotificationType::dontSendNotification);
    correlationLabel.setText("Correlation", juce::NotificationType::dontSendNotification);
    correlationLabel.setBounds(0, 132, 80, 40);
    correlationEnabledButtonLabel.setText("Correlation enabled", juce::NotificationType::dontSendNotification);

    attackLabel.attachToComponent(&attackSlider, false);
    releaseLabel.attachToComponent(&releaseSlider, false);
    filterQualityLabel.attachToComponent(&filterQualitySlider, false);
    filterOrderLabel.attachToComponent(&filterOrderSlider, false);
    outputGainLabel.attachToComponent(&outputGainSlider, false);
    correlationEnabledButtonLabel.attachToComponent(&correlationEnabledButton, false);

    addAndMakeVisible(attackLabel);
    addAndMakeVisible(releaseLabel);
    addAndMakeVisible(filterQualityLabel);
    addAndMakeVisible(filterOrderLabel);
    addAndMakeVisible(outputGainLabel);
    addAndMakeVisible(correlationLabel);
    addAndMakeVisible(correlationEnabledButtonLabel);
}

OvocoderAudioProcessorEditor::~OvocoderAudioProcessorEditor()
{
}

void OvocoderAudioProcessorEditor::timerCallback() {
  for (int channel = 0; channel < OvocoderAudioProcessor::numChannels; channel++) {
    for (int band = 0; band < OvocoderAudioProcessor::numBands; band++) {
      bandEnvelopes[channel][band] = audioProcessor.getEnvelopeValue(channel, band);
      mainBandEnvelopes[channel][band] = audioProcessor.getMainInputEnvelopeValue(channel, band);
      outputBandEnvelopes[channel][band] = audioProcessor.getOutputEnvelopeValue(channel, band);
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
      int height = 400 * mainBandEnvelopes[displayedChannel][i];
      g.fillRect(i * barWidth + (i > 0 ? i : 0) * gap, bounds.getHeight() - height, barWidth, height);
    }

    g.setColour(outputColour);

    for (int i = 0; i < OvocoderAudioProcessor::numBands; i++) {
      int height = 400 * outputBandEnvelopes[displayedChannel][i];
      g.fillRect(i * barWidth + (i > 0 ? i : 0) * gap, bounds.getHeight() - height, barWidth, height);
    }

    g.setColour(sidechainColour);
    
    for (int i = 0; i < OvocoderAudioProcessor::numBands; i++) {
      int height = 400 * bandEnvelopes[displayedChannel][i];
      g.fillRect(i * barWidth + (i > 0 ? i : 0) * gap, bounds.getHeight() - height, barWidth, 10);
    }

    int correlationWidth = bounds.getWidth() * audioProcessor.getCorrelationValue(displayedChannel);
    g.setColour(juce::Colours::black);
    g.fillRect(0, 170, bounds.getWidth() / 4, 20);
    g.setColour(mainColour);
    g.fillRect(0, 170, correlationWidth / 4, 20);

    
    juce::Rectangle<int> legendSection = getLocalBounds().removeFromRight(200);
    legendSection.removeFromTop(100);
    legendSection.removeFromBottom(300);

    g.setColour(mainColour);
    g.fillRect(legendSection.getX(), legendSection.getY(), legendRectSize, legendRectSize);
    g.setColour(juce::Colours::white);
    g.drawText("Input", legendSection.getX() + legendRectSize + legendRectTextGap, legendSection.getY(), legendTextWidth, legendRectSize, juce::Justification::left);

    g.setColour(sidechainColour);
    g.fillRect(legendSection.getX(), legendSection.getY() + legendRowGap, legendRectSize, legendRectSize);
    g.setColour(juce::Colours::white);
    g.drawText("Sidechain", legendSection.getX() + legendRectSize + legendRectTextGap, legendSection.getY() + legendRowGap, legendTextWidth, legendRectSize, juce::Justification::left);

    g.setColour(outputColour);
    g.fillRect(legendSection.getX(), legendSection.getY() + 2 * legendRowGap, legendRectSize, legendRectSize);
    g.setColour(juce::Colours::white);
    g.drawText("Output", legendSection.getX() + legendRectSize + legendRectTextGap, legendSection.getY() + 2 * legendRowGap, legendTextWidth, legendRectSize, juce::Justification::left);
}

void OvocoderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
