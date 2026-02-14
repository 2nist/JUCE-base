#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TwoNistAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TwoNistAudioProcessorEditor(TwoNistAudioProcessor&);
    ~TwoNistAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TwoNistAudioProcessor& audioProcessor;
    juce::Label title;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwoNistAudioProcessorEditor)
};

