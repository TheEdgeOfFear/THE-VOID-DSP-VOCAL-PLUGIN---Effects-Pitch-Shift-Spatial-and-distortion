#pragma once

#include <cmath>
#include <algorithm>
#include <juce_core/juce_core.h>

namespace VoidDSP
{

class NoiseGate
{
public:
    NoiseGate() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        holdSamplesMax = static_cast<int>(sampleRate * 0.010); // 10 ms hold
        
        // Attack: <= 0.5 ms
        const double attackTimeSec = 0.0005;
        attackCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * attackTimeSec)));

        // Release: 25 ms
        const double releaseTimeSec = 0.025;
        releaseCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * releaseTimeSec)));

        reset();
    }

    void reset()
    {
        currentGain = 1.0f;
        envelope = 0.0f;
        holdCounter = 0;
    }

    inline float processSample(float in, float thresholdDb, bool gateEnabled = true)
    {
        if (!gateEnabled)
            return in;

        const float absIn = std::abs(in);
        // Fast peak/RMS follower
        if (absIn > envelope)
            envelope += 0.1f * (absIn - envelope);
        else
            envelope += 0.001f * (absIn - envelope);

        const float thresholdLinear = std::pow(10.0f, thresholdDb / 20.0f);

        if (envelope >= thresholdLinear)
        {
            holdCounter = holdSamplesMax;
            // Ramp up gain quickly
            currentGain += attackCoeff * (1.0f - currentGain);
        }
        else
        {
            if (holdCounter > 0)
            {
                --holdCounter;
                // Maintain unity gain during hold
            }
            else
            {
                // Exponential decay towards 0
                currentGain += releaseCoeff * (0.0f - currentGain);
            }
        }

        currentGain = std::clamp(currentGain, 0.0f, 1.0f);
        return in * currentGain;
    }

private:
    double currentSampleRate = 44100.0;
    int holdSamplesMax = 441;
    int holdCounter = 0;
    float attackCoeff = 0.1f;
    float releaseCoeff = 0.01f;
    float currentGain = 1.0f;
    float envelope = 0.0f;
};

} // namespace VoidDSP
