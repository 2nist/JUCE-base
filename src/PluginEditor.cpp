#include "PluginEditor.h"

TwoNistAudioProcessorEditor::TwoNistAudioProcessorEditor(TwoNistAudioProcessor& processor)
    : AudioProcessorEditor(&processor), audioProcessor(processor)
{
    title.setText("2nist MIDI Arranger - B.5", juce::dontSendNotification);
    title.setFont(juce::FontOptions(20.0f));
    title.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title);

    status.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(status);

    transportLamp.setJustificationType(juce::Justification::centred);
    transportLamp.setColour(juce::Label::backgroundColourId, juce::Colours::darkred);
    transportLamp.setColour(juce::Label::textColourId, juce::Colours::white);
    transportLamp.setText("STOP", juce::dontSendNotification);
    addAndMakeVisible(transportLamp);

    paramsHeader.setText("Generator Parameters", juce::dontSendNotification);
    paramsHeader.setFont(juce::FontOptions(16.0f));
    addAndMakeVisible(paramsHeader);

    transportHeader.setText("Transport / Safety", juce::dontSendNotification);
    transportHeader.setFont(juce::FontOptions(16.0f));
    addAndMakeVisible(transportHeader);

    panicButton.onClick = [this] { audioProcessor.requestPanic(); };
    addAndMakeVisible(panicButton);

    auto configureSlider = [](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    };

    configureSlider(rootSlider);
    configureSlider(repeatsSlider);
    configureSlider(channelSlider);
    configureSlider(gateSlider);
    configureSlider(velocitySlider);

    patternBox.addItemList({ "I-V-vi-IV", "vi-IV-I-V", "I-vi-ii-V", "ii-V-I", "I-IV-I-V" }, 1);

    const auto setupLabel = [this](juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(label);
    };
    setupLabel(rootLabel, "Root");
    setupLabel(patternLabel, "Pattern");
    setupLabel(repeatsLabel, "Repeats");
    setupLabel(channelLabel, "MIDI Ch");
    setupLabel(gateLabel, "Gate %");
    setupLabel(velocityLabel, "Velocity");

    addAndMakeVisible(rootSlider);
    addAndMakeVisible(patternBox);
    addAndMakeVisible(repeatsSlider);
    addAndMakeVisible(channelSlider);
    addAndMakeVisible(gateSlider);
    addAndMakeVisible(velocitySlider);

    auto& vts = audioProcessor.getValueTreeState();
    rootAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "root", rootSlider);
    patternAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(vts, "pattern", patternBox);
    repeatsAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "repeats", repeatsSlider);
    channelAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "channel", channelSlider);
    gateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "gate", gateSlider);
    velocityAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "velocity", velocitySlider);

    startTimerHz(12);
    setSize(780, 420);
}

void TwoNistAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withBrightness(0.12f));
    auto bounds = getLocalBounds().reduced(10);
    g.setColour(juce::Colours::darkgrey);
    g.drawRoundedRectangle(bounds.removeFromTop(80).toFloat(), 6.0f, 1.0f);
    g.drawRoundedRectangle(getLocalBounds().reduced(10).withTrimmedTop(90).toFloat(), 6.0f, 1.0f);
    g.setColour(juce::Colours::whitesmoke);
    g.drawRect(getLocalBounds(), 1);
}

void TwoNistAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    title.setBounds(area.removeFromTop(32));
    auto statusRow = area.removeFromTop(28);
    transportLamp.setBounds(statusRow.removeFromRight(80));
    status.setBounds(statusRow);
    transportHeader.setBounds(area.removeFromTop(24));
    auto transportRow = area.removeFromTop(32);
    panicButton.setBounds(transportRow.removeFromLeft(120));
    area.removeFromTop(6);
    paramsHeader.setBounds(area.removeFromTop(24));
    area.removeFromTop(4);

    const int rowH = 34;
    const auto layoutRow = [&](juce::Label& label, juce::Component& control)
    {
        auto row = area.removeFromTop(rowH);
        label.setBounds(row.removeFromLeft(120));
        control.setBounds(row.removeFromLeft(520));
    };

    layoutRow(rootLabel, rootSlider);
    layoutRow(patternLabel, patternBox);
    layoutRow(repeatsLabel, repeatsSlider);
    layoutRow(channelLabel, channelSlider);
    layoutRow(gateLabel, gateSlider);
    layoutRow(velocityLabel, velocitySlider);
}

void TwoNistAudioProcessorEditor::timerCallback()
{
    const auto text = audioProcessor.getSchedulerStatusText();
    status.setText(text, juce::dontSendNotification);
    const bool playing = text.startsWith("Transport: Playing");
    transportLamp.setText(playing ? "PLAY" : "STOP", juce::dontSendNotification);
    transportLamp.setColour(
        juce::Label::backgroundColourId,
        playing ? juce::Colours::darkgreen : juce::Colours::darkred);
}
