#include "PluginProcessor.h"
#include "PluginEditor.h"

TwoNistAudioProcessor::TwoNistAudioProcessor()
    : AudioProcessor(BusesProperties())
{
}

void TwoNistAudioProcessor::prepareToPlay(double, int)
{
}

void TwoNistAudioProcessor::releaseResources()
{
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

    juce::MidiBuffer processed;
    for (const auto metadata : midiMessages)
        processed.addEvent(metadata.getMessage(), metadata.samplePosition);

    midiMessages.swapWith(processed);
}

juce::AudioProcessorEditor* TwoNistAudioProcessor::createEditor()
{
    return new TwoNistAudioProcessorEditor(*this);
}

bool TwoNistAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String TwoNistAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TwoNistAudioProcessor::acceptsMidi() const
{
    return true;
}

bool TwoNistAudioProcessor::producesMidi() const
{
    return true;
}

bool TwoNistAudioProcessor::isMidiEffect() const
{
    return true;
}

double TwoNistAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TwoNistAudioProcessor::getNumPrograms()
{
    return 1;
}

int TwoNistAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TwoNistAudioProcessor::setCurrentProgram(int)
{
}

const juce::String TwoNistAudioProcessor::getProgramName(int)
{
    return {};
}

void TwoNistAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void TwoNistAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeInt(1);
}

void TwoNistAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    juce::ignoreUnused(stream.readInt());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TwoNistAudioProcessor();
}

