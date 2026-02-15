# GUI-First VST3 Workflow (JUCE + foleys_gui_magic)

## Goal

Design plugin UI visually (WYSIWYG) and keep shipping a standard VST3 built by JUCE/CMake.
This repo now also includes a standalone designer app for this purpose.

## What is already wired

- Module source is vendored at `vendor/foleys_gui_magic`.
- CMake links `foleys_gui_magic` into target `2nist`.
- CMake also builds `DesignerApp` (`TwoNist Designer.exe`).
- Debug compile flags currently enable runtime editing palette:
  - `FOLEYS_SHOW_GUI_EDITOR_PALLETTE=1`
  - `FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE=1`

## Day-to-day loop

1. Build:
```powershell
.\scripts\build.ps1
```
2. Launch standalone designer:
```powershell
.\scripts\run-designer.ps1
```
3. Use top controls:
   - `New Layout`
   - `Load XML`
   - `Save XML`
   - `Export For Plugin`
   - `Vector Panel ON/OFF`
   - `Load URL` / `Load Local HTML` for embedded vector editor panel
   - Preset buttons: `SVG-Edit`, `miniPaint`, `Method Draw`
4. Use the Foleys palette:
   - add sliders/buttons/labels/layout containers
   - connect controls to parameter IDs or properties/triggers
5. Export writes:
   - `resources/designer_layout.xml`
   - Plugin `2nist` reads this file at editor creation (fallback is Foleys default GUI).
6. Load plugin in REAPER for runtime audio testing and repeat.

## Parameter IDs available for binding

- `root`
- `pattern`
- `repeats`
- `channel`
- `gate`
- `velocity`

## Release practice

Before release builds, disable palette in `CMakeLists.txt`:

- set `FOLEYS_SHOW_GUI_EDITOR_PALLETTE=0`

Keep save-in-state enabled only if you want users to edit UI layout in deployed builds.
