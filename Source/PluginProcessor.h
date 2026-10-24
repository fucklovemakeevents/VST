#pragma once
#include <JuceHeader.h>

class FLMMVoxCleanAudioProcessor : public juce::AudioProcessor
{
public:
    FLMMVoxCleanAudioProcessor();
    ~FLMMVoxCleanAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Fuck Love Make Music Vox Clean"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void pushAnalyserSample(float sample) noexcept;
    bool popAnalyserSample(float& sample) noexcept;

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPass;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowMid;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> presence;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> air;
    double currentSampleRate = 44100.0;
    float envelope = 0.0f;
    static constexpr int fifoSize = 8192;
    juce::AbstractFifo analyserFifo { fifoSize };
    std::array<float, fifoSize> analyserData {};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FLMMVoxCleanAudioProcessor)
};
