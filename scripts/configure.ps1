param(
    [string]$Generator = "NMake Makefiles",
    [string]$BuildDir = "build_nmake"
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

Invoke-InVsDevShell "cmake -S . -B ""$BuildDir"" -G ""$Generator"""
