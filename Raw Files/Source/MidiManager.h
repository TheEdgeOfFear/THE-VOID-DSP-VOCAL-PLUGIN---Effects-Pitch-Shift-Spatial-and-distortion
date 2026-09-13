#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include <mutex>

struct MidiMapping
{
    std::string paramId;
    std::string paramName;
    int ccNumber = -1; // -1 = unmapped
    int channel = 1;   // 1-16 (0 = all / omni)
    bool isToggle = false;
};

class MidiManager
{
public:
    MidiManager()
    {
        initDefaultMappings();
        loadMappings();
    }

    void initDefaultMappings()
    {
        mappings.clear();
        mappings.push_back({"masterEnabled", "Master FX Power", -1, 1, true});
        mappings.push_back({"pitchEnabled", "Pitch Footswitch", -1, 1, true});
        mappings.push_back({"distortionEnabled", "Carnage Distortion Footswitch", -1, 1, true});
        mappings.push_back({"compEnabled", "Compressor Footswitch", -1, 1, true});
        mappings.push_back({"widthEnabled", "Haas Width Footswitch", -1, 1, true});

        mappings.push_back({"gateThreshold", "Gate Threshold", -1, 1, false});
        mappings.push_back({"compRatio", "Compressor Ratio", -1, 1, false});
        mappings.push_back({"compThreshold", "Compressor Threshold", -1, 1, false});

        mappings.push_back({"pitchVoicing", "Pitch Voicing Select", -1, 1, false});
        mappings.push_back({"lfoTarget", "LFO Target Select", -1, 1, false});
        mappings.push_back({"lfoPhaseInvert", "LFO 180 Phase Invert", -1, 1, true});
        mappings.push_back({"lfoBpmSync", "LFO Sync Toggle", -1, 1, true});
        mappings.push_back({"lfoRate", "LFO Rate", -1, 1, false});
        mappings.push_back({"lfoDivision", "LFO Sync Division", -1, 1, false});

        mappings.push_back({"distortionDrive", "Carnage Drive", -1, 1, false});
        mappings.push_back({"distortionRadioMode", "Megaphone Radio Toggle", -1, 1, true});
        mappings.push_back({"reverbMode", "Reverb Space Select", -1, 1, false});
        mappings.push_back({"reverbMix", "Reverb Mix", -1, 1, false});

        mappings.push_back({"inputRouting", "Input Routing Select", -1, 1, false});
        mappings.push_back({"inputGain", "Input Gain", -1, 1, false});
        mappings.push_back({"outputGain", "Output Gain", -1, 1, false});
    }

    void startLearning(const std::string& paramId)
    {
        std::lock_guard<std::mutex> lock(mutex);
        learningParamId = paramId;
        isLearningActive = true;
    }

    void stopLearning()
    {
        std::lock_guard<std::mutex> lock(mutex);
        learningParamId = "";
        isLearningActive = false;
    }

    bool isLearning() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return isLearningActive;
    }

    std::string getLearningParamId() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return learningParamId;
    }

    void clearMapping(const std::string& paramId)
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& m : mappings)
        {
            if (m.paramId == paramId)
            {
                m.ccNumber = -1;
                break;
            }
        }
        saveMappings();
    }

    void setMapping(const std::string& paramId, int cc, int channel)
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& m : mappings)
        {
            if (m.paramId == paramId)
            {
                m.ccNumber = cc;
                m.channel = channel;
                break;
            }
        }
        saveMappings();
    }

    std::vector<MidiMapping> getMappings() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return mappings;
    }

    void processMidiBuffer(juce::MidiBuffer& midiMessages, juce::AudioProcessorValueTreeState& apvts)
    {
        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            if (msg.isController())
            {
                const int cc = msg.getControllerNumber();
                const int chan = msg.getChannel();
                const int val = msg.getControllerValue();
                const float normVal = static_cast<float>(val) / 127.0f;

                std::lock_guard<std::mutex> lock(mutex);

                // Handle MIDI Learn
                if (isLearningActive && !learningParamId.empty())
                {
                    for (auto& m : mappings)
                    {
                        if (m.paramId == learningParamId)
                        {
                            m.ccNumber = cc;
                            m.channel = chan;
                            break;
                        }
                    }
                    isLearningActive = false;
                    learningParamId = "";
                    saveMappings();
                    continue;
                }

                // Handle Mapped CC messages
                for (const auto& m : mappings)
                {
                    if (m.ccNumber == cc && (m.channel == 0 || m.channel == chan))
                    {
                        auto* param = apvts.getParameter(m.paramId);
                        if (param != nullptr)
                        {
                            if (m.isToggle)
                            {
                                if (val >= 64)
                                {
                                    const float current = param->getValue();
                                    param->setValueNotifyingHost(current > 0.5f ? 0.0f : 1.0f);
                                }
                            }
                            else
                            {
                                param->setValueNotifyingHost(normVal);
                            }
                        }
                    }
                }
            }
        }
    }

private:
    juce::File getConfigFile() const
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("THE EDGE OF FEAR")
                       .getChildFile("THE VOID");
        if (!dir.exists())
            dir.createDirectory();
        return dir.getChildFile("MidiMappings.json");
    }

    void loadMappings()
    {
        auto file = getConfigFile();
        if (!file.existsAsFile())
            return;

        auto parsed = juce::JSON::parse(file);
        if (!parsed.isArray())
            return;

        auto* arr = parsed.getArray();
        for (const auto& var : *arr)
        {
            if (!var.isObject())
                continue;

            const std::string pId = var.getProperty("paramId", "").toString().toStdString();
            const int cc = static_cast<int>(var.getProperty("cc", -1));
            const int ch = static_cast<int>(var.getProperty("channel", 1));

            for (auto& m : mappings)
            {
                if (m.paramId == pId)
                {
                    m.ccNumber = cc;
                    m.channel = ch;
                    break;
                }
            }
        }
    }

    void saveMappings()
    {
        juce::Array<juce::var> arr;
        for (const auto& m : mappings)
        {
            juce::DynamicObject* obj = new juce::DynamicObject();
            obj->setProperty("paramId", juce::String(m.paramId));
            obj->setProperty("cc", m.ccNumber);
            obj->setProperty("channel", m.channel);
            arr.add(juce::var(obj));
        }

        auto file = getConfigFile();
        juce::FileOutputStream fos(file);
        if (fos.openedOk())
        {
            file.replaceWithText(juce::JSON::toString(juce::var(arr)));
        }
    }

    mutable std::mutex mutex;
    std::vector<MidiMapping> mappings;
    bool isLearningActive = false;
    std::string learningParamId;
};
