#pragma once

#include <string>
#include <vector>

namespace twonist::engine
{
int parseDegree(const std::string& degree);
std::vector<int> getChordNotes(int root, const std::string& quality);
std::vector<int> applyInversion(const std::vector<int>& notes, int inversion);
std::vector<int> applyDrop(const std::vector<int>& notes, int drop);
std::vector<int> applyVoicing(const std::vector<int>& notes, int inversion, int drop);
} // namespace twonist::engine

