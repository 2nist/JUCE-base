#pragma once

#include "Types.h"

#include <optional>
#include <string>
#include <vector>

namespace twonist::engine
{
struct Pattern
{
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> degrees;
    std::vector<std::string> qualities;
    std::vector<int> semitones;
    int duration = 4;
};

struct DetectedPattern
{
    std::string id;
    std::string name;
    std::string description;
    int startIndex = 0;
    int length = 0;
    int root = 60;
};

struct ApplyPatternOptions
{
    int root = 60;
    int duration = 4;
    int inversion = 0;
    int drop = 0;
};

std::vector<Pattern> getDefaultPatterns();
std::vector<Pattern> getPatternDefinitions(const std::vector<Pattern>& customPatterns = {});
std::optional<Pattern> findPattern(const std::string& idOrName, const std::vector<Pattern>& customPatterns = {});
std::vector<DetectedPattern> detectPatterns(const Progression& progression, std::optional<int> keyRoot = std::nullopt);
std::optional<Progression> applyPattern(
    const std::string& patternId,
    const ApplyPatternOptions& options = {},
    const std::vector<Pattern>& customPatterns = {});
} // namespace twonist::engine

