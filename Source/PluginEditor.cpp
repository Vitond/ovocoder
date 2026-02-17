/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OvocoderAudioProcessorEditor::OvocoderAudioProcessorEditor (OvocoderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
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
    g.setColour(juce::Colour(200, 200, 66));

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
