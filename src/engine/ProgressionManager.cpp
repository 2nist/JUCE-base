#include "ProgressionManager.h"

#include "MusicTheoryEngine.h"

#include <algorithm>
#include <cctype>

namespace twonist::engine
{
namespace
{
int normalizeDegree(const std::string& degree)
{
    const int semitone = parseDegree(degree);
    return ((semitone % 12) + 12) % 12;
}

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
} // namespace

std::vector<Pattern> getDefaultPatterns()
{
    std::vector<Pattern> patterns {
        { "pop_1-5-6-4", "I-V-vi-IV (Pop Cadence)", "Classic pop progression with a hopeful resolution", { "1", "5", "6", "4" }, { "Maj", "Maj", "min", "Maj" } },
        { "sensitive_vi-iv-i-v", "vi-IV-I-V (Sensitive Loop)", "Emotionally charged loop used in ballads", { "6", "4", "1", "5" }, { "min", "Maj", "Maj", "Maj" } },
        { "50s_i-vi-ii-v", "I-vi-ii-V (50s Turnaround)", "Jazz/blues turnaround popular in classic progressions", { "1", "6", "2", "5" }, { "Maj", "min", "min", "Maj" } },
        { "jazz_ii-v-i", "ii-V-I (Jazz Cadence)", "The most common progression in jazz", { "2", "5", "1" }, { "min7", "dom7", "Maj7" } },
        { "blues_12bar", "I-IV-I-V (12-Bar Blues)", "Foundation of blues music", { "1", "4", "1", "5" }, { "dom7", "dom7", "dom7", "dom7" } },
        { "andalusian_cadence", "vi-V-IV-III (Andalusian)", "Descending progression common in flamenco", { "6", "5", "4", "3" }, { "min", "Maj", "Maj", "Maj" } },
        { "circle_of_fifths", "vi-ii-V-I (Circle Progression)", "Follows the circle of fifths", { "6", "2", "5", "1" }, { "min7", "min7", "dom7", "Maj7" } }
    };

    for (auto& pattern : patterns)
    {
        pattern.semitones.clear();
        for (const auto& degree : pattern.degrees)
            pattern.semitones.push_back(normalizeDegree(degree));
    }
    return patterns;
}

std::vector<Pattern> getPatternDefinitions(const std::vector<Pattern>& customPatterns)
{
    auto all = getDefaultPatterns();
    all.insert(all.end(), customPatterns.begin(), customPatterns.end());
    return all;
}

std::optional<Pattern> findPattern(const std::string& idOrName, const std::vector<Pattern>& customPatterns)
{
    if (idOrName.empty())
        return std::nullopt;

    const auto needle = toLower(idOrName);
    const auto all = getPatternDefinitions(customPatterns);
    for (const auto& pattern : all)
    {
        if (toLower(pattern.id) == needle || toLower(pattern.name) == needle)
            return pattern;
    }
    return std::nullopt;
}

std::vector<DetectedPattern> detectPatterns(const Progression& progression, std::optional<int> keyRoot)
{
    if (progression.empty())
        return {};

    int baseRoot = keyRoot.has_value() ? *keyRoot : 60;
    if (!progression.front().notes.empty())
        baseRoot = progression.front().notes.front();

    std::vector<int> offsets;
    offsets.reserve(progression.size());
    for (const auto& chord : progression)
    {
        if (chord.notes.empty())
            continue;
        const int note = chord.notes.front();
        offsets.push_back(((note - baseRoot) % 12 + 12) % 12);
    }

    if (offsets.size() < 2)
        return {};

    std::vector<DetectedPattern> matches;
    for (const auto& pattern : getDefaultPatterns())
    {
        if (pattern.semitones.empty() || pattern.semitones.size() > offsets.size())
            continue;

        for (size_t start = 0; start <= offsets.size() - pattern.semitones.size(); ++start)
        {
            bool match = true;
            for (size_t i = 0; i < pattern.semitones.size(); ++i)
            {
                if (pattern.semitones[i] != offsets[start + i])
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                matches.push_back({
                    pattern.id,
                    pattern.name,
                    pattern.description,
                    static_cast<int>(start),
                    static_cast<int>(pattern.semitones.size()),
                    baseRoot
                });
            }
        }
    }

    return matches;
}

std::optional<Progression> applyPattern(
    const std::string& patternId,
    const ApplyPatternOptions& options,
    const std::vector<Pattern>& customPatterns)
{
    const auto patternOpt = findPattern(patternId, customPatterns);
    if (!patternOpt.has_value())
        return std::nullopt;

    const auto& pattern = *patternOpt;
    Progression progression;
    progression.reserve(pattern.degrees.size());

    for (size_t i = 0; i < pattern.degrees.size(); ++i)
    {
        const std::string quality =
            i < pattern.qualities.size() ? pattern.qualities[i] : "Maj";
        const int chordRoot = options.root + normalizeDegree(pattern.degrees[i]);
        auto notes = getChordNotes(chordRoot, quality);
        notes = applyVoicing(notes, options.inversion, options.drop);

        Chord chord;
        chord.notes = notes;
        chord.duration = static_cast<double>(options.duration > 0 ? options.duration : pattern.duration);
        chord.metadata.root = chordRoot;
        chord.metadata.quality = quality;
        chord.metadata.inversion = options.inversion;
        chord.metadata.drop = options.drop;
        progression.push_back(chord);
    }

    return progression;
}
} // namespace twonist::engine

