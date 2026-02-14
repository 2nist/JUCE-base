#include "PluginEditor.h"

TwoNistAudioProcessorEditor::TwoNistAudioProcessorEditor(TwoNistAudioProcessor& processor)
    : AudioProcessorEditor(&processor), audioProcessor(processor)
{
    juce::ignoreUnused(audioProcessor);

    title.setText("2nist MIDI Arranger - Baseline", juce::dontSendNotification);
    title.setFont(juce::FontOptions(20.0f));
    title.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title);

    setSize(480, 180);
}

void TwoNistAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::whitesmoke);
    g.drawRect(getLocalBounds(), 1);
}

void TwoNistAudioProcessorEditor::resized()
{
    title.setBounds(getLocalBounds().reduced(16));
}

