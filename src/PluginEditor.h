#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TwoNistAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit TwoNistAudioProcessorEditor(TwoNistAudioProcessor&);
    ~TwoNistAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    TwoNistAudioProcessor& audioProcessor;
    juce::Label title;
    juce::Label status;
    juce::Label transportLamp;
    juce::TextButton panicButton { "Panic" };
    juce::Label paramsHeader;
    juce::Label transportHeader;

    juce::Label rootLabel;
    juce::Label patternLabel;
    juce::Label repeatsLabel;
    juce::Label channelLabel;
    juce::Label gateLabel;
    juce::Label velocityLabel;

    juce::Slider rootSlider;
    juce::ComboBox patternBox;
    juce::Slider repeatsSlider;
    juce::Slider channelSlider;
    juce::Slider gateSlider;
    juce::Slider velocitySlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rootAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> patternAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> repeatsAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> channelAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocityAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwoNistAudioProcessorEditor)
};
