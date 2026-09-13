# THE VOID

### Heavy-Duty Distressed Industrial Vocal Multi-Effects Processor, Pitch Harmonizer & Spatializer DSP Plugin
**Created by THE EDGE OF FEAR**

![THE VOID](THE%20VOID.png)

---

## Overview

**THE VOID** is a boutique, studio-grade vocal and instrument DSP multi-effects processor built from the ground up in modern **C++20** with the **JUCE 8** framework. Designed specifically for aggressive modern metal, industrial rock, deathcore, dark electronic, and cinematic sound design, **THE VOID** unifies high-gain vocal destruction, sub-harmonic pitch shifting, psychoacoustic spatial width, and vintage telephone/megaphone bandpass processing into a hardware-inspired pedalboard interface.

Whether tracking razor-sharp industrial screams like *Marilyn Manson* and *Nathan James*, layering demonic -12/-24 semitone sub-octave growls, or creating cavernous dark ambient vocal soundscapes, **THE VOID** delivers pristine, zero-latency 32-bit floating-point audio with atomic parameter smoothing.

---

## Repository Structure

```
THE-VOID-DSP-VOCAL-PLUGIN---Effects-Pitch-Shift-Spatial-and-distortion/
├── VST3/
│   ├── THE VOID.vst3/                           # Complete 64-bit VST3 bundle with moduleinfo
│   └── INSTALL_THE_VOID_VST3.bat                # Automated 1-click Windows VST3 Installer
├── DLL/
│   └── THE VOID.dll                             # Standalone 64-bit VST/PE dynamic library
├── Standalone/
│   └── THE VOID.exe                             # Zero-host standalone application with native ASIO
├── Raw Files/
│   ├── Source/                                  # Complete C++20 DSP and GUI source code
│   │   ├── DSP/                                 # Modular DSP engines (Distortion, Pitch, Gate, Comp, Haas, Reverb)
│   │   ├── PluginProcessor.*                    # Audio engine & APVTS parameter layouts
│   │   ├── PluginEditor.*                       # Hardware-textured graphical interface
│   │   ├── VoidLookAndFeel.h                   # Custom industrial UI widgets & glowing jewel LEDs
│   │   ├── VoidPresets.h                        # Factory preset definitions
│   │   ├── PresetManager.h                      # JSON preset persistence manager
│   │   └── MidiManager.h                        # Universal MIDI CC learn engine
│   ├── BACKGROUND IMAGE/                        # Embedded background artwork assets & renders
│   ├── asiosdk/                                 # Steinberg ASIO SDK headers
│   └── CMakeLists.txt                           # Self-contained CMake multi-target build script
├── THE VOID.png                                 # Hero interface graphic
├── LICENSE                                      # Project license
└── README.md                                    # Documentation and user manual
```

---

## Signal Architecture & Routing

```
                                  [INPUT ROUTING]
                      (CH 1 Mono -> Stereo / CH 2 / 1+2 Stereo)
                                         │
                                         ▼
                                  [INPUT GAIN TRIM]
                                (-24 dB to +12 dB)
                                         │
                                         ▼
                                 [PRE-FX NOISE GATE]
                                 (-80 dB to 0 dB)
                                         │
                                         ▼
                             [STEPPED OPTICAL COMPRESSOR]
                           (2:1, 4:1, 8:1, 12:1 Ratios)
                                         │
                                         ▼
                           [DUAL PITCH ENGINE & LFO GLIDE]
                      (-12 ST Sub / -24 ST Growl / Antiphase LFO)
                                         │
                                         ▼
                         ["CARNAGE" DYNAMIC DISTORTION]
                  (Transistor Saturation + Megaphone / Radio Mode)
                                         │
                                         ▼
                                [HAAS STEREO WIDTH]
                             (15ms Psychoacoustic Wall)
                                         │
                                         ▼
                           ["THE CRYPT" ALGORITHMIC REVERB]
                      (Bedroom, Dungeon Room, Cave, The Abyss)
                                         │
                                         ▼
                                 [OUTPUT GAIN TRIM]
                                (-24 dB to +12 dB)
                                         │
                                         ▼
                           [MASTER FOOTSWITCH & TRUE BYPASS]
```

