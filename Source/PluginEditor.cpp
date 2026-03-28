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
    correlationEnabledButtonAttachment(audioProcessor.apvts, "correlation_enabled", correlationEnabledButton),
    mixSliderAttachment(audioProcessor.apvts, "mix", mixSlider),
    numBandsSliderAttachment(audioProcessor.apvts, "num_bands", numBandsSlider),
    minFreqSliderAttachment(audioProcessor.apvts, "min_freq", minFreqSlider),
    maxFreqSliderAttachment(audioProcessor.apvts, "max_freq", maxFreqSlider)
{

    for (int channel = 0; channel < OvocoderAudioProcessor::numChannels; channel++) {
      for (int band = 0; band < OvocoderAudioProcessor::maxBands; band++) {
        bandEnvelopes[channel][band] = 0.0f;
      }
    }
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 600);
    startTimer(32);

    displayedChannelButton.setButtonText("L");
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
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(numBandsSlider);
    addAndMakeVisible(minFreqSlider);
    addAndMakeVisible(maxFreqSlider);

    attackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    releaseSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    filterQualitySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    filterOrderSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    numBandsSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    minFreqSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    maxFreqSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);

    attackSlider.setNumDecimalPlacesToDisplay(2);
    releaseSlider.setNumDecimalPlacesToDisplay(2);
    filterQualitySlider.setNumDecimalPlacesToDisplay(2);
    outputGainSlider.setNumDecimalPlacesToDisplay(2);
    mixSlider.setNumDecimalPlacesToDisplay(2);
    minFreqSlider.setNumDecimalPlacesToDisplay(1);
    maxFreqSlider.setNumDecimalPlacesToDisplay(1);

    attackSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    releaseSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    filterQualitySlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    filterOrderSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    outputGainSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    mixSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    numBandsSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    minFreqSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);
    maxFreqSlider.setColour(juce::Slider::ColourIds::thumbColourId, mainColour);

    attackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    releaseSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    filterQualitySlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    filterOrderSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    mixSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    numBandsSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    minFreqSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    maxFreqSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);

    attackSlider.setTextValueSuffix("ms");
    releaseSlider.setTextValueSuffix("ms");
    outputGainSlider.setTextValueSuffix("db");

    minFreqSlider.setBounds(0, 40, 80, 80);
    maxFreqSlider.setBounds(100, 40, 80, 80);
    numBandsSlider.setBounds(200, 40, 80, 80);
    filterQualitySlider.setBounds(300, 40, 80, 80);
    filterOrderSlider.setBounds(400, 40, 80, 80);
    attackSlider.setBounds(500, 40, 80, 80);
    releaseSlider.setBounds(600, 40, 80, 80);
    mixSlider.setBounds(700, 40, 80, 80);
    outputGainSlider.setBounds(800, 40, 80, 80);
    
  
    correlationEnabledButton.setBounds(230, 135, 200, 30);
    displayedChannelButton.setBounds(650, 138, 25, 25);

    attackLabel.setText("Attack", juce::NotificationType::dontSendNotification);
    releaseLabel.setText("Release", juce::NotificationType::dontSendNotification);
    filterQualityLabel.setText("Q", juce::NotificationType::dontSendNotification);
    filterOrderLabel.setText("Order", juce::NotificationType::dontSendNotification);
    outputGainLabel.setText("Output gain", juce::NotificationType::dontSendNotification);
    correlationEnabledButtonLabel.setText("Correlation enabled", juce::NotificationType::dontSendNotification);
    mixLabel.setText("Mix", juce::NotificationType::dontSendNotification);
    numBandsLabel.setText("Bands", juce::NotificationType::dontSendNotification);
    minFreqLabel.setText("Min freq", juce::NotificationType::dontSendNotification);
    maxFreqLabel.setText("Max freq", juce::NotificationType::dontSendNotification);

    attackLabel.attachToComponent(&attackSlider, false);
    releaseLabel.attachToComponent(&releaseSlider, false);
    filterQualityLabel.attachToComponent(&filterQualitySlider, false);
    filterOrderLabel.attachToComponent(&filterOrderSlider, false);
    outputGainLabel.attachToComponent(&outputGainSlider, false);
    mixLabel.attachToComponent(&mixSlider, false);
    numBandsLabel.attachToComponent(&numBandsSlider, false);
    minFreqLabel.attachToComponent(&minFreqSlider, false);
    maxFreqLabel.attachToComponent(&maxFreqSlider, false);
    correlationEnabledButtonLabel.setBounds(255, 134, 200, 30);

    addAndMakeVisible(attackLabel);
    addAndMakeVisible(releaseLabel);
    addAndMakeVisible(filterQualityLabel);
    addAndMakeVisible(filterOrderLabel);
    addAndMakeVisible(outputGainLabel);
    addAndMakeVisible(correlationEnabledButtonLabel);
    addAndMakeVisible(mixLabel);
    addAndMakeVisible(numBandsLabel);
    addAndMakeVisible(minFreqSlider);
    addAndMakeVisible(maxFreqSlider);
}

