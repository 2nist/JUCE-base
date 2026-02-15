# JUCE CMake Templates

Use the generator script:

```powershell
.\scripts\new-juce-plugin-template.ps1 -ProjectName "My JUCE Plugin" -Mode fetchcontent
```

Options:

- `-Mode fetchcontent`
- `-Mode cpm`
- `-Mode submodule`
- `-DestinationRoot experiments`
- `-CompanyName`
- `-BundlePrefix`

## Generated project contents

- `CMakeLists.txt`
- `src/PluginProcessor.h/.cpp`
- `src/PluginEditor.h/.cpp`
- `README.md`

Mode-specific:

- `cpm`: adds `cmake/CPM.cmake`
- `submodule`: adds `SUBMODULE_SETUP.txt`

## Dependency mode guide

1. `fetchcontent`
- Best default.
- Minimal setup.
- Good for fast experiments.

2. `cpm`
- Cleaner dependency declarations.
- Useful when managing many dependencies.

3. `submodule`
- Reproducible JUCE revision under version control.
- Best for locked-down production repos.

## Example commands

```powershell
.\scripts\new-juce-plugin-template.ps1 -ProjectName "ChordLab" -Mode fetchcontent
.\scripts\new-juce-plugin-template.ps1 -ProjectName "FXLab" -Mode cpm
.\scripts\new-juce-plugin-template.ps1 -ProjectName "SynthLab" -Mode submodule
```
