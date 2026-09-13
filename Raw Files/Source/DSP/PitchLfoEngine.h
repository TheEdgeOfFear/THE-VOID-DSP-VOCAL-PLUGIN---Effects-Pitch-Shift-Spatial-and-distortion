#pragma once

#include <cmath>
#include <juce_core/juce_core.h>

namespace VoidDSP
{

enum class LfoDivision
{
    Div_1_1 = 0,    // 4 beats
    Div_1_2,        // 2 beats
    Div_1_2D,       // 3 beats
    Div_1_4,        // 1 beat
    Div_1_4D,       // 1.5 beats
    Div_1_4T,       // 2/3 beat
    Div_1_8,        // 0.5 beat
    Div_1_8D,       // 0.75 beat
    Div_1_8T,       // 1/3 beat
    Div_1_16,       // 0.25 beat
    Div_1_16T       // 1/6 beat
};

class PitchLfoEngine
{
public:
    PitchLfoEngine() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        phase = 0.0;
    }

    void reset()
    {
        phase = 0.0;
    }

    // Calculates LFO frequency in Hz given BPM and sync division
    static double getSyncedFrequencyHz(double bpm, LfoDivision div)
    {
        const double safeBpm = std::clamp(bpm, 20.0, 400.0);
        const double bps = safeBpm / 60.0; // Beats per second

        double beatsPerCycle = 1.0;
        switch (div)
        {
            case LfoDivision::Div_1_1:   beatsPerCycle = 4.0; break;
            case LfoDivision::Div_1_2D:  beatsPerCycle = 3.0; break;
            case LfoDivision::Div_1_2:   beatsPerCycle = 2.0; break;
            case LfoDivision::Div_1_4D:  beatsPerCycle = 1.5; break;
            case LfoDivision::Div_1_4:   beatsPerCycle = 1.0; break;
            case LfoDivision::Div_1_4T:  beatsPerCycle = 2.0 / 3.0; break;
            case LfoDivision::Div_1_8D:  beatsPerCycle = 0.75; break;
            case LfoDivision::Div_1_8:   beatsPerCycle = 0.5; break;
            case LfoDivision::Div_1_8T:  beatsPerCycle = 1.0 / 3.0; break;
            case LfoDivision::Div_1_16:  beatsPerCycle = 0.25; break;
            case LfoDivision::Div_1_16T: beatsPerCycle = 1.0 / 6.0; break;
            default: beatsPerCycle = 1.0; break;
        }

        return bps / beatsPerCycle;
    }

    // Advance LFO by 1 sample and compute current pitch ratios for Layer A & Layer B
    // targetMode: 0 = Off, 1 = -12, 2 = -24, 3 = Both
    inline void processSample(float freeRateHz,
                              bool bpmSync,
                              LfoDivision division,
                              int targetMode,
                              bool phaseInvert180,
                              double hostBpm,
                              double hostPpq,
                              bool isHostPlaying,
                              float& outRatioA,
                              float& outRatioB)
    {
        double freqHz = freeRateHz;
        if (bpmSync)
        {
            freqHz = getSyncedFrequencyHz(hostBpm, division);

            if (isHostPlaying && hostPpq >= 0.0)
            {
                // Lock phase to host song position
                double beatsPerCycle = 1.0;
                switch (division)
                {
                    case LfoDivision::Div_1_1:   beatsPerCycle = 4.0; break;
                    case LfoDivision::Div_1_2D:  beatsPerCycle = 3.0; break;
                    case LfoDivision::Div_1_2:   beatsPerCycle = 2.0; break;
                    case LfoDivision::Div_1_4D:  beatsPerCycle = 1.5; break;
                    case LfoDivision::Div_1_4:   beatsPerCycle = 1.0; break;
                    case LfoDivision::Div_1_4T:  beatsPerCycle = 2.0 / 3.0; break;
                    case LfoDivision::Div_1_8D:  beatsPerCycle = 0.75; break;
                    case LfoDivision::Div_1_8:   beatsPerCycle = 0.5; break;
                    case LfoDivision::Div_1_8T:  beatsPerCycle = 1.0 / 3.0; break;
                    case LfoDivision::Div_1_16:  beatsPerCycle = 0.25; break;
                    case LfoDivision::Div_1_16T: beatsPerCycle = 1.0 / 6.0; break;
                    default: beatsPerCycle = 1.0; break;
                }
                const double cycleFraction = std::fmod(hostPpq, beatsPerCycle) / beatsPerCycle;
                phase = cycleFraction;
            }
        }

        if (!bpmSync || !isHostPlaying || hostPpq < 0.0)
        {
            const double phaseIncrement = freqHz / currentSampleRate;
            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;
        }

        // Base unmodulated semitones
        float semitonesA = -12.0f;
        float semitonesB = -24.0f;

        if (targetMode > 0)
        {
            // Smooth sinusoidal LFO in range [0.0, 1.0]
            const float lfoValA = 0.5f * (1.0f - std::cos(static_cast<float>(phase) * juce::MathConstants<float>::twoPi));
            float lfoValB = lfoValA;

            if (phaseInvert180 && targetMode == 3) // Both & Phase Invert enabled
            {
                lfoValB = 1.0f - lfoValA; // Anti-phase modulation for pitch shearing
            }

            // Target 1: -12 only
            if (targetMode == 1 || targetMode == 3)
            {
                // Sweeps Layer A from -12 st to 0 st
                semitonesA = -12.0f + (12.0f * lfoValA);
            }

            // Target 2: -24 only
            if (targetMode == 2 || targetMode == 3)
            {
                // Sweeps Layer B from -24 st to 0 st
                semitonesB = -24.0f + (24.0f * lfoValB);
            }
        }

        // Convert semitones to frequency ratio
        outRatioA = std::pow(2.0f, semitonesA / 12.0f);
        outRatioB = std::pow(2.0f, semitonesB / 12.0f);
    }

private:
    double currentSampleRate = 44100.0;
    double phase = 0.0;
};

} // namespace VoidDSP
