#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "VoidLookAndFeel.h"
#include "PresetManager.h"
#include "MidiManager.h"

class VoidAudioProcessorEditor : public juce::AudioProcessorEditor,
                                 public juce::Timer,
                                 public juce::ComboBox::Listener
{
public:
    explicit VoidAudioProcessorEditor(VoidAudioProcessor&);
    ~VoidAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

private:
    void loadBackgroundImage();
    void syncPresetUIFromProcessorState();
    void showMidiMenu(const juce::String& paramId, const juce::MouseEvent& e);
    void openMidiMappingModal();

    VoidAudioProcessor& audioProcessor;
    VoidLookAndFeel voidLookAndFeel;

    juce::Image backgroundImage;

    // Header Controls
    juce::TextButton prevPresetButton{"<"};
    juce::TextButton nextPresetButton{">"};
    juce::ComboBox categoryBox{"Category"};
    juce::ComboBox presetBox{"Preset"};
    juce::TextButton savePresetButton{"SAVE"};
    juce::TextButton deletePresetButton{"DEL"};
    juce::TextButton midiMapButton{"MIDI MAP"};
    juce::Label bpmLabel;

    std::vector<std::string> categories;

    // Column 0: Dual Pitch Engine & LFO
    VoidSegmentSwitch pitchVoicingSwitch{{"-12", "-24", "BOTH"}};
    VoidSegmentSwitch lfoTargetSwitch{{"OFF", "-12", "-24", "BOTH"}};
    VoidToggleButton lfoPhaseInvertToggle{"180° PHASE"};
    VoidToggleButton lfoSyncToggle{"BPM SYNC"};
    VoidKnob lfoRateKnob{1.0};
    juce::ComboBox lfoDivisionBox{"Division"};

    juce::Label pitchVoicingLabel;
    juce::Label lfoTargetLabel;
    juce::Label lfoRateLabel;

    // Column 1: "Carnage" Dynamic Distortion & "The Crypt" Reverb
    VoidKnob carnageDriveKnob{0.35};
    VoidToggleButton radioModeToggle{"RADIO / MEGAPHONE"};
    VoidSegmentSwitch reverbModeSwitch{{"BEDROOM", "DUNGEON", "CAVE", "ABYSS"}};
    VoidKnob reverbMixKnob{0.25};

    juce::Label carnageDriveLabel;
    juce::Label reverbModeLabel;
    juce::Label reverbMixLabel;

    // Column 2: Dynamics & Master I/O
    VoidKnob gateThreshKnob{-50.0};
    VoidKnob compThreshKnob{-18.0};
    VoidSegmentSwitch compRatioSwitch{{"2:1", "4:1", "8:1", "12:1"}};
    VoidKnob inputGainKnob{0.0};
    VoidKnob outputGainKnob{0.0};
    VoidSegmentSwitch inputRoutingSwitch{{"CH 1", "CH 2", "1+2"}};

    juce::Label gateThreshLabel;
    juce::Label compThreshLabel;
    juce::Label compRatioLabel;
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Label inputRoutingLabel;

    // Bottom Row: 5 Stomp Footswitches
    VoidFootswitchComponent pitchFootswitch{"PITCH"};
    VoidFootswitchComponent carnageFootswitch{"CARNAGE"};
    VoidFootswitchComponent compFootswitch{"COMP"};
    VoidFootswitchComponent widthFootswitch{"WIDTH"};
    VoidFootswitchComponent masterFootswitch{"MASTER"};

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> outputGainAttachment;
    std::unique_ptr<SliderAttachment> gateThreshAttachment;
    std::unique_ptr<SliderAttachment> compThreshAttachment;
    std::unique_ptr<SliderAttachment> lfoRateAttachment;
    std::unique_ptr<ComboBoxAttachment> lfoDivisionAttachment;
    std::unique_ptr<SliderAttachment> carnageDriveAttachment;
    std::unique_ptr<SliderAttachment> reverbMixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoidAudioProcessorEditor)
};
