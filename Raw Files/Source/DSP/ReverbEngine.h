#pragma once

#include <cmath>
#include <vector>
#include <algorithm>
#include <juce_dsp/juce_dsp.h>

namespace VoidDSP
{

enum class CryptReverbMode
{
    AlexInTheBedroom = 0, // 0.4s tight reflective box
    DungeonRoom,          // 1.2s cold stone room
    OminousCave,          // 3.5s dark cavern, heavy HF roll-off above 4.5kHz
    TheAbyss              // 8.0s massive void with shimmer modulation
};

class ReverbEngine
{
public:
    ReverbEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels)
    {
        currentSampleRate = sampleRate;
        channels = numChannels;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

        reverb.prepare(spec);
        reverb.reset();

        caveDampingFilter.prepare(spec);
        caveDampingFilter.reset();
        *caveDampingFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 4500.0f, 0.707f);

        dungeonMidFilter.prepare(spec);
        dungeonMidFilter.reset();
        *dungeonMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 420.0f, 1.2f, 1.35f);

        wetBuffer.setSize(numChannels, samplesPerBlock);

        shimmerPhase = 0.0;
        modDelayBuffer.assign(static_cast<size_t>(sampleRate * 0.05), 0.0f);
        modWriteIndex = 0;

        currentMode = -1;
        setMode(0);
    }

    void reset()
    {
        reverb.reset();
        caveDampingFilter.reset();
        dungeonMidFilter.reset();
        std::fill(modDelayBuffer.begin(), modDelayBuffer.end(), 0.0f);
        modWriteIndex = 0;
        shimmerPhase = 0.0;
    }

    void setMode(int modeIndex)
    {
        if (modeIndex == currentMode)
            return;

        currentMode = modeIndex;
        juce::dsp::Reverb::Parameters params;

        switch (modeIndex)
        {
            case 0: // Alex in the Bedroom: ~0.4s, high early reflections, heavy damping
                params.roomSize   = 0.22f;
                params.damping    = 0.75f;
                params.wetLevel   = 1.0f;
                params.dryLevel   = 0.0f;
                params.width      = 0.85f;
                params.freezeMode = 0.0f;
                break;

            case 1: // Dungeon Room: ~1.2s cold stone room
                params.roomSize   = 0.58f;
                params.damping    = 0.40f;
                params.wetLevel   = 1.0f;
                params.dryLevel   = 0.0f;
                params.width      = 0.95f;
                params.freezeMode = 0.0f;
                break;

            case 2: // Ominous Cave: ~3.5s dark cavern
                params.roomSize   = 0.86f;
                params.damping    = 0.82f;
                params.wetLevel   = 1.0f;
                params.dryLevel   = 0.0f;
                params.width      = 1.0f;
                params.freezeMode = 0.0f;
                break;

            case 3: // The Abyss: ~8.0s endless modulated void
            default:
                params.roomSize   = 0.98f;
                params.damping    = 0.18f;
                params.wetLevel   = 1.0f;
                params.dryLevel   = 0.0f;
                params.width      = 1.0f;
                params.freezeMode = 0.0f;
                break;
        }

        reverb.setParameters(params);
    }

    void process(juce::AudioBuffer<float>& buffer, int modeIndex, float mix, bool enabled = true)
    {
        if (!enabled || mix <= 0.0001f)
            return;

        const int numSamples = buffer.getNumSamples();
        const int numChans = std::min(buffer.getNumChannels(), channels);

        if (numSamples == 0 || numChans == 0)
            return;

        setMode(modeIndex);

        // Copy input to wetBuffer for reverb processing
        wetBuffer.setSize(numChans, numSamples, false, false, true);
        for (int ch = 0; ch < numChans; ++ch)
            wetBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

        juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);

        // Pre/Post-filtering based on acoustic room character
        if (modeIndex == 1) // Dungeon Room low-mid boost
        {
            dungeonMidFilter.process(wetContext);
        }

        reverb.process(wetContext);

        if (modeIndex == 2) // Ominous Cave HF roll-off above 4.5 kHz
        {
            caveDampingFilter.process(wetContext);
        }
        else if (modeIndex == 3) // The Abyss shimmer/chorus modulation
        {
            applyAbyssModulation(wetBuffer);
        }

        // Mix blending: Dry/Wet
        const float wetGain = mix;
        const float dryGain = 1.0f - (mix * 0.35f); // Maintain solid direct punch

        for (int ch = 0; ch < numChans; ++ch)
        {
            float* dest = buffer.getWritePointer(ch);
            const float* wet = wetBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                dest[i] = (dest[i] * dryGain) + (wet[i] * wetGain);
            }
        }
    }

private:
    void applyAbyssModulation(juce::AudioBuffer<float>& buf)
    {
        const int numSamples = buf.getNumSamples();
        const int numChans = buf.getNumChannels();
        const int bufLen = static_cast<int>(modDelayBuffer.size());
        if (bufLen == 0) return;

        const double lfoRateHz = 0.45;
        const double phaseInc = lfoRateHz / currentSampleRate;

        for (int i = 0; i < numSamples; ++i)
        {
            shimmerPhase += phaseInc;
            if (shimmerPhase >= 1.0) shimmerPhase -= 1.0;

            const float lfo = std::sin(static_cast<float>(shimmerPhase * juce::MathConstants<double>::twoPi));
            const float modDelaySamples = 200.0f + (60.0f * lfo);

            for (int ch = 0; ch < numChans; ++ch)
            {
                float* data = buf.getWritePointer(ch);
                const float in = data[i];

                if (ch == 1) // Apply subtle pitch-chorus shimmer to right channel
                {
                    modDelayBuffer[modWriteIndex] = in;
                    float readPos = static_cast<float>(modWriteIndex) - modDelaySamples;
                    while (readPos < 0.0f) readPos += static_cast<float>(bufLen);
                    while (readPos >= static_cast<float>(bufLen)) readPos -= static_cast<float>(bufLen);

                    const int i0 = static_cast<int>(readPos);
                    const int i1 = (i0 + 1 >= bufLen) ? 0 : i0 + 1;
                    const float frac = readPos - static_cast<float>(i0);
                    const float delayed = modDelayBuffer[i0] + frac * (modDelayBuffer[i1] - modDelayBuffer[i0]);

                    data[i] = (in * 0.7f) + (delayed * 0.4f);
                }
            }

            if (++modWriteIndex >= bufLen)
                modWriteIndex = 0;
        }
    }

    double currentSampleRate = 44100.0;
    int channels = 2;
    int currentMode = -1;

    juce::dsp::Reverb reverb;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> caveDampingFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> dungeonMidFilter;

    juce::AudioBuffer<float> wetBuffer;
    std::vector<float> modDelayBuffer;
    int modWriteIndex = 0;
    double shimmerPhase = 0.0;
};

} // namespace VoidDSP
