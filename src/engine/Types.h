#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace twonist::engine
{
struct ChordMetadata
{
    std::optional<int> root;
    std::optional<std::string> quality;
    std::optional<int> inversion;
    std::optional<int> drop;
    std::optional<int> degree;
    std::vector<int> velocities;
    std::vector<int> strum;
    std::vector<int> gate;
};

struct Chord
{
    std::vector<int> notes;
    double duration = 1.0;
    ChordMetadata metadata;
};

using Progression = std::vector<Chord>;

struct Section
{
    std::string id;
    std::string name;
    Progression progression;
    std::map<std::string, Progression> modeProgressions;
    int repeats = 1;
    int beatsPerBar = 4;
};

struct ArrangementBlock
{
    std::string id;
    std::string sourceId;
    std::string mode = "harmony";
    std::optional<int> midiChannel;
    double startBeat = 0.0;
    double lengthBeats = 0.0;
    std::string label;
    int repeats = 1;
};

struct ArrangedChordEvent
{
    std::string blockId;
    std::string sectionId;
    std::string sectionName;
    std::string mode;
    std::optional<int> midiChannel;
    double startBeat = 0.0;
    double durationBeats = 0.0;
    std::vector<int> notes;
    int velocity = 100;
    int gatePercent = 100;
    int strumMs = 0;
    int chordIndex = 0;
};

struct OscProgressionChord
{
    std::vector<int> notes;
    double duration = 0.0;
};
} // namespace twonist::engine

