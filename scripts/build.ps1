param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build_nmake"
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

Invoke-InVsDevShell "cmake --build ""$BuildDir"" --config ""$Config"""
