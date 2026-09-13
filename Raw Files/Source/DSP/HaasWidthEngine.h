#pragma once

#include <cmath>
#include <vector>
#include <algorithm>
#include <juce_core/juce_core.h>

namespace VoidDSP
{

class HaasWidthEngine
{
public:
    HaasWidthEngine() = default;

    void prepare(double sampleRate, float defaultDelayMs = 16.5f)
    {
        currentSampleRate = sampleRate;
        delayMs = defaultDelayMs;

        const int maxDelaySamples = static_cast<int>(sampleRate * 0.050); // up to 50ms buffer
        delayBufferRight.assign(maxDelaySamples, 0.0f);
        writeIndex = 0;
        smoothedEngageGain = 0.0f;
    }

    void reset()
    {
        std::fill(delayBufferRight.begin(), delayBufferRight.end(), 0.0f);
        writeIndex = 0;
        smoothedEngageGain = 0.0f;
    }

    void setDelayMs(float ms)
    {
        delayMs = std::clamp(ms, 10.0f, 30.0f);
    }

    void process(juce::AudioBuffer<float>& buffer, bool enabled = true)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChans = buffer.getNumChannels();

        if (numSamples == 0 || numChans < 2)
            return;

        const float targetEngage = enabled ? 1.0f : 0.0f;
        const float delaySamples = static_cast<float>(currentSampleRate * (delayMs / 1000.0f));
        const int bufferSize = static_cast<int>(delayBufferRight.size());
        // -0.5 dB trim on lagging channel to balance psychoacoustic Haas localization
        const float laggingTrim = 0.94406f; // 10^(-0.5 / 20)

        float* right = buffer.getWritePointer(1);


        for (int i = 0; i < numSamples; ++i)
        {
            // Smooth engage transition to prevent clicks
            smoothedEngageGain += 0.005f * (targetEngage - smoothedEngageGain);

            const float rightIn = right[i];
            delayBufferRight[writeIndex] = rightIn;

            // Read delayed sample with linear interpolation
            float readPos = static_cast<float>(writeIndex) - delaySamples;
            while (readPos < 0.0f) readPos += static_cast<float>(bufferSize);
            while (readPos >= static_cast<float>(bufferSize)) readPos -= static_cast<float>(bufferSize);

            const int i0 = static_cast<int>(readPos);
            const int i1 = (i0 + 1 >= bufferSize) ? 0 : i0 + 1;
            const float frac = readPos - static_cast<float>(i0);
            const float delayedRight = delayBufferRight[i0] + frac * (delayBufferRight[i1] - delayBufferRight[i0]);

            if (++writeIndex >= bufferSize)
                writeIndex = 0;

            const float haasProcessedRight = delayedRight * laggingTrim;

            // Blend based on smooth engagement
            right[i] = (1.0f - smoothedEngageGain) * rightIn + (smoothedEngageGain * haasProcessedRight);
        }
    }

private:
    double currentSampleRate = 44100.0;
    float delayMs = 16.5f;
    std::vector<float> delayBufferRight;
    int writeIndex = 0;
    float smoothedEngageGain = 0.0f;
};

} // namespace VoidDSP
