param(
    [string]$BuildDir = "build_nmake"
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

Invoke-InVsDevShell "ctest --test-dir ""$BuildDir"" --output-on-failure"