---

## Key Features

### 1. Flexible Input Channel Routing (`CH 1`, `CH 2`, `1+2`)
- **Mono-to-Stereo Duplicate Mode (`CH 1` Default)**: Specifically built for standard audio interfaces where vocal microphones arrive on Channel 1 (Left). The plugin automatically duplicates Channel 1 into both Left and Right internal signal paths before hitting the DSP chain. This guarantees full stereo width and prevents Output Channel 2 from suffering silence or drop-off.
- **Channel 2 Mode (`CH 2`)**: Direct routing for microphones plugged into input 2.
- **Dual Tracking Mode (`1+2`)**: Full stereo pass-through for stereo microphone pairs, backing tracks, or guitar buses.

### 2. Pre-FX High-Speed Noise Gate
- **Dynamic Noise Floor Clamping**: Optical response curve with fast hysteresis prevents breath noise, hum, and high-gain feedback between aggressive vocal phrases.
- **Threshold**: Continuous range from `-80.0 dB` to `0.0 dB`.

### 3. Stepped Optical / VCA Compressor
- **Studio-Calibrated Ratios**: 4 tactile ratio stages (`2:1` Leveling, `4:1` Punchy Vocal, `8:1` Hard Limiting, `12:1` Brickwall Smash).
- **Auto-Compensated Makeup Gain**: Integrated soft-knee program-dependent auto makeup ensures consistent vocal presence without sudden volume drops.
- **Independent Stomp**: Dedicated `COMP` footswitch with glowing LED status.

### 4. Dual Pitch Shifter Engine & Inverted LFO Modulator
- **Voicing Selector**: 3 illuminated modes:
  - `-12 ST`: Tight sub-octave harmonizer for thick industrial vocal doubling.
  - `-24 ST`: Deep demonic sub-growl for crushing metal vocals.
  - `BOTH`: Dual-engine octave stacking simultaneously tracking in parallel.
- **Antiphase LFO Sweeper**:
  - `180° PHASE INVERT`: Inverts modulation between the two pitch engines, creating extreme demonic vocal shearing and spatial movement.
  - `BPM SYNC`: Locks LFO modulation to DAW host tempo across 11 musical divisions (`1/1`, `1/2`, `1/4`, `1/8`, `1/16`, triplets, and dotted).
  - `FREE RATE`: Smooth manual rate adjustment from `0.05 Hz` to `15.0 Hz`.
- **Independent Stomp**: Dedicated `PITCH` footswitch.

### 5. "Carnage" Dynamic Distortion & Megaphone / Radio Mode
- **Transistor Overdrive**: Smooth single-knob drive that introduces rich odd-order harmonics and aggressive saturation.
- **Iconic Megaphone / Radio Mode**: Inspired by *Marilyn Manson - This Is The New Shit*, *Nathan James - The Hanged Man*, and *Paleface Swiss - Secrets of the Shadows*:
  - **Bandpass Pre-Emphasis**: Steep 24 dB/octave Butterworth highpass ($380\text{ Hz}$) and lowpass ($3.8\text{ kHz}$) filters.
  - **Horn Resonance Peak**: $+9.0\text{ dB}$ resonant boost at $2.2\text{ kHz}$ ($Q=2.2$) emulating vintage PA horn drivers and intercoms.
  - **$4\times$ Oversampled Clipping**: Symmetrical and asymmetrical diode/transistor curve for blistering metallic bite.
- **Independent Stomp**: Dedicated `CARNAGE` footswitch.

### 6. Haas Stereo Width Expander
- **15ms Psychoacoustic Time Delay**: Delays the right channel to exploit the Haas effect, transforming mono vocal tracks into an expansive wall of sound without phase cancellation.
- **Independent Stomp**: Dedicated `WIDTH` footswitch.

