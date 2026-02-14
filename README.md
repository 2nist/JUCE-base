# JUCE VST Development Environment (`2nist`)

This repository is now a working baseline for JUCE VST3 development on Windows with CMake.

## What is included

- JUCE plugin project using `FetchContent` (no manual JUCE copy needed).
- A starter VST3 + Standalone MIDI effect plugin: `2nist MIDI Arranger`.
- Build/test automation scripts in `scripts/`.
- Plain-language workflow and best-practice docs in `docs/`.

## Quick start (Windows)

1. Run toolchain check:
```powershell
.\scripts\setup-windows.ps1
```
2. If C++ build tools are missing, install them:
```powershell
.\scripts\setup-windows.ps1 -InstallBuildTools
```
3. Configure:
```powershell
.\scripts\configure.ps1
```
4. Build:
```powershell
.\scripts\build.ps1 -Config Debug
```
5. Test:
```powershell
.\scripts\test.ps1
```

## Where the plugin lands

For this repo's default build (`build_nmake`), the VST3 output is:

- `build_nmake\2nist_artefacts\Debug\VST3\2nist MIDI Arranger.vst3`

Standalone app is under:

- `build_nmake\2nist_artefacts\Debug\Standalone\`

## External tool that adds major value

Use **REAPER** as your daily plugin host while developing.  
Why: fast startup, easy MIDI routing, and reliable live reload workflow for VST experiments.

## Suggested branch model for your upcoming experiments

- `main`: stable baseline that always builds.
- `exp/midi-arp-*`: experimental MIDI arranger ideas.
- `fx/*`: effect-focused plugin branches.

Keep each experiment branch small and merge only after host test + `ctest` pass.
