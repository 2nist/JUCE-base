#include "PluginProcessor.h"
#include "engine/ProgressionManager.h"

#include <algorithm>
#include <cmath>

TwoNistAudioProcessor::TwoNistAudioProcessor()
    : foleys::MagicProcessor(BusesProperties()),
      parameters(*this, nullptr, "PARAMS", createParameterLayout())
{
    FOLEYS_SET_SOURCE_PATH(__FILE__);

    const juce::StringArray ids { "root", "pattern", "repeats", "channel", "gate", "velocity" };
    for (const auto& id : ids)
        parameters.addParameterListener(id, this);

    buildDefaultArrangement();
}

TwoNistAudioProcessor::~TwoNistAudioProcessor()
{
    const juce::StringArray ids { "root", "pattern", "repeats", "channel", "gate", "velocity" };
    for (const auto& id : ids)
        parameters.removeParameterListener(id, this);
}

void TwoNistAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sampleRateHz = sampleRate > 0.0 ? sampleRate : 44100.0;
    lastPpqPosition = 0.0;
    wasPlaying = false;
    activeNotes.clear();
}

void TwoNistAudioProcessor::releaseResources()
{
    activeNotes.clear();
}

bool TwoNistAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    juce::ignoreUnused(layouts);
    return true;
}

void TwoNistAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (arrangementDirty.exchange(false))
        rebuildArrangementFromParams();

    juce::MidiBuffer processed(midiMessages);
    const int blockSamples = buffer.getNumSamples();

    if (panicRequested.exchange(false))
        panicAllNotes(processed, 0);

    juce::Optional<juce::AudioPlayHead::PositionInfo> positionInfo;
    if (auto* hostPlayHead = getPlayHead())
        positionInfo = hostPlayHead->getPosition();

    const bool gotPosition = positionInfo.hasValue();
    const bool isPlaying = gotPosition && positionInfo->getIsPlaying();
    const double bpm = gotPosition ? positionInfo->getBpm().orFallback(120.0) : 120.0;
    const double currentBeat = gotPosition ? positionInfo->getPpqPosition().orFallback(0.0) : 0.0;
    statusBpm.store(static_cast<float>(bpm));
    statusBeat.store(static_cast<float>(currentBeat));
    statusIsPlaying.store(isPlaying);

    if (!isPlaying)
    {
        if (wasPlaying)
            panicAllNotes(processed, 0);
        wasPlaying = false;
        statusActiveNotes.store(static_cast<int>(activeNotes.size()));
        midiMessages.swapWith(processed);
        return;
    }

    const double samplesPerBeat = (sampleRateHz * 60.0) / std::max(1.0, bpm);
    const double blockBeats = static_cast<double>(blockSamples) / samplesPerBeat;
    const double blockStartBeat = currentBeat;
    const double blockEndBeat = blockStartBeat + blockBeats;

    // Detect transport jumps/seek and flush active notes to avoid stuck notes.
    if (wasPlaying)
    {
        const double expectedNext = lastPpqPosition;
        const double delta = std::abs(blockStartBeat - expectedNext);
        if (delta > 0.5)
            panicAllNotes(processed, 0);
    }

    std::vector<BeatRange> segments;
    const auto loopPoints = gotPosition ? positionInfo->getLoopPoints() : juce::Optional<juce::AudioPlayHead::LoopPoints>();
    const bool hasValidLoop = gotPosition && positionInfo->getIsLooping() && loopPoints.hasValue() && loopPoints->ppqEnd > loopPoints->ppqStart;
    if (hasValidLoop && blockEndBeat > loopPoints->ppqEnd)
    {
        const double firstSegmentBeats = std::max(0.0, loopPoints->ppqEnd - blockStartBeat);
        const int firstSegmentSamples = juce::jlimit(0, blockSamples, static_cast<int>(std::round(firstSegmentBeats * samplesPerBeat)));
        segments.push_back({ blockStartBeat, loopPoints->ppqEnd, 0 });
        segments.push_back({ loopPoints->ppqStart, loopPoints->ppqStart + (blockEndBeat - loopPoints->ppqEnd), firstSegmentSamples });
    }
    else
    {
        segments.push_back({ blockStartBeat, blockEndBeat, 0 });
    }

    std::vector<ScheduledMidi> scheduled;
    scheduled.reserve(256);
    for (const auto& segment : segments)
        scheduleSegment(segment, bpm, blockSamples, scheduled);

    std::sort(scheduled.begin(), scheduled.end(), [](const ScheduledMidi& a, const ScheduledMidi& b) {
        if (a.sampleOffset != b.sampleOffset)
            return a.sampleOffset < b.sampleOffset;
        if (a.isNoteOn != b.isNoteOn)
            return !a.isNoteOn;
        return a.note < b.note;
    });

    for (const auto& event : scheduled)
    {
        const int key = (event.channel << 8) | event.note;
        if (event.isNoteOn)
        {
            processed.addEvent(juce::MidiMessage::noteOn(event.channel, event.note, static_cast<juce::uint8>(event.velocity)), event.sampleOffset);
            activeNotes.insert(key);
        }
        else
        {
            processed.addEvent(juce::MidiMessage::noteOff(event.channel, event.note), event.sampleOffset);
            activeNotes.erase(key);
        }
    }

    midiMessages.swapWith(processed);
    lastPpqPosition = blockEndBeat;
    wasPlaying = true;
    statusActiveNotes.store(static_cast<int>(activeNotes.size()));
}

