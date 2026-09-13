#include "PluginProcessor.h"
#include "PluginEditor.h"

VoidAudioProcessor::VoidAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    pitchActive.store(parameters.getRawParameterValue("pitchEnabled")->load() > 0.5f);
    distortionActive.store(parameters.getRawParameterValue("distortionEnabled")->load() > 0.5f);
    compActive.store(parameters.getRawParameterValue("compEnabled")->load() > 0.5f);
    widthActive.store(parameters.getRawParameterValue("widthEnabled")->load() > 0.5f);
    masterActive.store(parameters.getRawParameterValue("masterEnabled")->load() > 0.5f);
}

VoidAudioProcessor::~VoidAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VoidAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Master I/O Gain & Input Routing
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"inputRouting", 1}, "Input Routing",
        juce::StringArray{"CH 1", "CH 2", "1+2"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"inputGain", 1}, "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1}, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Module 1: Noise Gate
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gateThreshold", 1}, "Gate Threshold",
        juce::NormalisableRange<float>(-80.0f, 0.0f, 0.5f), -50.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Module 2: Stepped Compressor
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"compRatio", 1}, "Comp Ratio",
        juce::StringArray{"2:1", "4:1", "8:1", "12:1"}, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"compThreshold", 1}, "Comp Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.5f), -18.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"compEnabled", 1}, "Comp Foot-switch", true));

    // Module 3: Dual Pitch Shifter & LFO
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"pitchVoicing", 1}, "Pitch Voicing",
        juce::StringArray{"-12", "-24", "Both"}, 2));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"lfoTarget", 1}, "LFO Target",
        juce::StringArray{"Off", "-12", "-24", "Both"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"lfoPhaseInvert", 1}, "LFO 180 Phase Invert", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"lfoBpmSync", 1}, "LFO BPM Sync", false));

    juce::NormalisableRange<float> lfoRateRange(0.05f, 15.0f, 0.01f);
    lfoRateRange.setSkewForCentre(1.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lfoRate", 1}, "LFO Rate",
        lfoRateRange, 1.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"lfoDivision", 1}, "LFO Division",
        juce::StringArray{"1/1", "1/2", "1/2 Dotted", "1/4", "1/4 Dotted", "1/4 Triplet",
                          "1/8", "1/8 Dotted", "1/8 Triplet", "1/16", "1/16 Triplet"}, 3));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"pitchEnabled", 1}, "Pitch Foot-switch", true));

    // Module 4: "Carnage" Dynamic Distortion
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distortionDrive", 1}, "Carnage Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"distortionRadioMode", 1}, "Megaphone Radio Mode", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"distortionEnabled", 1}, "Carnage Foot-switch", true));

    // Module 5: Haas Stereo Width
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"widthEnabled", 1}, "Haas Width Foot-switch", true));

    // Module 6: "The Crypt" Algorithmic Reverb
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"reverbMode", 1}, "Reverb Space",
        juce::StringArray{"Alex in Bedroom", "Dungeon Room", "Ominous Cave", "The Abyss"}, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbMix", 1}, "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f));

    // Master Footswitch
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"masterEnabled", 1}, "Master Foot-switch", true));

    return { params.begin(), params.end() };
}

const juce::String VoidAudioProcessor::getName() const
{
    return "THE VOID";
}

bool VoidAudioProcessor::acceptsMidi() const { return true; }
bool VoidAudioProcessor::producesMidi() const { return false; }
bool VoidAudioProcessor::isMidiEffect() const { return false; }
double VoidAudioProcessor::getTailLengthSeconds() const { return 4.0; }

int VoidAudioProcessor::getNumPrograms()
{
    return static_cast<int>(presetManager.getAllPresets().size());
}

int VoidAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex.load();
}

void VoidAudioProcessor::setCurrentProgram(int index)
{
    const auto& presets = presetManager.getAllPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
    {
        currentPresetIndex.store(index);
        applyPreset(index);
    }
}

