#pragma once

#include <cmath>
#include <vector>
#include <algorithm>
#include <juce_core/juce_core.h>

namespace VoidDSP
{

// High-fidelity dual-head circular delay pitch shifter with cubic interpolation
class VoicePitchShifter
{
public:
    VoicePitchShifter() = default;

    void prepare(double sampleRate, float windowMs = 45.0f)
    {
        currentSampleRate = sampleRate;
        windowSize = std::max(64, static_cast<int>(sampleRate * (windowMs / 1000.0f)));
        bufferSize = windowSize * 4;

        delayBuffer.assign(bufferSize, 0.0f);
        writeIndex = 0;

        phase1 = 0.0f;
        phase2 = 0.5f; // 180 degrees out of phase for constant-power Hann crossfade
    }

    void reset()
    {
        std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
        writeIndex = 0;
        phase1 = 0.0f;
        phase2 = 0.5f;
    }

    // Process single sample with dynamic pitch shift ratio
    inline float processSample(float in, float pitchRatio)
    {
        delayBuffer[writeIndex] = in;

        // Rate of delay tap motion: (1.0 - pitchRatio)
        const float rate = (1.0f - pitchRatio) / static_cast<float>(windowSize);

        phase1 += rate;
        if (phase1 >= 1.0f) phase1 -= 1.0f;
        else if (phase1 < 0.0f) phase1 += 1.0f;

        phase2 += rate;
        if (phase2 >= 1.0f) phase2 -= 1.0f;
        else if (phase2 < 0.0f) phase2 += 1.0f;

        // Calculate tap delay in samples
        const float delaySamples1 = phase1 * static_cast<float>(windowSize);
        const float delaySamples2 = phase2 * static_cast<float>(windowSize);

        // Read cubic interpolated taps
        const float tap1 = readCubicInterpolated(static_cast<float>(writeIndex) - delaySamples1);
        const float tap2 = readCubicInterpolated(static_cast<float>(writeIndex) - delaySamples2);

        // Hann window weights for seamless crossfade
        const float w1 = 0.5f * (1.0f - std::cos(phase1 * juce::MathConstants<float>::twoPi));
        const float w2 = 0.5f * (1.0f - std::cos(phase2 * juce::MathConstants<float>::twoPi));

        if (++writeIndex >= bufferSize)
            writeIndex = 0;

        return (tap1 * w1) + (tap2 * w2);
    }

private:
    inline float readCubicInterpolated(float readPos) const
    {
        while (readPos < 0.0f) readPos += static_cast<float>(bufferSize);
        while (readPos >= static_cast<float>(bufferSize)) readPos -= static_cast<float>(bufferSize);

        const int i1 = static_cast<int>(readPos);
        const int i0 = (i1 - 1 < 0) ? (bufferSize - 1) : (i1 - 1);
        const int i2 = (i1 + 1 >= bufferSize) ? 0 : (i1 + 1);
        const int i3 = (i1 + 2 >= bufferSize) ? ((i1 + 2) - bufferSize) : (i1 + 2);

        const float frac = readPos - static_cast<float>(i1);

        const float p0 = delayBuffer[i0];
        const float p1 = delayBuffer[i1];
        const float p2 = delayBuffer[i2];
        const float p3 = delayBuffer[i3];

        const float a = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
        const float b = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
        const float c = -0.5f * p0 + 0.5f * p2;
        const float d = p1;

        return ((a * frac + b) * frac + c) * frac + d;
    }

    double currentSampleRate = 44100.0;
    std::vector<float> delayBuffer;
    int bufferSize = 8192;
    int windowSize = 1984;
    int writeIndex = 0;
    float phase1 = 0.0f;
    float phase2 = 0.5f;
};

// Dual-voice pitch engine per channel (Layer A = -12st, Layer B = -24st)
class ChannelPitchEngine
{
public:
    ChannelPitchEngine() = default;

    void prepare(double sampleRate)
    {
        // Layer A (-12st)
        layerA.prepare(sampleRate, 48.0f);
        // Layer B (-24st, slightly longer window for deep sub-octave fundamental)
        layerB.prepare(sampleRate, 60.0f);
    }

    void reset()
    {
        layerA.reset();
        layerB.reset();
    }

    // Voicing mode: 0 -> -12 only, 1 -> -24 only, 2 -> -12 + -24 dual tracking
    inline float processSample(float in, int voicingMode, float ratioA, float ratioB, bool enabled = true)
    {
        if (!enabled)
            return in;

        float out = 0.0f;

        if (voicingMode == 0) // -12 only
        {
            out = layerA.processSample(in, ratioA);
        }
        else if (voicingMode == 1) // -24 only
        {
            out = layerB.processSample(in, ratioB);
        }
        else // -12 + -24 dual tracking
        {
            const float vA = layerA.processSample(in, ratioA);
            const float vB = layerB.processSample(in, ratioB);
            out = (vA * 0.55f) + (vB * 0.55f);
        }

        return out;
    }

private:
    VoicePitchShifter layerA;
    VoicePitchShifter layerB;
};

} // namespace VoidDSP