OvocoderAudioProcessorEditor::~OvocoderAudioProcessorEditor()
{
}

void OvocoderAudioProcessorEditor::timerCallback() {
  for (int channel = 0; channel < OvocoderAudioProcessor::numChannels; channel++) {
    for (int band = 0; band < audioProcessor.getNumBands(); band++) {
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

    int numBands = audioProcessor.getNumBands();

    juce::Rectangle<int> bounds = getLocalBounds();
    int gap = 5;
    int barWidth = (bounds.getWidth() - (numBands - 1) * gap) / numBands;

    for (int i = 0; i < numBands; i++) {
      int height = 400 * mainBandEnvelopes[displayedChannel][i];
      g.fillRect(i * barWidth + (i > 0 ? i : 0) * gap, bounds.getHeight() - height, barWidth, height);
    }

    g.setColour(outputColour);

    for (int i = 0; i < numBands; i++) {
      int height = 400 * outputBandEnvelopes[displayedChannel][i];
      g.fillRect(i * barWidth + (i > 0 ? i : 0) * gap, bounds.getHeight() - height, barWidth, height);
    }

    g.setColour(sidechainColour);
    
    for (int i = 0; i < numBands; i++) {
      int height = 400 * bandEnvelopes[displayedChannel][i];
      g.fillRect(i * barWidth + (i > 0 ? i : 0) * gap, bounds.getHeight() - height, barWidth, 5);
    }

    int correlationWidth = 225 * audioProcessor.getCorrelationValue(displayedChannel);
    g.setColour(juce::Colours::black);
    g.fillRect(0, 140, 225, 20);
    g.setColour(mainColour);
    g.fillRect(0, 140, correlationWidth, 20);

    
    juce::Rectangle<int> legendSection = getLocalBounds();
    legendSection.removeFromTop(140);
    legendSection.removeFromLeft(400);

    g.setColour(mainColour);
    g.fillRect(legendSection.getX(), legendSection.getY(), legendRectSize, legendRectSize);
    g.setColour(juce::Colours::white);
    g.drawText("Input", legendSection.getX() + legendRectSize + legendRectTextGap, legendSection.getY(), legendTextWidth, legendRectSize, juce::Justification::left);

    g.setColour(sidechainColour);
    g.fillRect(legendSection.getX() + 70, legendSection.getY(), legendRectSize, legendRectSize);
    g.setColour(juce::Colours::white);
    g.drawText("Sidechain", legendSection.getX() + legendRectSize + legendRectTextGap + 70, legendSection.getY(), legendTextWidth, legendRectSize, juce::Justification::left);

    g.setColour(outputColour);
    g.fillRect(legendSection.getX() + 70 + 95, legendSection.getY(), legendRectSize, legendRectSize);
    g.setColour(juce::Colours::white);
    g.drawText("Output", legendSection.getX() + legendRectSize + legendRectTextGap + 70 + 95, legendSection.getY(), legendTextWidth, legendRectSize, juce::Justification::left);
}

void OvocoderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
