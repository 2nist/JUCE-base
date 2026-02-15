#include "engine/ArrangementOutput.h"
#include "engine/ProgressionManager.h"

#include <iostream>
#include <stdexcept>

using namespace twonist::engine;

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void testArrangementChannels()
{
    Section sectionA;
    sectionA.id = "sec-a";
    sectionA.name = "Verse";
    sectionA.progression = {
        { { 60, 64, 67 }, 1.0, { {}, {}, {}, {}, {}, { 96 }, { 0 }, { 120 } } },
        { { 62, 65, 69 }, 1.0, { {}, {}, {}, {}, {}, { 102 }, { 5 }, { 90 } } }
    };

    Section sectionB = sectionA;
    sectionB.id = "sec-b";
    sectionB.name = "Drums";
    sectionB.modeProgressions["drum"] = {
        { { 36 }, 2.0, { {}, {}, {}, {}, {}, { 110, 110 }, { 0, 0 }, { 100, 100 } } }
    };

    ArrangementBlock blockA { "blk-a", "sec-a", "harmony", 3, 0.0, 2.0, "Verse", 1 };
    ArrangementBlock blockB { "blk-b", "sec-b", "drum", 10, 2.0, 2.0, "Drums", 1 };
    const auto events = buildArrangedChordEvents({ sectionA, sectionB }, { blockA, blockB });

    require(events.size() == 3, "Expected 3 arranged events.");
    require(events[0].midiChannel.has_value() && *events[0].midiChannel == 3, "Expected block A MIDI channel to be 3.");
    require(events[2].midiChannel.has_value() && *events[2].midiChannel == 10, "Expected block B MIDI channel to be 10.");
    require(events[2].startBeat == 2.0, "Expected drum event to start at beat 2.");
}

void testOscProgressionGap()
{
    std::vector<ArrangedChordEvent> events {
        { "b1", "s1", "A", "harmony", 1, 0.0, 1.0, { 60, 64, 67 }, 100, 100, 0, 0 },
        { "b2", "s2", "B", "harmony", 1, 3.0, 1.0, { 65, 69, 72 }, 100, 100, 0, 0 }
    };

    const auto progression = toOscProgression(events);
    require(progression.size() == 3, "Expected 3 OSC progression events.");
    require(progression[1].notes.empty(), "Expected middle OSC event to be a gap.");
    require(progression[1].duration == 2.0, "Expected gap duration to be 2 beats.");
}

void testMidiHeader()
{
    Section section;
    section.id = "sec-a";
    section.name = "Verse";
    section.progression = {
        { { 60, 64, 67 }, 1.0, { {}, {}, {}, {}, {}, { 96 }, { 0 }, { 120 } } },
        { { 62, 65, 69 }, 1.0, { {}, {}, {}, {}, {}, { 102 }, { 5 }, { 90 } } }
    };

    ArrangementBlock block { "blk-a", "sec-a", "harmony", 1, 0.0, 2.0, "Verse", 1 };
    const auto events = buildArrangedChordEvents({ section }, { block });
    const auto bytes = toMidiFileBytes(events, 120.0, "4/4");

    require(bytes.size() > 20, "Expected MIDI bytes > 20.");
    require(bytes[0] == 0x4d && bytes[1] == 0x54 && bytes[2] == 0x68 && bytes[3] == 0x64, "Expected MThd header.");
}

void testPatternDetectionAndApply()
{
    auto progressionOpt = applyPattern("pop_1-5-6-4", ApplyPatternOptions { 60, 4, 0, 0 });
    require(progressionOpt.has_value(), "Expected pop pattern to apply.");
    const auto progression = *progressionOpt;
    require(progression.size() == 4, "Expected 4 chords in pop pattern.");

    const auto matches = detectPatterns(progression, 60);
    require(!matches.empty(), "Expected at least one detected pattern.");
}
} // namespace

int main()
{
    try
    {
        testArrangementChannels();
        testOscProgressionGap();
        testMidiHeader();
        testPatternDetectionAndApply();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Engine parity tests failed: " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "Engine parity tests passed." << std::endl;
    return 0;
}
