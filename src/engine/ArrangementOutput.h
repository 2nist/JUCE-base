#pragma once

#include "Types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace twonist::engine
{
std::vector<ArrangedChordEvent> buildArrangedChordEvents(
    const std::vector<Section>& sections,
    const std::vector<ArrangementBlock>& blocks);

std::vector<OscProgressionChord> toOscProgression(const std::vector<ArrangedChordEvent>& events);

std::vector<uint8_t> toMidiFileBytes(
    const std::vector<ArrangedChordEvent>& events,
    double tempo = 120.0,
    const std::string& timeSignature = "4/4",
    int ppq = 480);
} // namespace twonist::engine