juce::ValueTree TwoNistAudioProcessor::createGuiValueTree()
{
    const auto layoutFile = juce::File::getCurrentWorkingDirectory()
                                .getChildFile("resources")
                                .getChildFile("designer_layout.xml");
    if (layoutFile.existsAsFile())
    {
        const auto xmlText = layoutFile.loadFileAsString();
        const auto tree = juce::ValueTree::fromXml(xmlText);
        if (tree.isValid())
            return tree;
    }

    return foleys::MagicProcessor::createGuiValueTree();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TwoNistAudioProcessor();
}

void TwoNistAudioProcessor::buildDefaultArrangement()
{
    rebuildArrangementFromParams();
}

void TwoNistAudioProcessor::panicAllNotes(juce::MidiBuffer& midiMessages, int sampleOffset)
{
    for (const auto key : activeNotes)
    {
        const int channel = (key >> 8) & 0x0f;
        const int note = key & 0xff;
        midiMessages.addEvent(juce::MidiMessage::noteOff(channel, note), sampleOffset);
    }

    for (int channel = 1; channel <= 16; ++channel)
        midiMessages.addEvent(juce::MidiMessage::allNotesOff(channel), sampleOffset);

    activeNotes.clear();
}

juce::AudioProcessorValueTreeState::ParameterLayout TwoNistAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterInt>("root", "Root", 36, 84, 60));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "pattern",
        "Pattern",
        juce::StringArray {
            "I-V-vi-IV",
            "vi-IV-I-V",
            "I-vi-ii-V",
            "ii-V-I",
            "I-IV-I-V"
        },
        0));
    params.push_back(std::make_unique<juce::AudioParameterInt>("repeats", "Repeats", 1, 8, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>("channel", "Channel", 1, 16, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>("gate", "Gate", 25, 200, 100));
    params.push_back(std::make_unique<juce::AudioParameterInt>("velocity", "Velocity", 1, 127, 100));
    return { params.begin(), params.end() };
}

void TwoNistAudioProcessor::parameterChanged(const juce::String&, float)
{
    arrangementDirty.store(true);
}

void TwoNistAudioProcessor::rebuildArrangementFromParams()
{
    using namespace twonist::engine;

    const int root = juce::roundToInt(parameters.getRawParameterValue("root")->load());
    const int patternIdx = juce::roundToInt(parameters.getRawParameterValue("pattern")->load());
    const int repeats = juce::roundToInt(parameters.getRawParameterValue("repeats")->load());
    const int channel = juce::roundToInt(parameters.getRawParameterValue("channel")->load());
    const int gate = juce::roundToInt(parameters.getRawParameterValue("gate")->load());
    const int velocity = juce::roundToInt(parameters.getRawParameterValue("velocity")->load());

    const char* patternIds[] {
        "pop_1-5-6-4",
        "sensitive_vi-iv-i-v",
        "50s_i-vi-ii-v",
        "jazz_ii-v-i",
        "blues_12bar"
    };
    const int patternCount = 5;
    const int safePatternIdx = juce::jlimit(0, patternCount - 1, patternIdx);

    const auto progressionOpt = applyPattern(patternIds[safePatternIdx], ApplyPatternOptions {
        root,
        1,
        0,
        0
    });
    if (!progressionOpt.has_value())
    {
        arrangedEvents.clear();
        arrangementLengthBeats = 1.0;
        return;
    }

    Section section;
    section.id = "sec-main";
    section.name = "Main";
    section.progression = *progressionOpt;
    section.repeats = juce::jmax(1, repeats);

    ArrangementBlock block;
    block.id = "blk-main";
    block.sourceId = section.id;
    block.mode = "harmony";
    block.midiChannel = juce::jlimit(1, 16, channel);
    block.startBeat = 0.0;
    block.repeats = section.repeats;
    block.lengthBeats = 0.0;
    block.label = "Main";

    arrangedEvents = buildArrangedChordEvents({ section }, { block });
    for (auto& event : arrangedEvents)
    {
        event.velocity = juce::jlimit(1, 127, velocity);
        event.gatePercent = juce::jlimit(1, 200, gate);
    }

    arrangementLengthBeats = 0.0;
    for (const auto& event : arrangedEvents)
        arrangementLengthBeats = std::max(arrangementLengthBeats, event.startBeat + event.durationBeats);
    arrangementLengthBeats = std::max(1.0, arrangementLengthBeats);
}

juce::String TwoNistAudioProcessor::getSchedulerStatusText() const
{
    const auto playingText = statusIsPlaying.load() ? "Playing" : "Stopped";
    return juce::String::formatted(
        "Transport: %s | BPM: %.2f | Beat: %.2f | Active: %d",
        playingText,
        static_cast<double>(statusBpm.load()),
        static_cast<double>(statusBeat.load()),
        statusActiveNotes.load());
}

void TwoNistAudioProcessor::scheduleSegment(
    const BeatRange& segment,
    double bpm,
    int blockSamples,
    std::vector<ScheduledMidi>& scheduled) const
{
    if (arrangedEvents.empty() || arrangementLengthBeats <= 0.0 || segment.endBeat <= segment.startBeat)
        return;

    const double samplesPerBeat = (sampleRateHz * 60.0) / std::max(1.0, bpm);
    const double strumBeatsPerMs = bpm / 60000.0;

    for (const auto& event : arrangedEvents)
    {
        const double gateScale = std::max(0.01, static_cast<double>(event.gatePercent) / 100.0);
        const int channel = juce::jlimit(1, 16, event.midiChannel.value_or(1));

        const int minK = static_cast<int>(std::floor((segment.startBeat - event.startBeat) / arrangementLengthBeats)) - 1;
        const int maxK = static_cast<int>(std::floor((segment.endBeat - event.startBeat) / arrangementLengthBeats)) + 1;

        for (int k = minK; k <= maxK; ++k)
        {
            const double baseStart = event.startBeat + (static_cast<double>(k) * arrangementLengthBeats);

            for (size_t noteIndex = 0; noteIndex < event.notes.size(); ++noteIndex)
            {
                const double strumBeatOffset = static_cast<double>(event.strumMs) * static_cast<double>(noteIndex) * strumBeatsPerMs;
                const double noteOnBeat = baseStart + strumBeatOffset;
                const double noteOffBeat = noteOnBeat + (event.durationBeats * gateScale);

                if (noteOnBeat >= segment.startBeat && noteOnBeat < segment.endBeat)
                {
                    const int sample = segment.sampleOffset + static_cast<int>(std::round((noteOnBeat - segment.startBeat) * samplesPerBeat));
                    scheduled.push_back({
                        juce::jlimit(0, std::max(0, blockSamples - 1), sample),
                        true,
                        channel,
                        juce::jlimit(0, 127, event.notes[noteIndex]),
                        juce::jlimit(1, 127, event.velocity)
                    });
                }

                if (noteOffBeat >= segment.startBeat && noteOffBeat < segment.endBeat)
                {
                    const int sample = segment.sampleOffset + static_cast<int>(std::round((noteOffBeat - segment.startBeat) * samplesPerBeat));
                    scheduled.push_back({
                        juce::jlimit(0, std::max(0, blockSamples - 1), sample),
                        false,
                        channel,
                        juce::jlimit(0, 127, event.notes[noteIndex]),
                        0
                    });
                }
            }
        }
    }
}
