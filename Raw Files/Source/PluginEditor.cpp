#include "PluginEditor.h"
#include "MidiMappingModal.h"
#include <BinaryData.h>

VoidAudioProcessorEditor::VoidAudioProcessorEditor(VoidAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&voidLookAndFeel);
    loadBackgroundImage();

    // ==========================================
    // 1. TOP BAR PRESET BROWSER & UTILITIES
    // ==========================================
    prevPresetButton.onClick = [this]() {
        const int cur = audioProcessor.getCurrentProgram();
        const int total = audioProcessor.getNumPrograms();
        if (total > 0)
        {
            const int prev = (cur - 1 + total) % total;
            audioProcessor.setCurrentProgram(prev);
            syncPresetUIFromProcessorState();
        }
    };
    addAndMakeVisible(prevPresetButton);

    nextPresetButton.onClick = [this]() {
        const int cur = audioProcessor.getCurrentProgram();
        const int total = audioProcessor.getNumPrograms();
        if (total > 0)
        {
            const int next = (cur + 1) % total;
            audioProcessor.setCurrentProgram(next);
            syncPresetUIFromProcessorState();
        }
    };
    addAndMakeVisible(nextPresetButton);

    categories = audioProcessor.getPresetManager().getCategories();
    for (size_t i = 0; i < categories.size(); ++i)
        categoryBox.addItem(categories[i], static_cast<int>(i + 1));

    categoryBox.setSelectedId(1, juce::dontSendNotification);
    categoryBox.addListener(this);
    addAndMakeVisible(categoryBox);

    presetBox.addListener(this);
    addAndMakeVisible(presetBox);

    savePresetButton.onClick = [this]() {
        auto* aw = new juce::AlertWindow("SAVE USER PRESET", "Enter custom preset name for THE VOID:", juce::AlertWindow::NoIcon);
        aw->addTextEditor("name", "Custom Void Preset", "Preset Name:");
        aw->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int result) {
            if (result == 1)
            {
                const juce::String enteredName = aw->getTextEditorContents("name").trim();
                if (enteredName.isNotEmpty())
                {
                    VoidPresets::Preset userPreset;
                    userPreset.category = "Custom User";
                    userPreset.name = enteredName.toStdString();
                    userPreset.description = "User created custom preset for THE VOID.";
                    userPreset.inputRouting = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("inputRouting")->load());
                    userPreset.inputGain = audioProcessor.getAPVTS().getRawParameterValue("inputGain")->load();
                    userPreset.outputGain = audioProcessor.getAPVTS().getRawParameterValue("outputGain")->load();
                    userPreset.gateThreshold = audioProcessor.getAPVTS().getRawParameterValue("gateThreshold")->load();
                    userPreset.compRatio = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("compRatio")->load());
                    userPreset.compThreshold = audioProcessor.getAPVTS().getRawParameterValue("compThreshold")->load();
                    userPreset.compEnabled = audioProcessor.getAPVTS().getRawParameterValue("compEnabled")->load() > 0.5f;
                    userPreset.pitchVoicing = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("pitchVoicing")->load());
                    userPreset.lfoTarget = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("lfoTarget")->load());
                    userPreset.lfoPhaseInvert = audioProcessor.getAPVTS().getRawParameterValue("lfoPhaseInvert")->load() > 0.5f;
                    userPreset.lfoBpmSync = audioProcessor.getAPVTS().getRawParameterValue("lfoBpmSync")->load() > 0.5f;
                    userPreset.lfoRate = audioProcessor.getAPVTS().getRawParameterValue("lfoRate")->load();
                    userPreset.lfoDivision = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("lfoDivision")->load());
                    userPreset.pitchEnabled = audioProcessor.getAPVTS().getRawParameterValue("pitchEnabled")->load() > 0.5f;
                    userPreset.distortionDrive = audioProcessor.getAPVTS().getRawParameterValue("distortionDrive")->load();
                    userPreset.distortionRadioMode = audioProcessor.getAPVTS().getRawParameterValue("distortionRadioMode")->load() > 0.5f;
                    userPreset.distortionEnabled = audioProcessor.getAPVTS().getRawParameterValue("distortionEnabled")->load() > 0.5f;
                    userPreset.widthEnabled = audioProcessor.getAPVTS().getRawParameterValue("widthEnabled")->load() > 0.5f;
                    userPreset.reverbMode = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("reverbMode")->load());
                    userPreset.reverbMix = audioProcessor.getAPVTS().getRawParameterValue("reverbMix")->load();
                    userPreset.masterEnabled = audioProcessor.getAPVTS().getRawParameterValue("masterEnabled")->load() > 0.5f;

                    audioProcessor.getPresetManager().saveUserPreset(userPreset);
                    categories = audioProcessor.getPresetManager().getCategories();
                    categoryBox.clear();
                    for (size_t i = 0; i < categories.size(); ++i)
                        categoryBox.addItem(categories[i], static_cast<int>(i + 1));

                    for (size_t i = 0; i < categories.size(); ++i)
                    {
                        if (categories[i] == "Custom User")
                        {
                            categoryBox.setSelectedId(static_cast<int>(i + 1), juce::sendNotification);
                            break;
                        }
                    }
                }
            }
            delete aw;
        }));
    };
    addAndMakeVisible(savePresetButton);

    deletePresetButton.onClick = [this]() {
        const int cur = audioProcessor.getCurrentProgram();
        const auto& all = audioProcessor.getPresetManager().getAllPresets();
        if (cur >= 0 && cur < static_cast<int>(all.size()))
        {
            if (all[static_cast<size_t>(cur)].isFactory)
            {
                auto* aw = new juce::AlertWindow("PROTECTED PRESET", "Factory presets cannot be deleted.", juce::AlertWindow::WarningIcon);
                aw->addButton("OK", 1);
                aw->enterModalState(true, juce::ModalCallbackFunction::create([aw](int) { delete aw; }));
            }
            else
            {
                audioProcessor.getPresetManager().deleteUserPreset(cur);
                audioProcessor.setCurrentProgram(0);
                syncPresetUIFromProcessorState();
            }
        }
    };
    addAndMakeVisible(deletePresetButton);

    midiMapButton.onClick = [this]() { openMidiMappingModal(); };
    addAndMakeVisible(midiMapButton);

    bpmLabel.setText("BPM: 120.0", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centred);
    bpmLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    bpmLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff3344));
    addAndMakeVisible(bpmLabel);

    // ==========================================
    // 2. COLUMN 0: DUAL PITCH ENGINE & LFO
    // ==========================================
    pitchVoicingLabel.setText("OCTAVE SELECT", juce::dontSendNotification);
    pitchVoicingLabel.setJustificationType(juce::Justification::centred);
    pitchVoicingLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    pitchVoicingLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(pitchVoicingLabel);

    pitchVoicingSwitch.onSelectionChange = [this](int pos) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("pitchVoicing"))
            param->setValueNotifyingHost(static_cast<float>(pos) / 2.0f);
    };
    pitchVoicingSwitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("pitchVoicing", e); };
    addAndMakeVisible(pitchVoicingSwitch);

    lfoTargetLabel.setText("LFO TARGET", juce::dontSendNotification);
    lfoTargetLabel.setJustificationType(juce::Justification::centred);
    lfoTargetLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    lfoTargetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(lfoTargetLabel);

    lfoTargetSwitch.onSelectionChange = [this](int pos) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("lfoTarget"))
            param->setValueNotifyingHost(static_cast<float>(pos) / 3.0f);
    };
    lfoTargetSwitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("lfoTarget", e); };
    addAndMakeVisible(lfoTargetSwitch);

    lfoPhaseInvertToggle.onStateChange = [this](bool state) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("lfoPhaseInvert"))
            param->setValueNotifyingHost(state ? 1.0f : 0.0f);
    };
    lfoPhaseInvertToggle.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("lfoPhaseInvert", e); };
    addAndMakeVisible(lfoPhaseInvertToggle);

    lfoSyncToggle.onStateChange = [this](bool state) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("lfoBpmSync"))
            param->setValueNotifyingHost(state ? 1.0f : 0.0f);
        lfoRateKnob.setVisible(!state);
        lfoDivisionBox.setVisible(state);
    };
    lfoSyncToggle.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("lfoBpmSync", e); };
    addAndMakeVisible(lfoSyncToggle);

    lfoRateLabel.setText("LFO RATE / SPEED", juce::dontSendNotification);
    lfoRateLabel.setJustificationType(juce::Justification::centred);
    lfoRateLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    lfoRateLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(lfoRateLabel);

    lfoRateKnob.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("lfoRate", e); };
    addAndMakeVisible(lfoRateKnob);

    const juce::StringArray divs{"1/1", "1/2", "1/2 D", "1/4", "1/4 D", "1/4 T",
                                 "1/8", "1/8 D", "1/8 T", "1/16", "1/16 T"};
    for (int i = 0; i < divs.size(); ++i)
        lfoDivisionBox.addItem(divs[i], i + 1);
    addChildComponent(lfoDivisionBox);

    // ==========================================
    // 3. COLUMN 1: CARNAGE & THE CRYPT
    // ==========================================
    carnageDriveLabel.setText("CARNAGE DRIVE", juce::dontSendNotification);
    carnageDriveLabel.setJustificationType(juce::Justification::centred);
    carnageDriveLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    carnageDriveLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(carnageDriveLabel);

    carnageDriveKnob.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("distortionDrive", e); };
    addAndMakeVisible(carnageDriveKnob);

    radioModeToggle.onStateChange = [this](bool state) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("distortionRadioMode"))
            param->setValueNotifyingHost(state ? 1.0f : 0.0f);
    };
    radioModeToggle.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("distortionRadioMode", e); };
    addAndMakeVisible(radioModeToggle);

    reverbModeLabel.setText("REVERB SPACE", juce::dontSendNotification);
    reverbModeLabel.setJustificationType(juce::Justification::centred);
    reverbModeLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    reverbModeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(reverbModeLabel);

    reverbModeSwitch.onSelectionChange = [this](int pos) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("reverbMode"))
            param->setValueNotifyingHost(static_cast<float>(pos) / 3.0f);
    };
    reverbModeSwitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("reverbMode", e); };
    addAndMakeVisible(reverbModeSwitch);

    reverbMixLabel.setText("REVERB MIX", juce::dontSendNotification);
    reverbMixLabel.setJustificationType(juce::Justification::centred);
    reverbMixLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    reverbMixLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(reverbMixLabel);

    reverbMixKnob.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("reverbMix", e); };
    addAndMakeVisible(reverbMixKnob);

    // ==========================================
    // 4. COLUMN 2: DYNAMICS & MASTER I/O
    // ==========================================
    gateThreshKnob.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("gateThreshold", e); };
    addAndMakeVisible(gateThreshKnob);
    gateThreshLabel.setText("GATE THRESH", juce::dontSendNotification);
    gateThreshLabel.setJustificationType(juce::Justification::centred);
    gateThreshLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    gateThreshLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(gateThreshLabel);

    compThreshKnob.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("compThreshold", e); };
    addAndMakeVisible(compThreshKnob);
    compThreshLabel.setText("COMP THRESH", juce::dontSendNotification);
    compThreshLabel.setJustificationType(juce::Justification::centred);
    compThreshLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    compThreshLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(compThreshLabel);

    compRatioLabel.setText("COMP RATIO", juce::dontSendNotification);
    compRatioLabel.setJustificationType(juce::Justification::centred);
    compRatioLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    compRatioLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(compRatioLabel);

    compRatioSwitch.onSelectionChange = [this](int pos) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("compRatio"))
            param->setValueNotifyingHost(static_cast<float>(pos) / 3.0f);
    };
    compRatioSwitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("compRatio", e); };
    addAndMakeVisible(compRatioSwitch);

    inputGainKnob.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("inputGain", e); };
    addAndMakeVisible(inputGainKnob);
    inputGainLabel.setText("INPUT GAIN", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(inputGainLabel);

    outputGainKnob.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("outputGain", e); };
    addAndMakeVisible(outputGainKnob);
    outputGainLabel.setText("OUTPUT GAIN", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(outputGainLabel);

    inputRoutingLabel.setText("INPUT ROUTE", juce::dontSendNotification);
    inputRoutingLabel.setJustificationType(juce::Justification::centred);
    inputRoutingLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    inputRoutingLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc5cad4));
    addAndMakeVisible(inputRoutingLabel);

    inputRoutingSwitch.onSelectionChange = [this](int pos) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("inputRouting"))
            param->setValueNotifyingHost(static_cast<float>(pos) / 2.0f);
    };
    inputRoutingSwitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("inputRouting", e); };
    addAndMakeVisible(inputRoutingSwitch);

    // ==========================================
    // 5. BOTTOM ROW: 5 FOOTSWITCHES
    // ==========================================
    pitchFootswitch.onPointerState = [this](bool isDown) { audioProcessor.handlePitchPointerState(isDown); };
    pitchFootswitch.onClick = [this]() { audioProcessor.handlePitchClick(); };
    pitchFootswitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("pitchEnabled", e); };
    addAndMakeVisible(pitchFootswitch);

    carnageFootswitch.onPointerState = [this](bool isDown) { audioProcessor.handleDistortionPointerState(isDown); };
    carnageFootswitch.onClick = [this]() { audioProcessor.handleDistortionClick(); };
    carnageFootswitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("distortionEnabled", e); };
    addAndMakeVisible(carnageFootswitch);

    compFootswitch.onPointerState = [this](bool isDown) { audioProcessor.handleCompPointerState(isDown); };
    compFootswitch.onClick = [this]() { audioProcessor.handleCompClick(); };
    compFootswitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("compEnabled", e); };
    addAndMakeVisible(compFootswitch);

    widthFootswitch.onPointerState = [this](bool isDown) { audioProcessor.handleWidthPointerState(isDown); };
    widthFootswitch.onClick = [this]() { audioProcessor.handleWidthClick(); };
    widthFootswitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("widthEnabled", e); };
    addAndMakeVisible(widthFootswitch);

    masterFootswitch.onPointerState = [this](bool isDown) { audioProcessor.handleMasterPointerState(isDown); };
    masterFootswitch.onClick = [this]() { audioProcessor.handleMasterClick(); };
    masterFootswitch.onRightClick = [this](const juce::MouseEvent& e) { showMidiMenu("masterEnabled", e); };
    addAndMakeVisible(masterFootswitch);

    // ==========================================
    // 6. APVTS ATTACHMENTS
    // ==========================================
    auto& apvts = audioProcessor.getAPVTS();
    inputGainAttachment = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGainKnob);
    outputGainAttachment = std::make_unique<SliderAttachment>(apvts, "outputGain", outputGainKnob);
    gateThreshAttachment = std::make_unique<SliderAttachment>(apvts, "gateThreshold", gateThreshKnob);
    compThreshAttachment = std::make_unique<SliderAttachment>(apvts, "compThreshold", compThreshKnob);
    lfoRateAttachment = std::make_unique<SliderAttachment>(apvts, "lfoRate", lfoRateKnob);
    lfoDivisionAttachment = std::make_unique<ComboBoxAttachment>(apvts, "lfoDivision", lfoDivisionBox);
    carnageDriveAttachment = std::make_unique<SliderAttachment>(apvts, "distortionDrive", carnageDriveKnob);
    reverbMixAttachment = std::make_unique<SliderAttachment>(apvts, "reverbMix", reverbMixKnob);

    // Initialize custom segment switches
    const int voicingInit = static_cast<int>(apvts.getRawParameterValue("pitchVoicing")->load());
    pitchVoicingSwitch.setSelectedIndex(voicingInit, juce::dontSendNotification);

    const int targetInit = static_cast<int>(apvts.getRawParameterValue("lfoTarget")->load());
    lfoTargetSwitch.setSelectedIndex(targetInit, juce::dontSendNotification);

    const int ratioInit = static_cast<int>(apvts.getRawParameterValue("compRatio")->load());
    compRatioSwitch.setSelectedIndex(ratioInit, juce::dontSendNotification);

    const int revModeInit = static_cast<int>(apvts.getRawParameterValue("reverbMode")->load());
    reverbModeSwitch.setSelectedIndex(revModeInit, juce::dontSendNotification);

    const int routeInit = static_cast<int>(apvts.getRawParameterValue("inputRouting")->load());
    inputRoutingSwitch.setSelectedIndex(routeInit, juce::dontSendNotification);

    const bool radioInit = apvts.getRawParameterValue("distortionRadioMode")->load() > 0.5f;
    radioModeToggle.setToggleState(radioInit, juce::dontSendNotification);

    const bool phaseInit = apvts.getRawParameterValue("lfoPhaseInvert")->load() > 0.5f;
    lfoPhaseInvertToggle.setToggleState(phaseInit, juce::dontSendNotification);

    const bool syncInit = apvts.getRawParameterValue("lfoBpmSync")->load() > 0.5f;
    lfoSyncToggle.setToggleState(syncInit, juce::dontSendNotification);
    lfoRateKnob.setVisible(!syncInit);
    lfoDivisionBox.setVisible(syncInit);

    syncPresetUIFromProcessorState();

    // 16:9 Aspect ratio: 1152 x 648
    setSize(1152, 648);
    setResizeLimits(800, 450, 1920, 1080);

    startTimerHz(30);
}

