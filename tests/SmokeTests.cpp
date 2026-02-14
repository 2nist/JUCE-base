#include <juce_core/juce_core.h>

int main()
{
    return juce::SystemStats::getNumCpus() > 0 ? 0 : 1;
}
