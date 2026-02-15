#pragma once

#include <JuceHeader.h>
#include <foleys_gui_magic/foleys_gui_magic.h>
#include "engine/ArrangementOutput.h"
#include <set>

class TwoNistAudioProcessor : public foleys::MagicProcessor,
                              private juce::AudioProcessorValueTreeState::Listener
{
public:
    TwoNistAudioProcessor();
    ~TwoNistAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::ValueTree createGuiValueTree() override;
    void postSetStateInformation() override { arrangementDirty.store(true); }

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    void requestPanic() { panicRequested.store(true); }
    juce::String getSchedulerStatusText() const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    struct ScheduledMidi
    {
        int sampleOffset = 0;
        bool isNoteOn = false;
        int channel = 1;
        int note = 60;
        int velocity = 100;
    };

    struct BeatRange
    {
        double startBeat = 0.0;
        double endBeat = 0.0;
        int sampleOffset = 0;
    };

    void buildDefaultArrangement();
    void rebuildArrangementFromParams();
    void panicAllNotes(juce::MidiBuffer& midiMessages, int sampleOffset);
    void scheduleSegment(
        const BeatRange& segment,
        double bpm,
        int blockSamples,
        std::vector<ScheduledMidi>& scheduled) const;

    double sampleRateHz = 44100.0;
    double lastPpqPosition = 0.0;
    bool wasPlaying = false;
    std::set<int> activeNotes;

    std::vector<twonist::engine::ArrangedChordEvent> arrangedEvents;
    double arrangementLengthBeats = 0.0;

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<bool> arrangementDirty { true };
    std::atomic<bool> panicRequested { false };

    std::atomic<int> statusActiveNotes { 0 };
    std::atomic<float> statusBpm { 120.0f };
    std::atomic<float> statusBeat { 0.0f };
    std::atomic<bool> statusIsPlaying { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwoNistAudioProcessor)
};
