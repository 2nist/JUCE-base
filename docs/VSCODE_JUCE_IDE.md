# VS Code JUCE IDE Setup

This repo includes an IDE starter kit for JUCE + CMake development.

## 1) Install recommended extensions

Open workspace and install recommendations from:

- `.vscode/extensions.json`
- `2nist-dev.code-workspace`

Core extensions:

- `ms-vscode.cmake-tools`
- `ms-vscode.cpptools`
- `llvm-vs-code-extensions.vscode-clangd`
- `ms-vscode.powershell`

## 2) Build tools shell

Before manual command-line builds:

```powershell
.\scripts\use-build-tools-path.ps1
```

This loads `cl`, `nmake`, and `msbuild` in current shell.

On macOS/Linux:

```bash
bash scripts/setup-unix.sh
```

## 3) Daily tasks

Use `Terminal -> Run Task` in VS Code:

- `Setup Toolchain`
- `Configure`
- `Build Debug`
- `Test`
- `Run Designer`
- `Build Projucer`
- `Run Projucer`

## 4) Debug launch configs

Provided in `.vscode/launch.json`:

- `Run TwoNist Designer (Debug)`
- `Run 2nist Standalone (Debug)`
- `Run Projucer (Debug)`

## 5) Projucer integration (safe mode)

Projucer is optional and kept separate from core build:

- `build_projucer` is used for Projucer.
- Main plugin/dev flow stays in `build_nmake`.
- Keep your plugin `CMakeLists.txt` as source-of-truth.
