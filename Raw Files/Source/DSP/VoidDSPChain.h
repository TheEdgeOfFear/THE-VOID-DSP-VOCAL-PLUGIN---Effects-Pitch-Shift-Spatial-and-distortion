#pragma once

#include "NoiseGate.h"
#include "CompressorEngine.h"
#include "PitchEngine.h"
#include "PitchLfoEngine.h"
#include "DistortionEngine.h"
#include "HaasWidthEngine.h"
#include "ReverbEngine.h"
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>

namespace VoidDSP
{

class VoidDSPChain
{
public:
    VoidDSPChain() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels)
    {
        currentSampleRate = sampleRate;
        channels = numChannels;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

        // Pre-conditioning filters (Highpass at 25 Hz)
        dcBlocker.prepare(spec);
        dcBlocker.reset();
        *dcBlocker.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 25.0f);

        // Module 1: Noise Gate per channel
        noiseGates.resize(static_cast<size_t>(numChannels));
        for (auto& g : noiseGates)
            g.prepare(sampleRate);

        // Module 2: Compressor per channel
        compressors.resize(static_cast<size_t>(numChannels));
        for (auto& c : compressors)
            c.prepare(sampleRate);

        // Module 3: Pitch engines per channel + LFO engine
        pitchEngines.resize(static_cast<size_t>(numChannels));
        for (auto& p : pitchEngines)
            p.prepare(sampleRate);

        pitchLfo.prepare(sampleRate);

        // Module 4: Distortion
        distortion.prepare(sampleRate, samplesPerBlock, numChannels);

        // Module 5: Haas Width
        haasWidth.prepare(sampleRate);

        // Module 6: Reverb
        reverb.prepare(sampleRate, samplesPerBlock, numChannels);

        scratchBuffer.setSize(numChannels, samplesPerBlock);
    }

    void reset()
    {
        dcBlocker.reset();
        for (auto& g : noiseGates) g.reset();
        for (auto& c : compressors) c.reset();
        for (auto& p : pitchEngines) p.reset();
        pitchLfo.reset();
        distortion.reset();
        haasWidth.reset();
        reverb.reset();
    }

    // inputRouting: 0 = CH 1 (Mono Left -> Stereo), 1 = CH 2 (Mono Right -> Stereo), 2 = CH 1+2 (Stereo / Sum)
    void process(juce::AudioBuffer<float>& buffer,
                 int inputRouting,
                 float inputGainLinear,
                 float gateThresholdDb,
                 int compRatioIndex,
                 float compThresholdDb,
                 bool compActive,
                 int pitchVoicingMode,
                 int lfoTargetMode,
                 bool lfoPhaseInvert,
                 bool lfoBpmSync,
                 float lfoRateHz,
                 LfoDivision lfoDivision,
                 bool pitchActive,
                 float distortionDrive,
                 bool distortionRadioMode,
                 bool distortionActive,
                 bool widthActive,
                 int reverbModeIndex,
                 float reverbMix,
                 float outputGainLinear,
                 bool masterActive,
                 double hostBpm = 120.0,
                 double hostPpq = -1.0,
                 bool isHostPlaying = false)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChans = buffer.getNumChannels();

        if (numSamples == 0 || numChans == 0)
            return;

        // -------------------------------------------------------------
        // Input Routing & Mono-to-Stereo Split
        // -------------------------------------------------------------
        if (numChans >= 2)
        {
            if (inputRouting == 0) // CH 1 (Mono -> Both L and R)
            {
                buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
            }
            else if (inputRouting == 1) // CH 2 (Mono -> Both L and R)
            {
                buffer.copyFrom(0, 0, buffer, 1, 0, numSamples);
            }
            // If inputRouting == 2 (1+2), leave Left and Right as-is for stereo tracking
        }

        // If Master is bypassed, clean pass-through with I/O gains
        if (!masterActive)
        {
            if (std::abs(inputGainLinear - 1.0f) > 0.001f)
                buffer.applyGain(inputGainLinear);
            if (std::abs(outputGainLinear - 1.0f) > 0.001f)
                buffer.applyGain(outputGainLinear);
            return;
        }

        // Apply Input Gain Trim
        buffer.applyGain(inputGainLinear);

        // DC Blocker filter
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        dcBlocker.process(context);

        // 1. Noise Gate & 2. Stepped Compressor & 3. Pitch Engine (Sample-by-sample loop)
        for (int i = 0; i < numSamples; ++i)
        {
            float ratioA = 0.5f;  // -12st
            float ratioB = 0.25f; // -24st

            pitchLfo.processSample(
                lfoRateHz,
                lfoBpmSync,
                lfoDivision,
                lfoTargetMode,
                lfoPhaseInvert,
                hostBpm,
                hostPpq,
                isHostPlaying,
                ratioA,
                ratioB
            );

            for (int ch = 0; ch < std::min(numChans, channels); ++ch)
            {
                float* chData = buffer.getWritePointer(ch);
                float sample = chData[i];

                // Stage 1: Noise Gate
                sample = noiseGates[static_cast<size_t>(ch)].processSample(sample, gateThresholdDb, true);

                // Stage 2: Stepped Compressor
                sample = compressors[static_cast<size_t>(ch)].processSample(sample, compRatioIndex, compThresholdDb, compActive);

                // Stage 3: Dual Pitch Shifter
                if (pitchActive)
                {
                    sample = pitchEngines[static_cast<size_t>(ch)].processSample(sample, pitchVoicingMode, ratioA, ratioB, true);
                }

                chData[i] = sample;
            }
        }

        // If mono input into stereo buffer, duplicate left channel to right
        if (buffer.getNumChannels() >= 2 && numChans == 1)
        {
            buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
        }

        // Stage 4: "Carnage" Dynamic Distortion (with Radio/Megaphone mode)
        distortion.process(buffer, distortionDrive, distortionRadioMode, distortionActive);

        // Stage 5: Haas Stereo Width
        haasWidth.process(buffer, widthActive);

        // Stage 6: "The Crypt" Algorithmic Reverb
        reverb.process(buffer, reverbModeIndex, reverbMix, true);

        // Master Output Gain Trim
        buffer.applyGain(outputGainLinear);
    }

private:
    double currentSampleRate = 44100.0;
    int channels = 2;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> dcBlocker;
    std::vector<NoiseGate> noiseGates;
    std::vector<CompressorEngine> compressors;
    std::vector<ChannelPitchEngine> pitchEngines;
    PitchLfoEngine pitchLfo;
    DistortionEngine distortion;
    HaasWidthEngine haasWidth;
    ReverbEngine reverb;

    juce::AudioBuffer<float> scratchBuffer;
};

} // namespace VoidDSP
