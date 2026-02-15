# Theme Pack System (External Graphics Workflow)

This repo now includes a theme pack layout so you can design visuals in external tools
(Photoshop, Illustrator, Affinity, Figma export, Krita, Inkscape, etc.) and plug them into your JUCE/Foleys workflow.

## Folder structure

- `resources/ui/themes/base/`:
  - Starter editable template assets.
- `resources/ui/themes/<your-theme>/`:
  - Your custom theme pack cloned from base.
- `resources/ui/themes/active-theme.json`:
  - Records currently selected theme metadata.

## Theme manifest

Each theme has:

- `manifest.json`

It maps semantic slots (background, knob skin, menu strip, button states) to actual files.
You can change assets to `.png`, `.jpg`, or `.svg`.

## Quick start

1. Create a new theme:
```powershell
.\scripts\new-theme.ps1 -ThemeId neon_city
```
2. Mark it active:
```powershell
.\scripts\set-active-theme.ps1 -ThemeId neon_city
```
3. Edit files in:
- `resources/ui/themes/neon_city/`
4. Use Designer app:
```powershell
.\scripts\run-designer.ps1
```
5. In Designer, load theme graphics as component backgrounds/images, then:
- `Save XML` or `Export For Plugin`

## Recommended graphics pipeline

- Keep a high-res source file in your own design app format (`.psd`, `.afdesign`, `.fig`).
- Export production assets into the theme folder:
  - backgrounds: 1920x1080+ preferred
  - controls: power-of-two sizes (128/256/512) for reuse
- Use consistent naming so replacing skins does not break layout references.

## Notes

- The plugin loads GUI layout from `resources/designer_layout.xml` when present.
- Theme packs are source assets; layout XML references whichever files you assign in Designer.

