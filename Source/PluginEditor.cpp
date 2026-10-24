#include "PluginEditor.h"

namespace
{
    const juce::Colour bg { 0xff101112 };
    const juce::Colour panel { 0xff1b1d1f };
    const juce::Colour lime { 0xffb7f36b };
    const juce::Colour muted { 0xff8b9298 };

    void styleSlider(juce::Slider& s)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 20);
        s.setColour(juce::Slider::rotarySliderFillColourId, lime);
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff393d40));
        s.setColour(juce::Slider::thumbColourId, lime);
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setColour(juce::Slider::textBoxBackgroundColourId, panel);
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }
    void styleLabel(juce::Label& l, float size = 11.0f)
    {
        l.setFont(juce::Font(size, juce::Font::bold));
        l.setColour(juce::Label::textColourId, muted);
        l.setJustificationType(juce::Justification::centred);
    }
}

FLMMVoxCleanAudioProcessorEditor::FLMMVoxCleanAudioProcessorEditor(FLMMVoxCleanAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(1040, 790);
    title.setText("FLMM  /  VOX CLEAN", juce::dontSendNotification);
    title.setFont(juce::Font(25.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colours::white);
    subtitle.setText("FUCK LOVE MAKE MUSIC LABEL   •   VOCAL RESTORATION", juce::dontSendNotification);
    subtitle.setFont(juce::Font(10.0f, juce::Font::bold));
    subtitle.setColour(juce::Label::textColourId, lime);
    denoiseTitle.setText("ADAPTIVE CLEAN", juce::dontSendNotification); styleLabel(denoiseTitle, 13.0f);
    eqTitle.setText("PARAMETRIC EQ", juce::dontSendNotification); styleLabel(eqTitle, 13.0f);
    analyzerTitle.setText("LIVE SPECTRUM  /  INPUT ACTIVITY", juce::dontSendNotification); styleLabel(analyzerTitle, 12.0f);
    for (auto* c : std::array<juce::Component*, 5>{ &title, &subtitle, &denoiseTitle, &eqTitle, &analyzerTitle }) addAndMakeVisible(c);

    deepClean.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    deepClean.setColour(juce::ToggleButton::tickColourId, lime);
    deepClean.setColour(juce::ToggleButton::tickDisabledColourId, muted);
    addAndMakeVisible(deepClean);
    deepAttachment = std::make_unique<ButtonAttachment>(audioProcessor.parameters, "deepClean", deepClean);

    auto setup = [this](juce::Slider& s, juce::Label& l, const juce::String& text, const juce::String& id)
    {
        styleSlider(s); styleLabel(l); l.setText(text, juce::dontSendNotification);
        addAndMakeVisible(s); addAndMakeVisible(l);
        return std::make_unique<SliderAttachment>(audioProcessor.parameters, id, s);
    };
    thresholdAttachment = setup(threshold, thresholdLabel, "THRESHOLD", "threshold");
    strengthAttachment = setup(strength, strengthLabel, "REDUCTION", "strength");
    hpAttachment = setup(hpFreq, hpLabel, "HIGH PASS", "hpFreq");
    lowMidFreqAttachment = setup(lowMidFreq, lowMidFreqLabel, "LOW-MID FREQ", "lowMidFreq");
    lowMidGainAttachment = setup(lowMidGain, lowMidGainLabel, "LOW-MID dB", "lowMidGain");
    presenceFreqAttachment = setup(presenceFreq, presenceFreqLabel, "PRESENCE FREQ", "presenceFreq");
    presenceGainAttachment = setup(presenceGain, presenceGainLabel, "PRESENCE dB", "presenceGain");
    airFreqAttachment = setup(airFreq, airFreqLabel, "AIR FREQ", "airFreq");
    airGainAttachment = setup(airGain, airGainLabel, "AIR dB", "airGain");
    outputAttachment = setup(output, outputLabel, "OUTPUT dB", "output");
    startTimerHz(30);
}

void FLMMVoxCleanAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(lime); g.fillRoundedRectangle(22.0f, 23.0f, 5.0f, 44.0f, 2.0f);
    g.setColour(panel); g.fillRoundedRectangle(22.0f, 112.0f, 996.0f, 190.0f, 12.0f);
    g.setColour(panel); g.fillRoundedRectangle(22.0f, 312.0f, 996.0f, 190.0f, 12.0f);
    g.setColour(panel); g.fillRoundedRectangle(22.0f, 512.0f, 996.0f, 252.0f, 12.0f);
    g.setColour(juce::Colour(0xff2c3033));
    for (int i = 0; i < 11; ++i) g.drawVerticalLine(42 + i * 90, 562.0f, 730.0f);
    for (int i = 0; i < 5; ++i) g.drawHorizontalLine(570 + i * 39, 42.0f, 998.0f);
    drawSpectrum(g, { 42, 562, 956, 168 });
    g.setColour(muted); g.setFont(10.0f);
    g.drawText("20 Hz", 38, 735, 50, 15, juce::Justification::left);
    g.drawText("1 kHz", 500, 735, 50, 15, juce::Justification::centred);
    g.drawText("20 kHz", 944, 735, 50, 15, juce::Justification::right);
    g.setColour(lime); g.setFont(10.0f);
    g.drawText("LIVE MONITOR", 850, 530, 140, 18, juce::Justification::right);
}