const juce::String VoidAudioProcessor::getProgramName(int index)
{
    const auto& presets = presetManager.getAllPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void VoidAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void VoidAudioProcessor::applyPreset(int globalIndex)
{
    const auto& presets = presetManager.getAllPresets();
    if (globalIndex < 0 || globalIndex >= static_cast<int>(presets.size()))
        return;

    const auto& p = presets[static_cast<size_t>(globalIndex)];

    if (auto* param = parameters.getParameter("inputRouting"))
        param->setValueNotifyingHost(static_cast<float>(p.inputRouting) / 2.0f);

    if (auto* param = parameters.getParameter("inputGain"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(p.inputGain));

    if (auto* param = parameters.getParameter("outputGain"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(p.outputGain));

    if (auto* param = parameters.getParameter("gateThreshold"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(p.gateThreshold));

    if (auto* param = parameters.getParameter("compRatio"))
        param->setValueNotifyingHost(static_cast<float>(p.compRatio) / 3.0f);

    if (auto* param = parameters.getParameter("compThreshold"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(p.compThreshold));

    if (auto* param = parameters.getParameter("compEnabled"))
    {
        param->setValueNotifyingHost(p.compEnabled ? 1.0f : 0.0f);
        compActive.store(p.compEnabled);
    }

    if (auto* param = parameters.getParameter("pitchVoicing"))
        param->setValueNotifyingHost(static_cast<float>(p.pitchVoicing) / 2.0f);

    if (auto* param = parameters.getParameter("lfoTarget"))
        param->setValueNotifyingHost(static_cast<float>(p.lfoTarget) / 3.0f);

    if (auto* param = parameters.getParameter("lfoPhaseInvert"))
        param->setValueNotifyingHost(p.lfoPhaseInvert ? 1.0f : 0.0f);

    if (auto* param = parameters.getParameter("lfoBpmSync"))
        param->setValueNotifyingHost(p.lfoBpmSync ? 1.0f : 0.0f);

    if (auto* param = parameters.getParameter("lfoRate"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(p.lfoRate));

    if (auto* param = parameters.getParameter("lfoDivision"))
        param->setValueNotifyingHost(static_cast<float>(p.lfoDivision) / 10.0f);

    if (auto* param = parameters.getParameter("pitchEnabled"))
    {
        param->setValueNotifyingHost(p.pitchEnabled ? 1.0f : 0.0f);
        pitchActive.store(p.pitchEnabled);
    }

    if (auto* param = parameters.getParameter("distortionDrive"))
        param->setValueNotifyingHost(p.distortionDrive);

    if (auto* param = parameters.getParameter("distortionRadioMode"))
        param->setValueNotifyingHost(p.distortionRadioMode ? 1.0f : 0.0f);

    if (auto* param = parameters.getParameter("distortionEnabled"))
    {
        param->setValueNotifyingHost(p.distortionEnabled ? 1.0f : 0.0f);
        distortionActive.store(p.distortionEnabled);
    }

    if (auto* param = parameters.getParameter("widthEnabled"))
    {
        param->setValueNotifyingHost(p.widthEnabled ? 1.0f : 0.0f);
        widthActive.store(p.widthEnabled);
    }

    if (auto* param = parameters.getParameter("reverbMode"))
        param->setValueNotifyingHost(static_cast<float>(p.reverbMode) / 3.0f);

    if (auto* param = parameters.getParameter("reverbMix"))
        param->setValueNotifyingHost(p.reverbMix);

    if (auto* param = parameters.getParameter("masterEnabled"))
    {
        param->setValueNotifyingHost(p.masterEnabled ? 1.0f : 0.0f);
        masterActive.store(p.masterEnabled);
    }
}

// =========================================================================
// Footswitch Logic: Smart Momentary Hold (>280ms) & Tap Toggle (<280ms)
// =========================================================================
void VoidAudioProcessor::handlePitchPointerState(bool isDown)
{
    if (isDown)
    {
        pitchPressTimestamp = juce::Time::currentTimeMillis();
    }
    else
    {
        const juce::int64 duration = juce::Time::currentTimeMillis() - pitchPressTimestamp;
        if (duration >= 280)
        {
            // Momentary hold release -> turn off
            pitchActive.store(false);
            if (auto* p = parameters.getParameter("pitchEnabled"))
                p->setValueNotifyingHost(0.0f);
        }
    }
}

void VoidAudioProcessor::handlePitchClick()
{
    const bool current = pitchActive.load();
    const bool next = !current;
    pitchActive.store(next);
    if (auto* p = parameters.getParameter("pitchEnabled"))
        p->setValueNotifyingHost(next ? 1.0f : 0.0f);
}

void VoidAudioProcessor::handleDistortionPointerState(bool isDown)
{
    if (isDown)
    {
        distortionPressTimestamp = juce::Time::currentTimeMillis();
    }
    else
    {
        const juce::int64 duration = juce::Time::currentTimeMillis() - distortionPressTimestamp;
        if (duration >= 280)
        {
            distortionActive.store(false);
            if (auto* p = parameters.getParameter("distortionEnabled"))
                p->setValueNotifyingHost(0.0f);
        }
    }
}

void VoidAudioProcessor::handleDistortionClick()
{
    const bool current = distortionActive.load();
    const bool next = !current;
    distortionActive.store(next);
    if (auto* p = parameters.getParameter("distortionEnabled"))
        p->setValueNotifyingHost(next ? 1.0f : 0.0f);
}

void VoidAudioProcessor::handleCompPointerState(bool isDown)
{
    if (isDown)
    {
        compPressTimestamp = juce::Time::currentTimeMillis();
    }
    else
    {
        const juce::int64 duration = juce::Time::currentTimeMillis() - compPressTimestamp;
        if (duration >= 280)
        {
            compActive.store(false);
            if (auto* p = parameters.getParameter("compEnabled"))
                p->setValueNotifyingHost(0.0f);
        }
    }
}

void VoidAudioProcessor::handleCompClick()
{
    const bool current = compActive.load();
    const bool next = !current;
    compActive.store(next);
    if (auto* p = parameters.getParameter("compEnabled"))
        p->setValueNotifyingHost(next ? 1.0f : 0.0f);
}

void VoidAudioProcessor::handleWidthPointerState(bool isDown)
{
    if (isDown)
    {
        widthPressTimestamp = juce::Time::currentTimeMillis();
    }
    else
    {
        const juce::int64 duration = juce::Time::currentTimeMillis() - widthPressTimestamp;
        if (duration >= 280)
        {
            widthActive.store(false);
            if (auto* p = parameters.getParameter("widthEnabled"))
                p->setValueNotifyingHost(0.0f);
        }
    }
}

void VoidAudioProcessor::handleWidthClick()
{
    const bool current = widthActive.load();
    const bool next = !current;
    widthActive.store(next);
    if (auto* p = parameters.getParameter("widthEnabled"))
        p->setValueNotifyingHost(next ? 1.0f : 0.0f);
}

void VoidAudioProcessor::handleMasterPointerState(bool isDown)
{
    if (isDown)
    {
        masterPressTimestamp = juce::Time::currentTimeMillis();
    }
    else
    {
        const juce::int64 duration = juce::Time::currentTimeMillis() - masterPressTimestamp;
        if (duration >= 280)
        {
            masterActive.store(false);
            if (auto* p = parameters.getParameter("masterEnabled"))
                p->setValueNotifyingHost(0.0f);
        }
    }
}

void VoidAudioProcessor::handleMasterClick()
{
    const bool current = masterActive.load();
    const bool next = !current;
    masterActive.store(next);
    if (auto* p = parameters.getParameter("masterEnabled"))
        p->setValueNotifyingHost(next ? 1.0f : 0.0f);
}

void VoidAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dspChain.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void VoidAudioProcessor::releaseResources()
{
    dspChain.reset();
}

bool VoidAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    const auto& mainInput  = layouts.getMainInputChannelSet();

    if (mainOutput != juce::AudioChannelSet::mono() && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    if (mainInput != juce::AudioChannelSet::mono() && mainInput != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void VoidAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Process MIDI messages & CC mappings
    midiManager.processMidiBuffer(midiMessages, parameters);

    // Synchronize Footswitch toggles from APVTS (e.g. Host Automation or MIDI CC)
    pitchActive.store(parameters.getRawParameterValue("pitchEnabled")->load() > 0.5f);
    distortionActive.store(parameters.getRawParameterValue("distortionEnabled")->load() > 0.5f);
    compActive.store(parameters.getRawParameterValue("compEnabled")->load() > 0.5f);
    widthActive.store(parameters.getRawParameterValue("widthEnabled")->load() > 0.5f);
    masterActive.store(parameters.getRawParameterValue("masterEnabled")->load() > 0.5f);

    // Host Transport Info
    double hostBpm = 120.0;
    double hostPpq = -1.0;
    bool isPlaying = false;

    if (auto* ph = getPlayHead())
    {
        if (auto posOpt = ph->getPosition())
        {
            if (posOpt->getBpm().hasValue())
                hostBpm = *posOpt->getBpm();
            if (posOpt->getPpqPosition().hasValue())
                hostPpq = *posOpt->getPpqPosition();
            isPlaying = posOpt->getIsPlaying();
        }
    }


    currentHostBpm.store(hostBpm);
    hostIsPlaying.store(isPlaying);

    // Extract APVTS Parameters
    const int inRouting = static_cast<int>(parameters.getRawParameterValue("inputRouting")->load());

    const float inGainDb = parameters.getRawParameterValue("inputGain")->load();
    const float inGainLinear = std::pow(10.0f, inGainDb / 20.0f);

    const float outGainDb = parameters.getRawParameterValue("outputGain")->load();
    const float outGainLinear = std::pow(10.0f, outGainDb / 20.0f);

    const float gateThreshDb = parameters.getRawParameterValue("gateThreshold")->load();

    const int compRatioIdx = static_cast<int>(parameters.getRawParameterValue("compRatio")->load());
    const float compThreshDb = parameters.getRawParameterValue("compThreshold")->load();
    const bool compOn = compActive.load();

    const int pitchVoicing = static_cast<int>(parameters.getRawParameterValue("pitchVoicing")->load());
    const int lfoTarget = static_cast<int>(parameters.getRawParameterValue("lfoTarget")->load());
    const bool lfoPhaseInv = parameters.getRawParameterValue("lfoPhaseInvert")->load() > 0.5f;
    const bool lfoSync = parameters.getRawParameterValue("lfoBpmSync")->load() > 0.5f;
    const float lfoRateHz = parameters.getRawParameterValue("lfoRate")->load();
    const auto lfoDiv = static_cast<VoidDSP::LfoDivision>(static_cast<int>(parameters.getRawParameterValue("lfoDivision")->load()));
    const bool pitchOn = pitchActive.load();

    const float distDrive = parameters.getRawParameterValue("distortionDrive")->load();
    const bool distRadio = parameters.getRawParameterValue("distortionRadioMode")->load() > 0.5f;
    const bool distOn = distortionActive.load();

    const bool widthOn = widthActive.load();

    const int revMode = static_cast<int>(parameters.getRawParameterValue("reverbMode")->load());
    const float revMix = parameters.getRawParameterValue("reverbMix")->load();

    const bool masterOn = masterActive.load();

    dspChain.process(
        buffer,
        inRouting,
        inGainLinear,
        gateThreshDb,
        compRatioIdx,
        compThreshDb,
        compOn,
        pitchVoicing,
        lfoTarget,
        lfoPhaseInv,
        lfoSync,
        lfoRateHz,
        lfoDiv,
        pitchOn,
        distDrive,
        distRadio,
        distOn,
        widthOn,
        revMode,
        revMix,
        outGainLinear,
        masterOn,
        hostBpm,
        hostPpq,
        isPlaying
    );
}

juce::AudioProcessorEditor* VoidAudioProcessor::createEditor()
{
    return new VoidAudioProcessorEditor(*this);
}

void VoidAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VoidAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            pitchActive.store(parameters.getRawParameterValue("pitchEnabled")->load() > 0.5f);
            distortionActive.store(parameters.getRawParameterValue("distortionEnabled")->load() > 0.5f);
            compActive.store(parameters.getRawParameterValue("compEnabled")->load() > 0.5f);
            widthActive.store(parameters.getRawParameterValue("widthEnabled")->load() > 0.5f);
            masterActive.store(parameters.getRawParameterValue("masterEnabled")->load() > 0.5f);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoidAudioProcessor();
}