VoidAudioProcessorEditor::~VoidAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void VoidAudioProcessorEditor::loadBackgroundImage()
{
    backgroundImage = juce::ImageFileFormat::loadFrom(BinaryData::THEVOIDBG_jpg,
                                                      static_cast<size_t>(BinaryData::THEVOIDBG_jpgSize));

    if (!backgroundImage.isValid())
    {
        juce::File localFile("C:\\Coding\\Tunings VST3\\THE VOID\\BACKGROUND IMAGE\\THEVOIDBG.jpg");
        if (localFile.existsAsFile())
            backgroundImage = juce::ImageFileFormat::loadFrom(localFile);
    }
}

void VoidAudioProcessorEditor::syncPresetUIFromProcessorState()
{
    const int cur = audioProcessor.getCurrentProgram();
    const auto& presets = audioProcessor.getPresetManager().getAllPresets();
    if (cur >= 0 && cur < static_cast<int>(presets.size()))
    {
        const auto& p = presets[static_cast<size_t>(cur)];
        for (size_t i = 0; i < categories.size(); ++i)
        {
            if (categories[i] == p.category)
            {
                categoryBox.setSelectedId(static_cast<int>(i + 1), juce::dontSendNotification);
                break;
            }
        }

        presetBox.clear(juce::dontSendNotification);
        auto inCat = audioProcessor.getPresetManager().getPresetsInCategory(p.category);
        int selectedItem = 1;
        for (size_t i = 0; i < inCat.size(); ++i)
        {
            presetBox.addItem(inCat[i].name, static_cast<int>(i + 1));
            if (inCat[i].name == p.name)
                selectedItem = static_cast<int>(i + 1);
        }
        presetBox.setSelectedId(selectedItem, juce::dontSendNotification);
    }
}

void VoidAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &categoryBox)
    {
        const int catIdx = categoryBox.getSelectedId() - 1;
        if (catIdx >= 0 && catIdx < static_cast<int>(categories.size()))
        {
            const auto catName = categories[static_cast<size_t>(catIdx)];
            auto inCat = audioProcessor.getPresetManager().getPresetsInCategory(catName);
            presetBox.clear(juce::dontSendNotification);
            for (size_t i = 0; i < inCat.size(); ++i)
                presetBox.addItem(inCat[i].name, static_cast<int>(i + 1));

            presetBox.setSelectedId(1, juce::dontSendNotification);
            const int globalIdx = audioProcessor.getPresetManager().getGlobalIndex(catName, 0);
            audioProcessor.setCurrentProgram(globalIdx);
        }
    }
    else if (comboBoxThatHasChanged == &presetBox)
    {
        const int catIdx = categoryBox.getSelectedId() - 1;
        const int pIdx = presetBox.getSelectedId() - 1;
        if (catIdx >= 0 && catIdx < static_cast<int>(categories.size()) && pIdx >= 0)
        {
            const auto catName = categories[static_cast<size_t>(catIdx)];
            const int globalIdx = audioProcessor.getPresetManager().getGlobalIndex(catName, pIdx);
            audioProcessor.setCurrentProgram(globalIdx);
        }
    }
}

