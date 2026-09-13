#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP/VoidDSPChain.h"
#include "PresetManager.h"
#include "MidiManager.h"
#include <atomic>

class VoidAudioProcessor : public juce::AudioProcessor
{
public:
    VoidAudioProcessor();
    ~VoidAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }
    VoidPresets::PresetManager& getPresetManager() { return presetManager; }
    MidiManager& getMidiManager() { return midiManager; }

    // Footswitches Handlers (Smart Momentary Hold & Tap Toggle)
    void handlePitchPointerState(bool isDown);
    void handlePitchClick();

    void handleDistortionPointerState(bool isDown);
    void handleDistortionClick();

    void handleCompPointerState(bool isDown);
    void handleCompClick();

    void handleWidthPointerState(bool isDown);
    void handleWidthClick();

    void handleMasterPointerState(bool isDown);
    void handleMasterClick();

    bool isPitchActive() const { return pitchActive.load(); }
    bool isDistortionActive() const { return distortionActive.load(); }
    bool isCompActive() const { return compActive.load(); }
    bool isWidthActive() const { return widthActive.load(); }
    bool isMasterActive() const { return masterActive.load(); }

    double getHostBpm() const { return currentHostBpm.load(); }

    void applyPreset(int globalIndex);

private:
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    VoidDSP::VoidDSPChain dspChain;
    VoidPresets::PresetManager presetManager;
    MidiManager midiManager;

    std::atomic<int> currentPresetIndex{0};
    std::atomic<double> currentHostBpm{120.0};
    std::atomic<bool> hostIsPlaying{false};

    // Stomp active state machines
    std::atomic<bool> pitchActive{true};
    std::atomic<bool> distortionActive{true};
    std::atomic<bool> compActive{true};
    std::atomic<bool> widthActive{true};
    std::atomic<bool> masterActive{true};

    juce::int64 pitchPressTimestamp{0};
    juce::int64 distortionPressTimestamp{0};
    juce::int64 compPressTimestamp{0};
    juce::int64 widthPressTimestamp{0};
    juce::int64 masterPressTimestamp{0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoidAudioProcessor)
};
