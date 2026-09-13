#pragma once

#include "VoidPresets.h"
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include <algorithm>

namespace VoidPresets
{

class PresetManager
{
public:
    PresetManager()
    {
        presets = getBuiltInPresets();
        loadUserPresets();
    }

    const std::vector<Preset>& getAllPresets() const { return presets; }

    std::vector<std::string> getCategories() const
    {
        std::vector<std::string> cats;
        for (const auto& p : presets)
        {
            if (std::find(cats.begin(), cats.end(), p.category) == cats.end())
                cats.push_back(p.category);
        }
        return cats;
    }

    std::vector<Preset> getPresetsInCategory(const std::string& cat) const
    {
        std::vector<Preset> res;
        for (const auto& p : presets)
        {
            if (p.category == cat)
                res.push_back(p);
        }
        return res;
    }

    int getGlobalIndex(const std::string& cat, int categoryIndex) const
    {
        int count = 0;
        for (size_t i = 0; i < presets.size(); ++i)
        {
            if (presets[i].category == cat)
            {
                if (count == categoryIndex)
                    return static_cast<int>(i);
                count++;
            }
        }
        return 0;
    }

    bool saveUserPreset(const Preset& newPreset)
    {
        Preset p = newPreset;
        p.isFactory = false;

        auto it = std::find_if(presets.begin(), presets.end(),
                               [&](const Preset& existing) {
                                   return !existing.isFactory && existing.name == p.name;
                               });

        if (it != presets.end())
            *it = p;
        else
            presets.push_back(p);

        saveUserPresetsToFile();
        return true;
    }

    bool deleteUserPreset(int globalIndex)
    {
        if (globalIndex < 0 || globalIndex >= static_cast<int>(presets.size()))
            return false;

        if (presets[static_cast<size_t>(globalIndex)].isFactory)
            return false; // Cannot delete factory preset

        presets.erase(presets.begin() + globalIndex);
        saveUserPresetsToFile();
        return true;
    }

private:
    std::vector<Preset> presets;

    juce::File getPresetsFile() const
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("THE EDGE OF FEAR")
                       .getChildFile("THE VOID");
        if (!dir.exists())
            dir.createDirectory();
        return dir.getChildFile("UserPresets.json");
    }

    void loadUserPresets()
    {
        auto file = getPresetsFile();
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

            Preset p;
            p.isFactory = false;
            p.category = var.getProperty("category", "Custom User").toString().toStdString();
            p.name = var.getProperty("name", "Untitled").toString().toStdString();
            p.description = var.getProperty("description", "").toString().toStdString();
            p.inputRouting = static_cast<int>(var.getProperty("inputRouting", 0));
            p.inputGain = static_cast<float>(var.getProperty("inputGain", 0.0));
            p.outputGain = static_cast<float>(var.getProperty("outputGain", 0.0));
            p.gateThreshold = static_cast<float>(var.getProperty("gateThreshold", -50.0));
            p.compRatio = static_cast<int>(var.getProperty("compRatio", 1));
            p.compThreshold = static_cast<float>(var.getProperty("compThreshold", -18.0));
            p.compEnabled = static_cast<bool>(var.getProperty("compEnabled", true));
            p.pitchVoicing = static_cast<int>(var.getProperty("pitchVoicing", 2));
            p.lfoTarget = static_cast<int>(var.getProperty("lfoTarget", 0));
            p.lfoPhaseInvert = static_cast<bool>(var.getProperty("lfoPhaseInvert", false));
            p.lfoBpmSync = static_cast<bool>(var.getProperty("lfoBpmSync", false));
            p.lfoRate = static_cast<float>(var.getProperty("lfoRate", 1.0));
            p.lfoDivision = static_cast<int>(var.getProperty("lfoDivision", 3));
            p.pitchEnabled = static_cast<bool>(var.getProperty("pitchEnabled", true));
            p.distortionDrive = static_cast<float>(var.getProperty("distortionDrive", 0.35));
            p.distortionRadioMode = static_cast<bool>(var.getProperty("distortionRadioMode", false));
            p.distortionEnabled = static_cast<bool>(var.getProperty("distortionEnabled", true));
            p.widthEnabled = static_cast<bool>(var.getProperty("widthEnabled", true));
            p.reverbMode = static_cast<int>(var.getProperty("reverbMode", 1));
            p.reverbMix = static_cast<float>(var.getProperty("reverbMix", 0.25));
            p.masterEnabled = static_cast<bool>(var.getProperty("masterEnabled", true));

            presets.push_back(p);
        }
    }

    void saveUserPresetsToFile()
    {
        juce::Array<juce::var> arr;
        for (const auto& p : presets)
        {
            if (p.isFactory)
                continue;

            juce::DynamicObject* obj = new juce::DynamicObject();
            obj->setProperty("category", juce::String(p.category));
            obj->setProperty("name", juce::String(p.name));
            obj->setProperty("description", juce::String(p.description));
            obj->setProperty("inputRouting", p.inputRouting);
            obj->setProperty("inputGain", p.inputGain);
            obj->setProperty("outputGain", p.outputGain);
            obj->setProperty("gateThreshold", p.gateThreshold);
            obj->setProperty("compRatio", p.compRatio);
            obj->setProperty("compThreshold", p.compThreshold);
            obj->setProperty("compEnabled", p.compEnabled);
            obj->setProperty("pitchVoicing", p.pitchVoicing);
            obj->setProperty("lfoTarget", p.lfoTarget);
            obj->setProperty("lfoPhaseInvert", p.lfoPhaseInvert);
            obj->setProperty("lfoBpmSync", p.lfoBpmSync);
            obj->setProperty("lfoRate", p.lfoRate);
            obj->setProperty("lfoDivision", p.lfoDivision);
            obj->setProperty("pitchEnabled", p.pitchEnabled);
            obj->setProperty("distortionDrive", p.distortionDrive);
            obj->setProperty("distortionRadioMode", p.distortionRadioMode);
            obj->setProperty("distortionEnabled", p.distortionEnabled);
            obj->setProperty("widthEnabled", p.widthEnabled);
            obj->setProperty("reverbMode", p.reverbMode);
            obj->setProperty("reverbMix", p.reverbMix);
            obj->setProperty("masterEnabled", p.masterEnabled);

            arr.add(juce::var(obj));
        }


        auto file = getPresetsFile();
        juce::FileOutputStream fos(file);
        if (fos.openedOk())
        {
            file.replaceWithText(juce::JSON::toString(juce::var(arr)));
        }
    }
};

} // namespace VoidPresets