### 7. "The Crypt" Algorithmic Reverb
- **4 Distinct Reverb Spaces**:
  - **Alex in Bedroom**: Tight, explosive small-room reflections for aggressive modern rap-metal and industrial vocals.
  - **Dungeon Room**: Dark, damp stone room with medium decay and cold early reflections.
  - **Ominous Cave**: Expansive 3.5s cavern decay with natural diffusion and dark damping.
  - **The Abyss**: Massive infinite shimmer space for eerie intros, breakdowns, and dark atmospheres.
- **Wet/Dry Mix Control**: Seamless linear-to-exponential mix knob.

### 8. Smart Momentary & Latch Footswitches
- **Dual-Action Stomp Control**:
  - **Quick Tap (<280ms)**: Latches module state ON or OFF.
  - **Hold (>280ms)**: Enters momentary mode; the module stays active only while the footswitch is held down and bypasses automatically upon release—ideal for live punches, scream accents, or stutter effects.
- **Glowing Jewel LEDs**: Neon crimson status LEDs with dynamic glow diffusion.

### 9. Multi-Category Preset Manager
- **16 Production-Ready Factory Presets**:
  - *Iconic Megaphone & Radio*: Marilyn Manson - New Shit, Nathan James - The Hanged Man, Paleface - Secrets of Shadows, Industrial Intercom Horn.
  - *Demonic & Sub-Harmonic*: The Void Master, Abyssal Sub-Growl, Dual Shearing Beast, Slamming Gutturals.
  - *Extreme Metal & Core*: Deathcore Pig Squeal, Black Metal Cold Shriek, Industrial Cyber-Crush, Nu-Metal Slapback.
  - *Atmospheric & Eerie*: The Endless Crypt, Dungeon Echoes, Haunted Whispers, Cavernous Hallucination.
  - *Studio Dynamics & Utility*: Vocal Gate & Leveler, Default Init.
- **User Preset Management**: Save custom presets with one click (persisted automatically to JSON in AppData).

### 10. Universal MIDI CC Mapping & MIDI Learn
- **Right-Click Learning**: Right-click any dial, switch, or footswitch to engage MIDI Learn or open the full mapping manager.
- **Full Hardware Controller Support**: Seamlessly maps to foot controllers (Behringer FCB1010, Line 6 FBV), MIDI keyboards, and motorized fader surfaces.

### 11. Native ASIO Low-Latency Driver Engine
- Standalone executable is compiled with native **Steinberg ASIO SDK** support for low-latency live performance (buffer sizes down to 32/64 samples) on Focusrite, Universal Audio, RME, MOTU, Behringer, PreSonus, and ASIO4ALL interfaces.

---

## How to Install

### Option A: 1-Click Automated VST3 Installer (Recommended for Windows)
1. Open the `VST3/` folder.
2. Right-click **`INSTALL_THE_VOID_VST3.bat`** and choose **Run as Administrator** (or double-click; it will automatically request elevation).
3. The script will clean any previous builds and install `THE VOID.vst3` directly into:
   ```
   C:\Program Files\Common Files\VST3\THE VOID.vst3
   ```
4. Open your DAW (Cakewalk Sonar, Reaper, Cubase, FL Studio, Ableton Live, Studio One, Pro Tools via VST wrapper, Bitwig) and trigger a VST plugin rescan.

### Option B: Manual VST3 Installation
1. Copy the entire `THE VOID.vst3` folder from `VST3/` into your system's VST3 directory:
   ```
   C:\Program Files\Common Files\VST3\
   ```
2. Rescan plugins in your DAW.