void VoidAudioProcessorEditor::showMidiMenu(const juce::String& paramId, const juce::MouseEvent& e)
{
    juce::PopupMenu menu;
    menu.addSectionHeader("MIDI LEARN / CC");

    if (audioProcessor.getMidiManager().isLearning() && audioProcessor.getMidiManager().getLearningParamId() == paramId.toStdString())
    {
        menu.addItem(1, "Cancel Learning...", true, false);
    }
    else
    {
        menu.addItem(1, "Learn MIDI CC (Move hardware controller)...");
    }

    menu.addItem(2, "Clear Mapped MIDI CC");
    menu.addSeparator();
    menu.addItem(3, "Open Full MIDI Mapping Manager...");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(e.eventComponent), [this, paramId](int result) {
        if (result == 1)
        {
            if (audioProcessor.getMidiManager().isLearning() && audioProcessor.getMidiManager().getLearningParamId() == paramId.toStdString())
                audioProcessor.getMidiManager().stopLearning();
            else
                audioProcessor.getMidiManager().startLearning(paramId.toStdString());
        }
        else if (result == 2)
        {
            audioProcessor.getMidiManager().clearMapping(paramId.toStdString());
        }
        else if (result == 3)
        {
            openMidiMappingModal();
        }
    });
}

void VoidAudioProcessorEditor::openMidiMappingModal()
{
    auto* modal = new MidiMappingModal(audioProcessor.getMidiManager());
    juce::DialogWindow::LaunchOptions opt;
    opt.dialogTitle = "THE VOID - MIDI CC MAPPING MANAGER";
    opt.content.setOwned(modal);
    opt.resizable = false;
    opt.useNativeTitleBar = false;
    opt.launchAsync();
}

