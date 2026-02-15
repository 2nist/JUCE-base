# Cross-Platform Workflow

This repo now supports Windows, macOS, and Linux workflows from the same VS Code project.

## Windows (existing flow)

```powershell
.\scripts\setup-windows.ps1
.\scripts\configure.ps1
.\scripts\build.ps1 -Config Debug
.\scripts\test.ps1
```

## macOS/Linux (new flow)

```bash
bash scripts/setup-unix.sh
bash scripts/configure.sh Ninja build_unix Debug
bash scripts/build.sh Debug build_unix
bash scripts/test.sh build_unix Debug
```

## Run apps

Windows:

```powershell
.\scripts\run-designer.ps1
.\scripts\run-projucer.ps1 -Config Debug
```

macOS/Linux:

```bash
bash scripts/run-designer.sh build_unix
bash scripts/run-projucer.sh Debug build_projucer
```

## VS Code tasks

`Terminal -> Run Task` uses OS-specific command routing for:

- `Setup Toolchain`
- `Configure`
- `Build Debug`
- `Test`
- `Run Designer`
- `Configure Projucer`
- `Build Projucer`
- `Run Projucer`
