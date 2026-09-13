#pragma once

#include <cmath>
#include <algorithm>
#include <juce_core/juce_core.h>

namespace VoidDSP
{

class CompressorEngine
{
public:
    CompressorEngine() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;

        // Attack = 8 ms
        const double attackTimeSec = 0.008;
        attackCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * attackTimeSec)));

        // Release = 120 ms
        const double releaseTimeSec = 0.120;
        releaseCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * releaseTimeSec)));

        reset();
    }

    void reset()
    {
        smoothedGainReductionDb = 0.0f;
    }

    // Ratio index: 0 -> 2:1, 1 -> 4:1, 2 -> 8:1, 3 -> 12:1
    static float getRatioFromIndex(int index)
    {
        switch (index)
        {
            case 0: return 2.0f;
            case 1: return 4.0f;
            case 2: return 8.0f;
            case 3: return 12.0f;
            default: return 4.0f;
        }
    }

    inline float processSample(float in, int ratioIndex, float thresholdDb = -18.0f, bool enabled = true)
    {
        if (!enabled)
            return in;

        const float ratio = getRatioFromIndex(ratioIndex);
        const float kneeDb = 4.0f;
        const float halfKnee = kneeDb * 0.5f;

        const float absIn = std::abs(in);
        const float inDb = (absIn > 1e-6f) ? 20.0f * std::log10(absIn) : -120.0f;

        // Soft-knee feedforward static characteristic
        float targetGrDb = 0.0f;
        const float delta = inDb - thresholdDb;

        if (delta <= -halfKnee)
        {
            targetGrDb = 0.0f;
        }
        else if (delta > -halfKnee && delta < halfKnee)
        {
            const float slope = (1.0f / ratio) - 1.0f;
            const float excess = delta + halfKnee;
            targetGrDb = (slope * excess * excess) / (2.0f * kneeDb);
        }
        else
        {
            const float slope = (1.0f / ratio) - 1.0f;
            targetGrDb = slope * delta;
        }

        // Ballistics smoothing in dB domain (negative targetGrDb)
        if (targetGrDb < smoothedGainReductionDb)
        {
            // Attack (increasing compression)
            smoothedGainReductionDb += attackCoeff * (targetGrDb - smoothedGainReductionDb);
        }
        else
        {
            // Release (decreasing compression)
            smoothedGainReductionDb += releaseCoeff * (targetGrDb - smoothedGainReductionDb);
        }

        // Automatic makeup gain compensation: G_comp ≈ -Threshold * (1 - 1/Ratio) * 0.5 dB
        const float autoMakeupDb = -thresholdDb * (1.0f - (1.0f / ratio)) * 0.5f;
        const float totalGainDb = smoothedGainReductionDb + autoMakeupDb;
        const float totalGainLinear = std::pow(10.0f, totalGainDb / 20.0f);

        return in * totalGainLinear;
    }

private:
    double currentSampleRate = 44100.0;
    float attackCoeff = 0.01f;
    float releaseCoeff = 0.001f;
    float smoothedGainReductionDb = 0.0f;
};

} // namespace VoidDSP