void VoidAudioProcessorEditor::timerCallback()
{
    // Update LEDs from atomic processor states
    pitchFootswitch.setLedActive(audioProcessor.isPitchActive());
    carnageFootswitch.setLedActive(audioProcessor.isDistortionActive());
    compFootswitch.setLedActive(audioProcessor.isCompActive());
    widthFootswitch.setLedActive(audioProcessor.isWidthActive());
    masterFootswitch.setLedActive(audioProcessor.isMasterActive());

    // Update BPM display
    const double bpm = audioProcessor.getHostBpm();
    bpmLabel.setText("BPM: " + juce::String(bpm, 1), juce::dontSendNotification);

    // Sync custom switches from processor APVTS in case of automation
    auto& apvts = audioProcessor.getAPVTS();
    const int voicingVal = static_cast<int>(apvts.getRawParameterValue("pitchVoicing")->load());
    if (pitchVoicingSwitch.getSelectedIndex() != voicingVal)
        pitchVoicingSwitch.setSelectedIndex(voicingVal, juce::dontSendNotification);

    const int targetVal = static_cast<int>(apvts.getRawParameterValue("lfoTarget")->load());
    if (lfoTargetSwitch.getSelectedIndex() != targetVal)
        lfoTargetSwitch.setSelectedIndex(targetVal, juce::dontSendNotification);

    const int ratioVal = static_cast<int>(apvts.getRawParameterValue("compRatio")->load());
    if (compRatioSwitch.getSelectedIndex() != ratioVal)
        compRatioSwitch.setSelectedIndex(ratioVal, juce::dontSendNotification);

    const int revModeVal = static_cast<int>(apvts.getRawParameterValue("reverbMode")->load());
    if (reverbModeSwitch.getSelectedIndex() != revModeVal)
        reverbModeSwitch.setSelectedIndex(revModeVal, juce::dontSendNotification);

    const int routeVal = static_cast<int>(apvts.getRawParameterValue("inputRouting")->load());
    if (inputRoutingSwitch.getSelectedIndex() != routeVal)
        inputRoutingSwitch.setSelectedIndex(routeVal, juce::dontSendNotification);

    const bool radioVal = apvts.getRawParameterValue("distortionRadioMode")->load() > 0.5f;
    if (radioModeToggle.getToggleState() != radioVal)
        radioModeToggle.setToggleState(radioVal, juce::dontSendNotification);

    const bool phaseVal = apvts.getRawParameterValue("lfoPhaseInvert")->load() > 0.5f;
    if (lfoPhaseInvertToggle.getToggleState() != phaseVal)
        lfoPhaseInvertToggle.setToggleState(phaseVal, juce::dontSendNotification);

    const bool syncVal = apvts.getRawParameterValue("lfoBpmSync")->load() > 0.5f;
    if (lfoSyncToggle.getToggleState() != syncVal)
    {
        lfoSyncToggle.setToggleState(syncVal, juce::dontSendNotification);
        lfoRateKnob.setVisible(!syncVal);
        lfoDivisionBox.setVisible(syncVal);
    }
}

void VoidAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Full Pedal Background Artwork
    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage, bounds, juce::RectanglePlacement::stretchToFit);
    }
    else
    {
        juce::ColourGradient bgGrad(juce::Colour(0xff161920), 0, 0,
                                    juce::Colour(0xff090b0e), 0, bounds.getBottom(), false);
        g.setGradientFill(bgGrad);
        g.fillAll();
    }

    // Outer Distressed Metal Bezel
    g.setColour(juce::Colour(0x66000000));
    g.drawRect(bounds, 3.0f);

    // 2. Translucent Frosted Module Background Plates
    const float marginX = bounds.getWidth() * 0.04f;
    const float panelY = bounds.getHeight() * 0.12f;
    const float panelH = bounds.getHeight() * 0.58f;
    const float totalW = bounds.getWidth() - (marginX * 2.0f);
    const float gap = bounds.getWidth() * 0.02f;
    const float colW = (totalW - (gap * 2.0f)) / 3.0f;

    // Draw 3 Section Module Panels
    for (int col = 0; col < 3; ++col)
    {
        const float colX = marginX + (static_cast<float>(col) * (colW + gap));
        juce::Rectangle<float> colRect(colX, panelY, colW, panelH);

        // Dark frosted plate
        g.setColour(juce::Colour(0xd5080a0f));
        g.fillRoundedRectangle(colRect, 8.0f);

        // Weathered crimson/steel border
        g.setColour(juce::Colour(0x883f1218));
        g.drawRoundedRectangle(colRect, 8.0f, 1.5f);

        // Inner subtle bevel
        g.setColour(juce::Colour(0x22ffffff));
        g.drawRoundedRectangle(colRect.reduced(1.5f), 7.0f, 1.0f);

        // Corner rivets
        const float r = 3.0f;
        g.setColour(juce::Colour(0xff454c5c));
        g.fillEllipse(colRect.getX() + 6.0f, colRect.getY() + 6.0f, r * 2.0f, r * 2.0f);
        g.fillEllipse(colRect.getRight() - 6.0f - (r * 2.0f), colRect.getY() + 6.0f, r * 2.0f, r * 2.0f);
        g.fillEllipse(colRect.getX() + 6.0f, colRect.getBottom() - 6.0f - (r * 2.0f), r * 2.0f, r * 2.0f);
        g.fillEllipse(colRect.getRight() - 6.0f - (r * 2.0f), colRect.getBottom() - 6.0f - (r * 2.0f), r * 2.0f, r * 2.0f);

        // Header Plate Ribbon
        juce::Rectangle<float> headerRibbon(colX + 12.0f, panelY + 8.0f, colW - 24.0f, 22.0f);
        g.setColour(juce::Colour(0xee160608));
        g.fillRoundedRectangle(headerRibbon, 4.0f);
        g.setColour(juce::Colour(0xff5a151e));
        g.drawRoundedRectangle(headerRibbon, 4.0f, 1.0f);

        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xffff3344));
        if (col == 0)
            g.drawText("PITCH ENGINE & LFO", headerRibbon, juce::Justification::centred);
        else if (col == 1)
            g.drawText("CARNAGE & THE CRYPT", headerRibbon, juce::Justification::centred);
        else
            g.drawText("DYNAMICS & CONTROL", headerRibbon, juce::Justification::centred);
    }

    // Top Bar Plate
    juce::Rectangle<float> topBarRect(marginX, bounds.getHeight() * 0.02f, totalW, bounds.getHeight() * 0.08f);
    g.setColour(juce::Colour(0xdd090b10));
    g.fillRoundedRectangle(topBarRect, 6.0f);
    g.setColour(juce::Colour(0xff4a141b));
    g.drawRoundedRectangle(topBarRect, 6.0f, 1.2f);

    // Title Branding
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE VOID", juce::Rectangle<float>(topBarRect.getX() + 14.0f, topBarRect.getY(), 110.0f, topBarRect.getHeight()), juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xffff3344));
    g.drawText("VOCAL PROCESSOR", juce::Rectangle<float>(topBarRect.getX() + 126.0f, topBarRect.getY() + 3.0f, 130.0f, topBarRect.getHeight()), juce::Justification::centredLeft);
}

void VoidAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().toFloat();
    const float marginX = bounds.getWidth() * 0.04f;
    const float totalW = bounds.getWidth() - (marginX * 2.0f);
    const float gap = bounds.getWidth() * 0.02f;
    const float colW = (totalW - (gap * 2.0f)) / 3.0f;

    // ==========================================
    // 1. TOP BAR LAYOUT
    // ==========================================
    const float topY = bounds.getHeight() * 0.025f;
    const float topH = bounds.getHeight() * 0.068f;

    const float preX = marginX + 270.0f;
    prevPresetButton.setBounds(static_cast<int>(preX), static_cast<int>(topY + 4.0f), 26, static_cast<int>(topH - 8.0f));
    nextPresetButton.setBounds(static_cast<int>(preX + 28.0f), static_cast<int>(topY + 4.0f), 26, static_cast<int>(topH - 8.0f));

    categoryBox.setBounds(static_cast<int>(preX + 60.0f), static_cast<int>(topY + 4.0f), 170, static_cast<int>(topH - 8.0f));
    presetBox.setBounds(static_cast<int>(preX + 236.0f), static_cast<int>(topY + 4.0f), 190, static_cast<int>(topH - 8.0f));

    savePresetButton.setBounds(static_cast<int>(preX + 432.0f), static_cast<int>(topY + 4.0f), 48, static_cast<int>(topH - 8.0f));
    deletePresetButton.setBounds(static_cast<int>(preX + 484.0f), static_cast<int>(topY + 4.0f), 44, static_cast<int>(topH - 8.0f));
    midiMapButton.setBounds(static_cast<int>(preX + 534.0f), static_cast<int>(topY + 4.0f), 78, static_cast<int>(topH - 8.0f));

    bpmLabel.setBounds(static_cast<int>(marginX + totalW - 90.0f), static_cast<int>(topY + 4.0f), 86, static_cast<int>(topH - 8.0f));

    // ==========================================
    // 2. MODULE PANELS LAYOUT
    // ==========================================
    const float panelY = bounds.getHeight() * 0.12f;

    // ------------------------------------------
    // Column 0: PITCH ENGINE & LFO
    // ------------------------------------------
    const float col0X = marginX;
    const float switchPad = 24.0f;

    // 1. Octave Select
    pitchVoicingLabel.setBounds(static_cast<int>(col0X), static_cast<int>(panelY + 36.0f), static_cast<int>(colW), 16);
    pitchVoicingSwitch.setBounds(static_cast<int>(col0X + switchPad), static_cast<int>(panelY + 54.0f), static_cast<int>(colW - (switchPad * 2.0f)), 26);

    // 2. LFO Target
    lfoTargetLabel.setBounds(static_cast<int>(col0X), static_cast<int>(panelY + 88.0f), static_cast<int>(colW), 16);
    lfoTargetSwitch.setBounds(static_cast<int>(col0X + switchPad), static_cast<int>(panelY + 106.0f), static_cast<int>(colW - (switchPad * 2.0f)), 26);

    // 3. Toggles Row (180° Phase & BPM Sync)
    const float toggleGap = 12.0f;
    const float toggleW = (colW - (switchPad * 2.0f) - toggleGap) * 0.5f;
    lfoPhaseInvertToggle.setBounds(static_cast<int>(col0X + switchPad), static_cast<int>(panelY + 144.0f), static_cast<int>(toggleW), 32);
    lfoSyncToggle.setBounds(static_cast<int>(col0X + switchPad + toggleW + toggleGap), static_cast<int>(panelY + 144.0f), static_cast<int>(toggleW), 32);

    // 4. LFO Speed / Division
    lfoRateLabel.setBounds(static_cast<int>(col0X), static_cast<int>(panelY + 188.0f), static_cast<int>(colW), 16);
    const float knobSize0 = 84.0f;
    lfoRateKnob.setBounds(static_cast<int>(col0X + (colW * 0.5f) - (knobSize0 * 0.5f)), static_cast<int>(panelY + 208.0f), static_cast<int>(knobSize0), static_cast<int>(knobSize0 + 20.0f));
    lfoDivisionBox.setBounds(static_cast<int>(col0X + (colW * 0.5f) - 65.0f), static_cast<int>(panelY + 236.0f), 130, 28);

    // ------------------------------------------
    // Column 1: CARNAGE & THE CRYPT
    // ------------------------------------------
    const float col1X = marginX + colW + gap;

    // 1. Carnage Drive
    carnageDriveLabel.setBounds(static_cast<int>(col1X), static_cast<int>(panelY + 34.0f), static_cast<int>(colW), 16);
    const float knobSize1 = 80.0f;
    carnageDriveKnob.setBounds(static_cast<int>(col1X + (colW * 0.5f) - (knobSize1 * 0.5f)), static_cast<int>(panelY + 50.0f), static_cast<int>(knobSize1), static_cast<int>(knobSize1 + 20.0f));

    // 2. Radio / Megaphone Toggle
    const float radioW = colW - (switchPad * 2.0f);
    radioModeToggle.setBounds(static_cast<int>(col1X + switchPad), static_cast<int>(panelY + 154.0f), static_cast<int>(radioW), 30);

    // 3. Reverb Space
    reverbModeLabel.setBounds(static_cast<int>(col1X), static_cast<int>(panelY + 190.0f), static_cast<int>(colW), 16);
    reverbModeSwitch.setBounds(static_cast<int>(col1X + 16.0f), static_cast<int>(panelY + 208.0f), static_cast<int>(colW - 32.0f), 26);

    // 4. Reverb Mix
    reverbMixLabel.setBounds(static_cast<int>(col1X), static_cast<int>(panelY + 240.0f), static_cast<int>(colW), 16);
    const float revKnobSize = 74.0f;
    reverbMixKnob.setBounds(static_cast<int>(col1X + (colW * 0.5f) - (revKnobSize * 0.5f)), static_cast<int>(panelY + 258.0f), static_cast<int>(revKnobSize), static_cast<int>(revKnobSize + 20.0f));

    // ------------------------------------------
    // Column 2: DYNAMICS & CONTROL (Balanced 2x2 Grid + Routing)
    // ------------------------------------------
    const float col2X = marginX + (colW + gap) * 2.0f;
    const float dynKnobSize = 68.0f;
    const float dynKnobH = dynKnobSize + 18.0f;
    const float col2Pad = 32.0f;
    const float rightKnobX = col2X + colW - dynKnobSize - col2Pad;
    const float leftKnobX = col2X + col2Pad;

    // Top: Gate Thresh (Left) & Comp Thresh (Right)
    const float topDynKnobY = panelY + 34.0f;
    gateThreshKnob.setBounds(static_cast<int>(leftKnobX), static_cast<int>(topDynKnobY), static_cast<int>(dynKnobSize), static_cast<int>(dynKnobH));
    gateThreshLabel.setBounds(static_cast<int>(leftKnobX - 10.0f), static_cast<int>(topDynKnobY + dynKnobH + 2.0f), static_cast<int>(dynKnobSize + 20.0f), 16);

    compThreshKnob.setBounds(static_cast<int>(rightKnobX), static_cast<int>(topDynKnobY), static_cast<int>(dynKnobSize), static_cast<int>(dynKnobH));
    compThreshLabel.setBounds(static_cast<int>(rightKnobX - 10.0f), static_cast<int>(topDynKnobY + dynKnobH + 2.0f), static_cast<int>(dynKnobSize + 20.0f), 16);

    // Middle: Comp Ratio (Full width 4-way switch)
    const float ratioY = panelY + 140.0f;
    compRatioLabel.setBounds(static_cast<int>(col2X), static_cast<int>(ratioY), static_cast<int>(colW), 16);
    compRatioSwitch.setBounds(static_cast<int>(col2X + 20.0f), static_cast<int>(ratioY + 18.0f), static_cast<int>(colW - 40.0f), 26);

    // Bottom: Input Gain (Left) & Output Gain (Right)
    const float btmDynKnobY = panelY + 190.0f;
    inputGainKnob.setBounds(static_cast<int>(leftKnobX), static_cast<int>(btmDynKnobY), static_cast<int>(dynKnobSize), static_cast<int>(dynKnobH));
    inputGainLabel.setBounds(static_cast<int>(leftKnobX - 10.0f), static_cast<int>(btmDynKnobY + dynKnobH + 2.0f), static_cast<int>(dynKnobSize + 20.0f), 16);

    outputGainKnob.setBounds(static_cast<int>(rightKnobX), static_cast<int>(btmDynKnobY), static_cast<int>(dynKnobSize), static_cast<int>(dynKnobH));
    outputGainLabel.setBounds(static_cast<int>(rightKnobX - 10.0f), static_cast<int>(btmDynKnobY + dynKnobH + 2.0f), static_cast<int>(dynKnobSize + 20.0f), 16);

    // Input Routing Switch (Bottom full width)
    const float routeY = panelY + 298.0f;
    inputRoutingLabel.setBounds(static_cast<int>(col2X), static_cast<int>(routeY), static_cast<int>(colW), 16);
    inputRoutingSwitch.setBounds(static_cast<int>(col2X + 36.0f), static_cast<int>(routeY + 18.0f), static_cast<int>(colW - 72.0f), 26);

    // ==========================================
    // 3. BOTTOM ROW: 5 FOOTSWITCHES
    // ==========================================
    const float stompY = bounds.getHeight() * 0.72f;
    const float stompH = bounds.getHeight() * 0.25f;
    const float stompSpacing = totalW / 5.0f;
    const float stompW = 120.0f;

    for (int i = 0; i < 5; ++i)
    {
        const float sX = marginX + (static_cast<float>(i) * stompSpacing) + ((stompSpacing - stompW) * 0.5f);
        juce::Component* comp = nullptr;
        if (i == 0) comp = &pitchFootswitch;
        else if (i == 1) comp = &carnageFootswitch;
        else if (i == 2) comp = &compFootswitch;
        else if (i == 3) comp = &widthFootswitch;
        else if (i == 4) comp = &masterFootswitch;

        if (comp != nullptr)
            comp->setBounds(static_cast<int>(sX), static_cast<int>(stompY), static_cast<int>(stompW), static_cast<int>(stompH));
    }
}