### Option C: Manual DLL Installation
1. Copy `THE VOID.dll` from `DLL/` into your custom VST 64-bit plugin directory (e.g., `C:\Program Files\VstPlugins\` or `C:\Program Files\Steinberg\VstPlugins\`).
2. Rescan plugins in your DAW.

### Option D: Standalone Application (No DAW Required)
1. Open the `Standalone/` folder.
2. Launch **`THE VOID.exe`**.
3. In the top audio settings menu:
   - Select **Audio Device Type: ASIO**.
   - Choose your audio interface (e.g., *Focusrite USB ASIO*, *Universal Audio Thunderbolt*, *MOTU Pro Audio*, etc.).
   - Set Sample Rate (44.1 kHz, 48 kHz, 96 kHz) and Audio Buffer Size (64 to 256 samples).

---

## How to Use the DSP Vocal & Guitar Multi-Effects Pedal

### 1. Setting Up Input Routing for Microphone Tracking
1. Connect your vocal microphone to **Input 1** of your audio interface.
2. Ensure the **`INPUT ROUTE`** switch in the *DYNAMICS & CONTROL* panel is set to **`CH 1`**.
3. Single-channel microphone audio is now mirrored across internal Left and Right channels, providing full stereo output with width and reverb across both output speakers/headphones.

### 2. Dialing in the Megaphone / Radio Screaming Vocal
1. In the preset browser, select **Iconic Megaphone & Radio -> Marilyn Manson - New Shit** or **Nathan James - The Hanged Man**.
2. Alternatively, set it up manually:
   - Toggle **`RADIO / MEGAPHONE`** ON in the *CARNAGE & THE CRYPT* panel (jewel LED lights up).
   - Set **`CARNAGE DRIVE`** between `0.60` and `0.85` for aggressive saturation.
   - Set **`COMP RATIO`** to `4:1` or `8:1` and lower **`COMP THRESH`** to around `-18 dB`.
   - Set **`REVERB SPACE`** to `BEDROOM` with **`REVERB MIX`** at `0.20` for tight, claustrophobic industrial presence.

### 3. Creating Demonic Sub-Harmonic Vocals & Shearing Effects
1. Engage the **`PITCH`** footswitch.
2. In the *PITCH ENGINE & LFO* panel:
   - Set **`OCTAVE SELECT`** to `BOTH` (engages both -12 ST and -24 ST engines).
   - Set **`LFO TARGET`** to `BOTH`.
   - Toggle **`180° PHASE`** ON to modulate the two sub-octaves in antiphase.
   - Toggle **`BPM SYNC`** ON and select `1/4` or `1/8` division for tempo-locked demonic swells.
3. Engage **`WIDTH`** and **`THE CRYPT`** (set to `CAVE` or `ABYSS`) for massive spatial depth.

### 4. Using Smart Momentary Punch-Ins Live
1. If a footswitch (e.g., `CARNAGE` or `PITCH`) is currently OFF:
   - **Click and hold** the footswitch during a specific scream or breakdown phrase.
   - The effect remains active for the duration of your hold.
   - Release your mouse or MIDI pedal immediately after the phrase to return to clean vocal bypass without touching additional toggles.

### 5. Mapping External MIDI Pedals & Hardware
1. Right-click any control (e.g., `CARNAGE DRIVE`, `PITCH FOOTSWITCH`, or `REVERB MIX`).
2. Select **Learn MIDI CC**.
3. Move your hardware foot controller pedal or turn a MIDI knob. The parameter will lock instantly.
4. Click **MIDI MAP** in the top bar to inspect, edit channels, or clear mappings.

---

## Building from Source

### Prerequisites
- **Operating System**: Windows 10 / 11 (64-bit)
- **Compiler**: Visual Studio 2022 (MSVC v143 or newer) with C++20 support
- **Build System**: CMake 3.22+
- **Git**

### Build Steps
```powershell
# Clone the repository
git clone https://github.com/TheEdgeOfFear/THE-VOID-DSP-VOCAL-PLUGIN---Effects-Pitch-Shift-Spatial-and-distortion.git
cd "THE-VOID-DSP-VOCAL-PLUGIN---Effects-Pitch-Shift-Spatial-and-distortion/Raw Files"

# Configure CMake build directory (JUCE 8 FetchContent is automated)
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build Release binaries (VST3 & Standalone)
cmake --build build --config Release --target TheVoid_All
```

---

## License & Credits

- **Developer**: [THE EDGE OF FEAR](https://github.com/TheEdgeOfFear)
- **Framework**: JUCE 8.0.4 (C++20)
- **DSP Core**: Custom Real-Time Pitch Shifter, Antiphase LFO Sweeper, Bandpass Transistor Distortion, Haas Spatializer, and Algorithmic Reverb
- **License**: See [LICENSE](LICENSE) file.
