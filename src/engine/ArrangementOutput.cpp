#include "ArrangementOutput.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace twonist::engine
{
namespace
{
int clampMidi(int value)
{
    return std::max(0, std::min(127, static_cast<int>(std::lround(value))));
}

double safeDuration(double duration)
{
    const double source = duration > 0.0 ? duration : 0.25;
    return std::max(0.25, source);
}

const Progression* getSectionProgressionForMode(const Section& section, const std::string& mode)
{
    const auto it = section.modeProgressions.find(mode);
    if (it != section.modeProgressions.end())
        return &it->second;
    if (mode == "harmony")
        return &section.progression;
    return nullptr;
}

std::vector<uint8_t> encodeVarLen(int value)
{
    int buffer = value & 0x7f;
    std::vector<uint8_t> bytes;
    while ((value >>= 7) != 0)
    {
        buffer <<= 8;
        buffer |= ((value & 0x7f) | 0x80);
    }

    while (true)
    {
        bytes.push_back(static_cast<uint8_t>(buffer & 0xff));
        if ((buffer & 0x80) == 0)
            break;
        buffer >>= 8;
    }
    return bytes;
}

void pushU32(std::vector<uint8_t>& target, uint32_t value)
{
    target.push_back(static_cast<uint8_t>((value >> 24) & 0xff));
    target.push_back(static_cast<uint8_t>((value >> 16) & 0xff));
    target.push_back(static_cast<uint8_t>((value >> 8) & 0xff));
    target.push_back(static_cast<uint8_t>(value & 0xff));
}

struct TimeSignature
{
    int numerator = 4;
    int denominatorPow = 2;
};

TimeSignature parseTimeSignature(const std::string& signature)
{
    const auto slashPos = signature.find('/');
    if (slashPos == std::string::npos)
        return {};

    const auto numerator = std::max(1, std::stoi(signature.substr(0, slashPos)));
    const auto denominator = std::max(1, std::stoi(signature.substr(slashPos + 1)));
    const auto denominatorPow = std::max(0, static_cast<int>(std::lround(std::log2(static_cast<double>(denominator)))));
    return { numerator, denominatorPow };
}
} // namespace

std::vector<ArrangedChordEvent> buildArrangedChordEvents(
    const std::vector<Section>& sections,
    const std::vector<ArrangementBlock>& blocks)
{
    std::map<std::string, const Section*> sectionMap;
    for (const auto& section : sections)
        sectionMap[section.id] = &section;

    std::vector<ArrangementBlock> sortedBlocks = blocks;
    std::sort(sortedBlocks.begin(), sortedBlocks.end(), [](const ArrangementBlock& a, const ArrangementBlock& b) {
        return a.startBeat < b.startBeat;
    });

    std::vector<ArrangedChordEvent> events;

    for (const auto& block : sortedBlocks)
    {
        const auto sectionIt = sectionMap.find(block.sourceId);
        if (sectionIt == sectionMap.end())
            continue;

        const auto* section = sectionIt->second;
        const auto* sectionProgression = getSectionProgressionForMode(*section, block.mode);
        if (sectionProgression == nullptr)
            continue;

        const int blockRepeats = std::max(1, block.repeats > 0 ? block.repeats : section->repeats);
        double sectionLength = 0.0;
        for (const auto& chord : *sectionProgression)
            sectionLength += safeDuration(chord.duration);

        for (int repeatIndex = 0; repeatIndex < blockRepeats; ++repeatIndex)
        {
            const double repeatOffset = static_cast<double>(repeatIndex) * sectionLength;
            double chordCursor = 0.0;

            for (size_t chordIndex = 0; chordIndex < sectionProgression->size(); ++chordIndex)
            {
                const auto& chord = (*sectionProgression)[chordIndex];
                const auto durationBeats = safeDuration(chord.duration);
                const int beatIndex = std::max(0, static_cast<int>(std::floor(chordCursor)));

                const auto readAt = [&](const std::vector<int>& values, int fallback)
                {
                    return beatIndex < static_cast<int>(values.size()) ? values[static_cast<size_t>(beatIndex)] : fallback;
                };

                ArrangedChordEvent event;
                event.blockId = block.id;
                event.sectionId = section->id;
                event.sectionName = section->name.empty() ? "Section" : section->name;
                event.mode = block.mode;
                event.midiChannel = block.midiChannel;
                event.startBeat = block.startBeat + repeatOffset + chordCursor;
                event.durationBeats = durationBeats;
                event.notes.reserve(chord.notes.size());
                for (const auto note : chord.notes)
                    event.notes.push_back(clampMidi(note));
                event.velocity = clampMidi(readAt(chord.metadata.velocities, 100));
                event.gatePercent = std::max(1, std::min(200, readAt(chord.metadata.gate, 100)));
                event.strumMs = std::max(0, readAt(chord.metadata.strum, 0));
                event.chordIndex = static_cast<int>(chordIndex);

                events.push_back(std::move(event));
                chordCursor += durationBeats;
            }
        }
    }

    std::sort(events.begin(), events.end(), [](const ArrangedChordEvent& a, const ArrangedChordEvent& b) {
        return a.startBeat < b.startBeat;
    });

    return events;
}

std::vector<OscProgressionChord> toOscProgression(const std::vector<ArrangedChordEvent>& events)
{
    if (events.empty())
        return {};

    std::vector<ArrangedChordEvent> sorted = events;
    std::sort(sorted.begin(), sorted.end(), [](const ArrangedChordEvent& a, const ArrangedChordEvent& b) {
        return a.startBeat < b.startBeat;
    });

    std::vector<OscProgressionChord> progression;
    double cursor = 0.0;

    for (const auto& event : sorted)
    {
        if (event.startBeat > cursor)
        {
            progression.push_back({ {}, event.startBeat - cursor });
            cursor = event.startBeat;
        }

        progression.push_back({ event.notes, event.durationBeats });
        cursor += event.durationBeats;
    }

    return progression;
}

std::vector<uint8_t> toMidiFileBytes(
    const std::vector<ArrangedChordEvent>& events,
    double tempo,
    const std::string& timeSignature,
    int ppq)
{
    const auto safeTempo = std::max(1.0, tempo);
    const auto usPerQuarter = std::max(1, static_cast<int>(std::lround(60000000.0 / safeTempo)));
    const auto ts = parseTimeSignature(timeSignature);

    struct MidiEvent
    {
        int tick;
        uint8_t status;
        uint8_t data1;
        uint8_t data2;
    };
    std::vector<MidiEvent> timeline;

    for (const auto& event : events)
    {
        const auto startTick = std::max(0, static_cast<int>(std::lround(event.startBeat * ppq)));
        const auto gateScale = std::max(0.01, static_cast<double>(event.gatePercent) / 100.0);
        const auto durationTicks = std::max(1, static_cast<int>(std::lround(event.durationBeats * ppq * gateScale)));
        const auto endTick = startTick + durationTicks;

        for (const auto note : event.notes)
        {
            timeline.push_back({ startTick, 0x90, static_cast<uint8_t>(clampMidi(note)), static_cast<uint8_t>(clampMidi(event.velocity)) });
            timeline.push_back({ endTick, 0x80, static_cast<uint8_t>(clampMidi(note)), 0x00 });
        }
    }

    std::sort(timeline.begin(), timeline.end(), [](const MidiEvent& a, const MidiEvent& b) {
        if (a.tick != b.tick)
            return a.tick < b.tick;
        return a.status < b.status;
    });

    std::vector<uint8_t> trackData;
    const auto pushMeta = [&](int delta, uint8_t type, std::initializer_list<uint8_t> data)
    {
        const auto deltaBytes = encodeVarLen(delta);
        trackData.insert(trackData.end(), deltaBytes.begin(), deltaBytes.end());
        trackData.push_back(0xff);
        trackData.push_back(type);
        trackData.push_back(static_cast<uint8_t>(data.size()));
        trackData.insert(trackData.end(), data.begin(), data.end());
    };

    pushMeta(0, 0x51, {
        static_cast<uint8_t>((usPerQuarter >> 16) & 0xff),
        static_cast<uint8_t>((usPerQuarter >> 8) & 0xff),
        static_cast<uint8_t>(usPerQuarter & 0xff)
    });
    pushMeta(0, 0x58, {
        static_cast<uint8_t>(ts.numerator & 0xff),
        static_cast<uint8_t>(ts.denominatorPow & 0xff),
        24,
        8
    });

    int lastTick = 0;
    for (const auto& event : timeline)
    {
        const auto delta = std::max(0, event.tick - lastTick);
        const auto deltaBytes = encodeVarLen(delta);
        trackData.insert(trackData.end(), deltaBytes.begin(), deltaBytes.end());
        trackData.push_back(event.status);
        trackData.push_back(event.data1);
        trackData.push_back(event.data2);
        lastTick = event.tick;
    }

    trackData.push_back(0x00);
    trackData.push_back(0xff);
    trackData.push_back(0x2f);
    trackData.push_back(0x00);

    std::vector<uint8_t> bytes;
    bytes.push_back(0x4d);
    bytes.push_back(0x54);
    bytes.push_back(0x68);
    bytes.push_back(0x64);
    pushU32(bytes, 6);
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    bytes.push_back(0x01);
    bytes.push_back(static_cast<uint8_t>((ppq >> 8) & 0xff));
    bytes.push_back(static_cast<uint8_t>(ppq & 0xff));

    bytes.push_back(0x4d);
    bytes.push_back(0x54);
    bytes.push_back(0x72);
    bytes.push_back(0x6b);
    pushU32(bytes, static_cast<uint32_t>(trackData.size()));
    bytes.insert(bytes.end(), trackData.begin(), trackData.end());

    return bytes;
}
} // namespace twonist::engine

