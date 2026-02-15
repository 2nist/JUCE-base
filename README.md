# JUCE VST Development Environment (`2nist`)

This repository is now a working baseline for JUCE VST3 development on Windows with CMake.

## What is included

- JUCE plugin project using `FetchContent` (no manual JUCE copy needed).
- A starter VST3 + Standalone MIDI effect plugin: `2nist MIDI Arranger`.
- GUI-first editor workflow via `foleys_gui_magic` (WYSIWYG).
- Separate standalone GUI Designer app target: `TwoNist Designer`.
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

## Cross-platform quick start (macOS/Linux)

```bash
bash scripts/setup-unix.sh
bash scripts/configure.sh Ninja build_unix Debug
bash scripts/build.sh Debug build_unix
bash scripts/test.sh build_unix Debug
```

Guide:

- `docs/CROSS_PLATFORM_WORKFLOW.md`

## CI (Phase 2)

GitHub Actions workflow at:

- `.github/workflows/ci.yml`

What it runs:

- Matrix build/test on `windows-latest`, `macos-latest`, `ubuntu-latest`
- Additional Windows Projucer tool build validation

## New: VS Code IDE enhancement kit

- IDE guide: `docs/VSCODE_JUCE_IDE.md`
- JUCE template guide: `docs/JUCE_CMAKE_TEMPLATES.md`
- Template generator:
```powershell
.\scripts\new-juce-plugin-template.ps1 -ProjectName "My JUCE Plugin" -Mode fetchcontent
```

Supported template modes:

- `fetchcontent`
- `cpm`
- `submodule`

## Run Designer App

Build, then launch:

```powershell
.\scripts\build.ps1
.\scripts\run-designer.ps1
```

Designer output path:

- `build_nmake\DesignerApp_artefacts\Debug\TwoNist Designer.exe`
- Includes embedded Vector Editor panel (URL/local HTML) for SVG workflow.
- Includes in-app `Diagnose` for vector tool loading checks (backend support, URL validity, last load error/success).
- Includes a command-based customizable toolbar (drag/reorder via `Customize`, persisted between launches).

## Optional: Projucer tool

This repo can optionally build JUCE Projucer without changing your main CMake-first workflow.

Configure/build/run:

```powershell
.\scripts\configure-projucer.ps1
.\scripts\build-projucer.ps1 -Config Debug
.\scripts\run-projucer.ps1 -Config Debug
```

Notes:

- Projucer is optional and controlled by `-DBUILD_PROJUCER_TOOL=ON`.
- Projucer scripts default to separate build dir: `build_projucer`.
- Keep `CMakeLists.txt` as source-of-truth for this repo.
- Use Projucer for reference/migration or separate `.jucer` projects to avoid config drift.

## Unified editing workflow (all 6 core files)

This repo now includes a VS Code workflow that opens all core edit targets at once:

- Wrapper: `src/PluginEditor.h`, `src/PluginEditor.cpp`
- Sounds/MIDI behavior: `src/PluginProcessor.h`, `src/PluginProcessor.cpp`
- Components/Designer app: `apps/designer/MainComponent.h`, `apps/designer/MainComponent.cpp`

Use:

```powershell
.\scripts\open-dev-workflow.ps1
```

For automated 3-column editor groups (Wrapper | Sound/MIDI | Components):

```powershell
.\scripts\open-dev-workflow-layout.ps1
```

If `code` is not in PATH, install it from VS Code command palette:

- `Shell Command: Install 'code' command in PATH`

Then rerun the script.

Included workspace/task files:

- `2nist-dev.code-workspace`
- `.vscode/tasks.json`

In VS Code, run `Terminal -> Run Task` for:

- `Configure`
- `Build Debug`
- `Test`
- `Run Designer`
- `Configure Projucer`
- `Build Projucer`
- `Run Projucer`
- `Open 2nist Editing Workflow (3-Group Layout)`

## Where the plugin lands

For this repo's default build (`build_nmake`), the VST3 output is:

- `build_nmake\2nist_artefacts\Debug\VST3\2nist MIDI Arranger.vst3`

Standalone app is under:

- `build_nmake\2nist_artefacts\Debug\Standalone\`

## External tool that adds major value

Use **REAPER** as your daily plugin host while developing.  
Why: fast startup, easy MIDI routing, and reliable live reload workflow for VST experiments.

## WYSIWYG GUI workflow (foleys_gui_magic)

This repo now includes `foleys_gui_magic` under `vendor/foleys_gui_magic` and links it into the plugin.

Debug builds are configured with:

- `FOLEYS_SHOW_GUI_EDITOR_PALLETTE=1`
- `FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE=1`

Workflow:

1. Build debug plugin:
```powershell
.\scripts\build.ps1
```
2. Load `2nist MIDI Arranger` in REAPER.
3. Open the plugin window and use the Foleys GUI editor palette to drag/drop controls.
4. Bind controls to parameters (`root`, `pattern`, `repeats`, `channel`, `gate`, `velocity`).
5. Save plugin preset/state in host to persist GUI tree while iterating.

For release builds you can disable the palette by changing compile definitions in `CMakeLists.txt`.

## Theme packs (external art workflow)

Use editable theme packs for custom visuals (SVG/PNG/JPG):

- Guide: `docs/THEME_PACKS.md`
- Create a theme:
```powershell
.\scripts\new-theme.ps1 -ThemeId my_theme
```
- Set active theme metadata:
```powershell
.\scripts\set-active-theme.ps1 -ThemeId my_theme
```

## Suggested branch model for your upcoming experiments

- `main`: stable baseline that always builds.
- `exp/midi-arp-*`: experimental MIDI arranger ideas.
- `fx/*`: effect-focused plugin branches.

Keep each experiment branch small and merge only after host test + `ctest` pass.
