param(
    [string]$Generator = "NMake Makefiles",
    [string]$BuildDir = "build_projucer"
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

Invoke-InVsDevShell "cmake -S . -B ""$BuildDir"" -G ""$Generator"" -DBUILD_PROJUCER_TOOL=ON"
