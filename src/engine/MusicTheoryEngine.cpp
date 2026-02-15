#include "MusicTheoryEngine.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <map>

namespace twonist::engine
{
namespace
{
int clampMidi(int value)
{
    return std::max(0, std::min(127, value));
}
} // namespace

int parseDegree(const std::string& degree)
{
    if (degree.empty())
        return 0;

    int accidentalOffset = 0;
    size_t index = 0;
    while (index < degree.size() && (degree[index] == 'b' || degree[index] == '#'))
    {
        accidentalOffset += degree[index] == '#' ? 1 : -1;
        ++index;
    }

    if (index >= degree.size())
        return accidentalOffset;

    int value = 0;
    for (; index < degree.size(); ++index)
    {
        const auto c = static_cast<unsigned char>(degree[index]);
        if (!std::isdigit(c))
            return accidentalOffset;
        value = (value * 10) + (degree[index] - '0');
    }

    if (value <= 0)
        return accidentalOffset;

    constexpr std::array<int, 7> majorScaleSemitones { 0, 2, 4, 5, 7, 9, 11 };
    const int degreeZeroBased = value - 1;
    const int octave = degreeZeroBased / 7;
    const int scaleIndex = degreeZeroBased % 7;

    return (octave * 12) + majorScaleSemitones[scaleIndex] + accidentalOffset;
}

std::vector<int> getChordNotes(int root, const std::string& quality)
{
    static const std::map<std::string, std::vector<int>> formulas {
        { "Maj", { 0, 4, 7 } },       { "min", { 0, 3, 7 } },       { "dim", { 0, 3, 6 } },
        { "aug", { 0, 4, 8 } },       { "sus2", { 0, 2, 7 } },      { "sus4", { 0, 5, 7 } },
        { "Maj7", { 0, 4, 7, 11 } },  { "min7", { 0, 3, 7, 10 } },  { "dom7", { 0, 4, 7, 10 } },
        { "dim7", { 0, 3, 6, 9 } },   { "hdim7", { 0, 3, 6, 10 } }, { "minMaj7", { 0, 3, 7, 11 } },
        { "aug7", { 0, 4, 8, 10 } },  { "Maj9", { 0, 4, 7, 11, 14 } },
        { "min9", { 0, 3, 7, 10, 14 } }, { "dom9", { 0, 4, 7, 10, 14 } },
        { "Maj11", { 0, 4, 7, 11, 14, 17 } }, { "min11", { 0, 3, 7, 10, 14, 17 } },
        { "dom11", { 0, 4, 7, 10, 14, 17 } }, { "Maj13", { 0, 4, 7, 11, 14, 17, 21 } },
        { "min13", { 0, 3, 7, 10, 14, 17, 21 } }, { "dom13", { 0, 4, 7, 10, 14, 17, 21 } }
    };

    auto it = formulas.find(quality);
    if (it == formulas.end())
        it = formulas.find("Maj");

    std::vector<int> notes;
    notes.reserve(it->second.size());
    for (const auto interval : it->second)
        notes.push_back(clampMidi(root + interval));
    return notes;
}

std::vector<int> applyInversion(const std::vector<int>& notes, int inversion)
{
    if (notes.empty() || inversion <= 0)
        return notes;

    std::vector<int> out = notes;
    std::sort(out.begin(), out.end());

    for (int i = 0; i < inversion; ++i)
    {
        if (out.empty())
            break;
        auto n = out.front();
        out.erase(out.begin());
        out.push_back(n + 12);
    }
    return out;
}

std::vector<int> applyDrop(const std::vector<int>& notes, int drop)
{
    if (notes.empty() || drop == 0)
        return notes;

    std::vector<int> out = notes;
    std::sort(out.begin(), out.end());

    const auto lowerAt = [&](int idxFromTop)
    {
        const int idx = static_cast<int>(out.size()) - idxFromTop;
        if (idx >= 0 && idx < static_cast<int>(out.size()))
            out[static_cast<size_t>(idx)] -= 12;
    };

    if (drop == 2 || drop == 23)
        lowerAt(2);
    if (drop == 3 || drop == 23)
        lowerAt(3);

    return out;
}

std::vector<int> applyVoicing(const std::vector<int>& notes, int inversion, int drop)
{
    return applyDrop(applyInversion(notes, inversion), drop);
}
} // namespace twonist::engine

