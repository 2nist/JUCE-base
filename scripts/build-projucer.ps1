param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build_projucer",
    [string]$Generator = "NMake Makefiles"
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

Invoke-InVsDevShell "cmake -S . -B ""$BuildDir"" -G ""$Generator"" -DBUILD_PROJUCER_TOOL=ON"
Invoke-InVsDevShell "cmake --build ""$BuildDir"" --config ""$Config"" --target Projucer"
