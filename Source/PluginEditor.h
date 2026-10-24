#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class FLMMVoxCleanAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit FLMMVoxCleanAudioProcessorEditor(FLMMVoxCleanAudioProcessor&);
    ~FLMMVoxCleanAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawSpectrum(juce::Graphics&, juce::Rectangle<int> area);
    FLMMVoxCleanAudioProcessor& audioProcessor;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::Label title, subtitle, denoiseTitle, eqTitle, analyzerTitle;
    juce::ToggleButton deepClean { "DEEP CLEAN" };
    juce::Slider threshold, strength, hpFreq, lowMidFreq, lowMidGain, presenceFreq, presenceGain, airFreq, airGain, output;
    juce::Label thresholdLabel, strengthLabel, hpLabel, lowMidFreqLabel, lowMidGainLabel, presenceFreqLabel, presenceGainLabel, airFreqLabel, airGainLabel, outputLabel;
    std::unique_ptr<ButtonAttachment> deepAttachment;
    std::unique_ptr<SliderAttachment> thresholdAttachment, strengthAttachment, hpAttachment, lowMidFreqAttachment, lowMidGainAttachment, presenceFreqAttachment, presenceGainAttachment, airFreqAttachment, airGainAttachment, outputAttachment;
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, fftSize * 2> fftData {};
    std::array<float, fftSize> fifo {};
    int fifoIndex = 0;
    std::array<float, fftSize / 2> spectrum {};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FLMMVoxCleanAudioProcessorEditor)
};
