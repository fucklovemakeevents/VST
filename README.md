# Fuck Love Make Music Vox Clean

**Vendor:** Fuck Love Make Music Label  
**Target:** Windows VST3 / FL Studio  
**Version:** 0.1.0 development scaffold

## What's included in this prototype

- JUCE + CMake project setup for VST3 and standalone builds.
- Dark, lime-accented custom plugin interface.
- Live input activity spectrum display.
- Adjustable high-pass filter and three parametric EQ bands (low-mid, presence, air).
- Output gain control.
- Threshold-based adaptive envelope expander/gate with a Deep Clean toggle.
- Saved/restored plugin parameters.

## Important status note

This is an **early development scaffold**, not yet the promised finished restoration product. The current noise-control section is an envelope-based expander/gate; it is **not yet a true STFT spectral denoiser**, and Deep Clean currently increases the gate threshold rather than running an AI source-separation model. The live spectrum display currently visualizes input activity rather than a calibrated frequency spectrum. These modules are explicitly identified as the next implementation tasks rather than being represented as completed features.

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 with Desktop development with C++
- CMake 3.22 or newer
- Git (CMake fetches JUCE 8.0.8 from GitHub on the first configure)

## Build

Open a Developer PowerShell for VS 2022, navigate to this folder, then run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

The VST3 bundle will normally be under:

`build/FLMMVoxClean_artefacts/Release/VST3/Fuck Love Make Music Vox Clean.vst3`

Copy the `.vst3` bundle to:

`C:\Program Files\Common Files\VST3\`

Then open FL Studio and use **Options > Manage plugins > Find installed plugins**.

## Roadmap

1. Replace the expander with STFT-based spectral denoise and adjustable noise-profile learning.
2. Implement a real calibrated FFT spectrum analyzer and EQ response curve.
3. Add offline Deep Clean processing and safe file-render workflow.
4. Add optional neural vocal/instrument separation, with clear CPU/latency requirements and model licensing checks.
5. Add A/B, delta/noise audition, presets, oversampling where useful, and test against real Suno vocal examples.

## License and distribution

JUCE has its own licensing terms. Before distributing a commercial build, review the current JUCE license and any third-party model/library licenses. The project name and vendor metadata are configured for Fuck Love Make Music Label.
