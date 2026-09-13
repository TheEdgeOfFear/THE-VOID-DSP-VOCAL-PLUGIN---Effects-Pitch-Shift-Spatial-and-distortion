#pragma once

#include <cmath>
#include <vector>
#include <algorithm>
#include <juce_dsp/juce_dsp.h>

namespace VoidDSP
{

class DistortionEngine
{
public:
    DistortionEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels)
    {
        currentSampleRate = sampleRate;
        channels = numChannels;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

        // Standard Dynamic pre-filters
        lowCutFilter.prepare(spec);
        lowCutFilter.reset();

        presenceFilter.prepare(spec);
        presenceFilter.reset();

        // Radio / Megaphone Lo-Fi bandpass & horn resonance filters
        radioHpFilter.prepare(spec);
        radioHpFilter.reset();
        *radioHpFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 380.0f, 0.707f);

        radioLpFilter.prepare(spec);
        radioLpFilter.reset();
        *radioLpFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 3800.0f, 0.707f);

        radioPeakFilter.prepare(spec);
        radioPeakFilter.reset();
        const float radioGainLinear = std::pow(10.0f, 9.0f / 20.0f); // +9 dB horn resonance
        *radioPeakFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 2200.0f, 2.2f, radioGainLinear);

        // 4x Oversampling
        oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
            static_cast<size_t>(numChannels),
            2, // 2^2 = 4x
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true
        );
        oversampling->initProcessing(static_cast<size_t>(samplesPerBlock));
        oversampling->reset();
    }

    void reset()
    {
        lowCutFilter.reset();
        presenceFilter.reset();
        radioHpFilter.reset();
        radioLpFilter.reset();
        radioPeakFilter.reset();
        if (oversampling)
            oversampling->reset();
    }

    void process(juce::AudioBuffer<float>& buffer, float drive, bool radioMode, bool enabled = true)
    {
        if (!enabled || (drive <= 0.001f && !radioMode))
            return;

        const int numSamples = buffer.getNumSamples();
        const int numChans = std::min(buffer.getNumChannels(), channels);

        if (numSamples == 0 || numChans == 0)
            return;

        juce::dsp::AudioBlock<float> mainBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> context(mainBlock);

        if (radioMode)
        {
            // Radio / Megaphone Mode (Manson / Nathan James lo-fi bandpass & horn bite)
            radioHpFilter.process(context);
            radioLpFilter.process(context);
            radioPeakFilter.process(context);
        }
        else
        {
            // Standard Dynamic Curves based on Drive macro (0.0 to 1.0)
            const float hpFreq = 40.0f + (55.0f * drive);
            *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, hpFreq, 0.707f);

            const float presenceGainDb = 8.0f * drive;
            const float presenceGainLinear = std::pow(10.0f, presenceGainDb / 20.0f);
            *presenceFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate, 3200.0f, 1.4f, presenceGainLinear);

            lowCutFilter.process(context);
            presenceFilter.process(context);
        }

        // 2. Drive Pre-Gain: 0 dB to +36 dB
        const float effectiveDrive = radioMode ? std::max(0.25f, drive) : drive;
        const float preGainDb = (radioMode ? 8.0f : 0.0f) + (36.0f * effectiveDrive);
        const float preGainLinear = std::pow(10.0f, preGainDb / 20.0f);

        // Post-gain compensation
        const float postGainLinear = radioMode ? (1.0f / (1.5f + 3.2f * effectiveDrive))
                                               : (1.0f / (1.0f + 2.8f * effectiveDrive));

        // 3. 4x Oversampled Waveshaper
        juce::dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(mainBlock);
        const size_t osSamples = oversampledBlock.getNumSamples();

        for (int ch = 0; ch < numChans; ++ch)
        {
            float* chData = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));

            for (size_t i = 0; i < osSamples; ++i)
            {
                float x = chData[i] * preGainLinear;

                if (radioMode)
                {
                    // Transistor / Intercom hard-soft clipping curve with aggressive odd harmonics
                    float y = std::tanh(x * 1.35f);
                    // Add sharp clipping threshold for that distorted megaphone grit
                    if (y > 0.82f) y = 0.82f + 0.18f * std::tanh((y - 0.82f) * 4.0f);
                    else if (y < -0.82f) y = -0.82f + 0.18f * std::tanh((y + 0.82f) * 4.0f);

                    chData[i] = y * postGainLinear * 1.35f;
                }
                else
                {
                    // Asymmetrical tube/diode saturation
                    const float bias = 0.08f * effectiveDrive;
                    float biased = x + bias;
                    float y = std::tanh(biased);
                    y = y - 0.12f * (y * y * y);
                    y -= std::tanh(bias) * 0.95f;

                    chData[i] = y * postGainLinear;
                }
            }
        }

        oversampling->processSamplesDown(mainBlock);
    }

private:
    double currentSampleRate = 44100.0;
    int channels = 2;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowCutFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> presenceFilter;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> radioHpFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> radioLpFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> radioPeakFilter;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
};

} // namespace VoidDSP
