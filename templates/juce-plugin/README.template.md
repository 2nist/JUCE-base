# __PRODUCT_NAME__ (Template)

Generated JUCE plugin template using mode: `__DEPENDENCY_MODE__`.

## Build

```powershell
cmake -S . -B build -G "NMake Makefiles"
cmake --build build --config Debug
```

## Notes

- This template is intentionally minimal and CMake-first.
- Start by editing `CMakeLists.txt` and files in `src/`.
- For host testing use REAPER and load the built `.vst3`.
