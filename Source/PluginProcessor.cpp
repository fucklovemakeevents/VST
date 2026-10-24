#include "PluginProcessor.h"
#include "PluginEditor.h"

FLMMVoxCleanAudioProcessor::FLMMVoxCleanAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout FLMMVoxCleanAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "deepClean", 1 }, "Deep Clean", false));
    layout.add(std::make_unique<APF>(juce::ParameterID { "threshold", 1 }, "Noise Threshold", juce::NormalisableRange<float>(-70.0f, -20.0f, 0.1f), -52.0f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "strength", 1 }, "Reduction", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.55f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "hpFreq", 1 }, "High Pass", juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.35f), 75.0f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "lowMidFreq", 1 }, "EQ Low-Mid Freq", juce::NormalisableRange<float>(100.0f, 1500.0f, 1.0f, 0.35f), 280.0f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "lowMidGain", 1 }, "EQ Low-Mid Gain", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -1.5f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "presenceFreq", 1 }, "EQ Presence Freq", juce::NormalisableRange<float>(1000.0f, 9000.0f, 1.0f, 0.35f), 3500.0f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "presenceGain", 1 }, "EQ Presence Gain", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.0f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "airFreq", 1 }, "EQ Air Freq", juce::NormalisableRange<float>(5000.0f, 18000.0f, 1.0f, 0.35f), 10500.0f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "airGain", 1 }, "EQ Air Gain", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.5f));
    layout.add(std::make_unique<APF>(juce::ParameterID { "output", 1 }, "Output", juce::NormalisableRange<float>(-18.0f, 6.0f, 0.1f), 0.0f));
    return layout;
}

void FLMMVoxCleanAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumOutputChannels() };
    highPass.prepare(spec); lowMid.prepare(spec); presence.prepare(spec); air.prepare(spec);
    highPass.reset(); lowMid.reset(); presence.reset(); air.reset();
    envelope = 0.0f;
}

void FLMMVoxCleanAudioProcessor::releaseResources() {}

void FLMMVoxCleanAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(midi);
    juce::ScopedNoDenormals noDenormals;
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch) buffer.clear(ch, 0, buffer.getNumSamples());

    const auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, parameters.getRawParameterValue("hpFreq")->load());
    *highPass.state = *hp;
    *lowMid.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, parameters.getRawParameterValue("lowMidFreq")->load(), 0.85f, juce::Decibels::decibelsToGain(parameters.getRawParameterValue("lowMidGain")->load()));
    *presence.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, parameters.getRawParameterValue("presenceFreq")->load(), 1.0f, juce::Decibels::decibelsToGain(parameters.getRawParameterValue("presenceGain")->load()));
    *air.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, parameters.getRawParameterValue("airFreq")->load(), 0.7f, juce::Decibels::decibelsToGain(parameters.getRawParameterValue("airGain")->load()));

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    highPass.process(context); lowMid.process(context); presence.process(context); air.process(context);

    const bool deep = parameters.getRawParameterValue("deepClean")->load() > 0.5f;
    const float thresholdDb = parameters.getRawParameterValue("threshold")->load() + (deep ? 5.0f : 0.0f);
    const float strength = parameters.getRawParameterValue("strength")->load();
    const float threshold = juce::Decibels::decibelsToGain(thresholdDb);
    const float release = std::exp(-1.0f / (0.08f * (float) currentSampleRate));
    const float attack = std::exp(-1.0f / (0.008f * (float) currentSampleRate));

    for (int n = 0; n < buffer.getNumSamples(); ++n)
    {
        float mono = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) mono += std::abs(buffer.getSample(ch, n));
        mono /= (float) juce::jmax(1, buffer.getNumChannels());
        const float coeff = mono > envelope ? attack : release;
        envelope = coeff * envelope + (1.0f - coeff) * mono;
        float gain = 1.0f;
        if (envelope < threshold)
        {
            const float target = juce::jlimit(0.06f, 1.0f, envelope / juce::jmax(threshold, 0.00001f));
            gain = 1.0f - strength * (1.0f - target);
        }
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float s = buffer.getSample(ch, n) * gain;
            s *= juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());
            buffer.setSample(ch, n, s);
        }
        pushAnalyserSample(mono);
    }
}

void FLMMVoxCleanAudioProcessor::pushAnalyserSample(float sample) noexcept
{
    int start1, size1, start2, size2;
    analyserFifo.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0) analyserData[(size_t) start1] = sample;
    else if (size2 > 0) analyserData[(size_t) start2] = sample;
    else return;
    analyserFifo.finishedWrite(size1 + size2);
}

bool FLMMVoxCleanAudioProcessor::popAnalyserSample(float& sample) noexcept
{
    int start1, size1, start2, size2;
    analyserFifo.prepareToRead(1, start1, size1, start2, size2);
    if (size1 > 0) sample = analyserData[(size_t) start1];
    else if (size2 > 0) sample = analyserData[(size_t) start2];
    else return false;
    analyserFifo.finishedRead(size1 + size2);
    return true;
}

void FLMMVoxCleanAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void FLMMVoxCleanAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* FLMMVoxCleanAudioProcessor::createEditor() { return new FLMMVoxCleanAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FLMMVoxCleanAudioProcessor(); }
