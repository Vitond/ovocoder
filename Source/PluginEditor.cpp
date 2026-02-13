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

    addAndMakeVisible(envelopeLabelLeft);
    addAndMakeVisible(envelopeLabelRight);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    startTimer(16);
}

OvocoderAudioProcessorEditor::~OvocoderAudioProcessorEditor()
{
}

void OvocoderAudioProcessorEditor::timerCallback() {
  float envelopeValueLeft = audioProcessor.getEnvelopeValue(0);
  float envelopeValueRight = audioProcessor.getEnvelopeValue(1);
  envelopeLabelLeft.setText(juce::String(envelopeValueLeft, 6), juce::dontSendNotification);
  envelopeLabelRight.setText(juce::String(envelopeValueRight, 6), juce::dontSendNotification);
}

//==============================================================================
void OvocoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void OvocoderAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    envelopeLabelLeft.setBounds(bounds.removeFromTop(40));
    envelopeLabelRight.setBounds(bounds.removeFromTop(80));
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
