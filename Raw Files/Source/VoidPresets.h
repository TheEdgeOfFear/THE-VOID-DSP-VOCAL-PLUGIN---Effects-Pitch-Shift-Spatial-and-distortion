#pragma once

#include <string>
#include <vector>

namespace VoidPresets
{

struct Preset
{
    std::string category;
    std::string name;
    std::string description;
    bool isFactory = true;

    int inputRouting = 0;         // 0=CH 1, 1=CH 2, 2=1+2
    float inputGain = 0.0f;       // dB (-24 to +12)
    float outputGain = 0.0f;      // dB (-24 to +12)
    float gateThreshold = -50.0f; // dB (-80 to 0)
    int compRatio = 1;            // 0=2:1, 1=4:1, 2=8:1, 3=12:1
    float compThreshold = -18.0f; // dB (-40 to 0)
    bool compEnabled = true;
    int pitchVoicing = 2;         // 0=-12, 1=-24, 2=Both
    int lfoTarget = 0;            // 0=Off, 1=-12, 2=-24, 3=Both
    bool lfoPhaseInvert = false;
    bool lfoBpmSync = false;
    float lfoRate = 1.0f;         // Hz
    int lfoDivision = 3;          // 1/4 note
    bool pitchEnabled = true;
    float distortionDrive = 0.35f;// 0.0 to 1.0
    bool distortionRadioMode = false; // Marilyn Manson / Nathan James Megaphone Radio Mode
    bool distortionEnabled = true;
    bool widthEnabled = true;
    int reverbMode = 1;           // 0=Bedroom, 1=Dungeon, 2=Cave, 3=Abyss
    float reverbMix = 0.25f;      // 0.0 to 1.0
    bool masterEnabled = true;
};

inline std::vector<Preset> getBuiltInPresets()
{
    return {
        // ==========================================
        // Iconic Megaphone & Radio Vocals
        // ==========================================
        {
            "Iconic Megaphone & Radio",
            "Marilyn Manson - New Shit",
            "Iconic breakdown vocal: aggressive industrial radio megaphone bandpass, hard transistor saturation, tight room.",
            true,
            0, 0.0f, 0.0f, -48.0f, 1, -16.0f, true, 0, 0, false, false, 1.0f, 3, false, 0.65f, true, true, true, 0, 0.20f, true
        },
        {
            "Iconic Megaphone & Radio",
            "Nathan James - The Hanged Man",
            "High-gain dirty megaphone distorted scream with fast 4:1 compression and wide stereo Haas wall.",
            true,
            0, 1.0f, -0.5f, -46.0f, 1, -18.0f, true, 0, 0, false, false, 1.0f, 3, false, 0.75f, true, true, true, 1, 0.25f, true
        },
        {
            "Iconic Megaphone & Radio",
            "Paleface - Secrets of Shadows",
            "Industrial megaphone distortion with eerie modulated sub-octave undertones and dark cavern space.",
            true,
            0, 0.0f, -1.0f, -44.0f, 2, -16.0f, true, 2, 3, true, false, 0.4f, 3, true, 0.80f, true, true, true, 2, 0.38f, true
        },
        {
            "Iconic Megaphone & Radio",
            "Industrial Intercom Horn",
            "Raw telephone/intercom frequency cutoff with screaming diode bite for metallic vocal cuts.",
            true,
            0, 0.0f, 0.0f, -42.0f, 3, -14.0f, true, 0, 0, false, false, 1.0f, 3, false, 0.85f, true, true, true, 0, 0.12f, true
        },

        // ==========================================
        // Demonic & Sub-Harmonic
        // ==========================================
        {
            "Demonic & Sub-Harmonic",
            "The Void Master",
            "The flagship vocal chain: crushing dual sub-octaves, high-cut saturation, wide Haas, and Dungeon reverb.",
            true,
            0, 0.0f, 0.0f, -50.0f, 1, -18.0f, true, 2, 0, false, false, 1.0f, 3, true, 0.40f, false, true, true, 1, 0.28f, true
        },
        {
            "Demonic & Sub-Harmonic",
            "Abyssal Sub-Growl",
            "Pure -24 semitone sub-octave monster tuned for lowest death metal gutturals.",
            true,
            0, 0.0f, 1.0f, -48.0f, 2, -16.0f, true, 1, 0, false, false, 0.8f, 3, true, 0.30f, false, true, true, 2, 0.35f, true
        },
        {
            "Demonic & Sub-Harmonic",
            "Dual Shearing Beast",
            "Both sub-octaves swept in antiphase (180° inverted LFO) creating extreme demonic pitch shearing.",
            true,
            0, 0.0f, 0.0f, -45.0f, 1, -18.0f, true, 2, 3, true, false, 0.65f, 3, true, 0.45f, false, true, true, 1, 0.30f, true
        },
        {
            "Demonic & Sub-Harmonic",
            "Slamming Gutturals",
            "Fast gate, 12:1 brutal compression, high Carnage drive, and -12 sub-octave punch.",
            true,
            0, 1.5f, -1.0f, -42.0f, 3, -15.0f, true, 0, 0, false, false, 1.0f, 3, true, 0.60f, false, true, true, 0, 0.18f, true
        },

        // ==========================================
        // Extreme Metal & Core
        // ==========================================
        {
            "Extreme Metal & Core",
            "Deathcore Pig Squeal",
            "Tight noise gate with presence boost distortion and wide Haas wall for high shrieks.",
            true,
            0, 0.0f, 0.0f, -46.0f, 2, -18.0f, true, 0, 0, false, false, 1.0f, 3, false, 0.75f, false, true, true, 1, 0.22f, true
        },
        {
            "Extreme Metal & Core",
            "Black Metal Cold Shriek",
            "Razor-sharp presence boost distortion with Ominous Cave reflections and -12 tracking.",
            true,
            0, 0.0f, 0.0f, -52.0f, 1, -20.0f, true, 0, 0, false, false, 1.0f, 3, true, 0.65f, false, true, true, 2, 0.40f, true
        },
        {
            "Extreme Metal & Core",
            "Industrial Cyber-Crush",
            "Tempo-synced 1/8th note pitch sweeps with full-on Carnage drive and Abyss shimmer.",
            true,
            0, 0.0f, -1.0f, -48.0f, 2, -16.0f, true, 2, 3, true, true, 2.0f, 6, true, 0.85f, false, true, true, 3, 0.35f, true
        },
        {
            "Extreme Metal & Core",
            "Nu-Metal Slapback",
            "Tight bedroom ambiance, punchy 4:1 compression, and discrete -12 sub layer.",
            true,
            0, 0.0f, 0.0f, -55.0f, 1, -16.0f, true, 0, 0, false, false, 1.0f, 3, true, 0.35f, false, true, true, 0, 0.25f, true
        },

        // ==========================================
        // Atmospheric & Eerie
        // ==========================================
        {
            "Atmospheric & Eerie",
            "The Endless Crypt",
            "Cavernous Abyss reverb with slow creeping pitch modulation and massive Haas stereo width.",
            true,
            0, 0.0f, 0.0f, -60.0f, 0, -22.0f, true, 2, 3, false, false, 0.15f, 0, true, 0.20f, false, true, true, 3, 0.65f, true
        },
        {
            "Atmospheric & Eerie",
            "Dungeon Echoes",
            "Cold stone dungeon reflections with subtle demonic undertones.",
            true,
            0, 0.0f, 0.0f, -54.0f, 1, -20.0f, true, 0, 0, false, false, 0.5f, 3, true, 0.25f, false, true, true, 1, 0.45f, true
        },
        {
            "Atmospheric & Eerie",
            "Haunted Whispers",
            "Tight noise gate with long dark cave decay for vocal soundscapes.",
            true,
            0, 2.0f, 0.0f, -44.0f, 0, -24.0f, true, 1, 0, false, false, 1.0f, 3, false, 0.15f, false, false, true, 2, 0.55f, true
        },
        {
            "Atmospheric & Eerie",
            "Cavernous Hallucination",
            "1/2 beat tempo-synced dual pitch glide in antiphase with 3.5s cavern space.",
            true,
            0, 0.0f, 0.0f, -50.0f, 1, -18.0f, true, 2, 3, true, true, 1.0f, 1, true, 0.40f, false, true, true, 2, 0.50f, true
        },

        // ==========================================
        // Studio Dynamics & Utility
        // ==========================================
        {
            "Studio Dynamics & Utility",
            "Vocal Gate & Leveler",
            "Fast noise gate and clean 8:1 optical compression for harsh vocal tracking.",
            true,
            0, 0.0f, 0.0f, -48.0f, 2, -18.0f, true, 0, 0, false, false, 1.0f, 3, false, 0.0f, false, false, false, 0, 0.0f, true
        },
        {
            "Studio Dynamics & Utility",
            "Default Init",
            "Standard factory default starting position (Mono Input CH 1 -> Stereo Out).",
            true,
            0, 0.0f, 0.0f, -50.0f, 1, -18.0f, true, 2, 0, false, false, 1.0f, 3, true, 0.35f, false, true, true, 1, 0.25f, true
        }
    };
}

} // namespace VoidPresets