void FLMMVoxCleanAudioProcessorEditor::drawSpectrum(juce::Graphics& g, juce::Rectangle<int> area)
{
    juce::Path path;
    path.startNewSubPath((float) area.getX(), (float) area.getBottom());
    const int bins = (int) spectrum.size();
    for (int x = 0; x < area.getWidth(); ++x)
    {
        const float norm = (float) x / (float) juce::jmax(1, area.getWidth() - 1);
        const int bin = juce::jlimit(1, bins - 1, (int) (std::pow(norm, 2.0f) * (float) (bins - 1)));
        const float mag = juce::jlimit(0.0f, 1.0f, spectrum[(size_t) bin] * 6.0f);
        const float y = (float) area.getBottom() - mag * (float) area.getHeight();
        path.lineTo((float) area.getX() + (float) x, y);
    }
    path.lineTo((float) area.getRight(), (float) area.getBottom());
    g.setColour(lime.withAlpha(0.10f)); g.fillPath(path);
    g.setColour(lime); g.strokePath(path, juce::PathStrokeType(1.7f));
}

void FLMMVoxCleanAudioProcessorEditor::resized()
{
    title.setBounds(42, 28, 500, 35); subtitle.setBounds(43, 62, 600, 18);
    denoiseTitle.setBounds(38, 116, 300, 25); deepClean.setBounds(780, 117, 210, 25);
    thresholdLabel.setBounds(50, 145, 160, 18); threshold.setBounds(50, 160, 160, 130);
    strengthLabel.setBounds(220, 145, 160, 18); strength.setBounds(220, 160, 160, 130);
    outputLabel.setBounds(390, 145, 160, 18); output.setBounds(390, 160, 160, 130);
    eqTitle.setBounds(38, 316, 300, 25);
    hpLabel.setBounds(45, 345, 120, 18); hpFreq.setBounds(45, 360, 120, 130);
    lowMidFreqLabel.setBounds(175, 345, 120, 18); lowMidFreq.setBounds(175, 360, 120, 130);
    lowMidGainLabel.setBounds(305, 345, 120, 18); lowMidGain.setBounds(305, 360, 120, 130);
    presenceFreqLabel.setBounds(435, 345, 120, 18); presenceFreq.setBounds(435, 360, 120, 130);
    presenceGainLabel.setBounds(565, 345, 120, 18); presenceGain.setBounds(565, 360, 120, 130);
    airFreqLabel.setBounds(695, 345, 120, 18); airFreq.setBounds(695, 360, 120, 130);
    airGainLabel.setBounds(825, 345, 120, 18); airGain.setBounds(825, 360, 120, 130);
    analyzerTitle.setBounds(42, 525, 500, 20);
}

void FLMMVoxCleanAudioProcessorEditor::timerCallback()
{
    float sample = 0.0f;
    while (audioProcessor.popAnalyserSample(sample))
    {
        fifo[(size_t) fifoIndex++] = sample;
        if (fifoIndex >= fftSize)
        {
            fifoIndex = 0;
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            fft.performFrequencyOnlyForwardTransform(fftData.data());
            for (int i = 0; i < fftSize / 2; ++i)
                spectrum[(size_t) i] = juce::jlimit(0.0f, 1.0f, fftData[(size_t) i] / (float) fftSize * 4.0f);
        }
    }
    repaint(35, 380, 890, 270);
}
