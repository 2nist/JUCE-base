param(
    [switch]$InstallBuildTools
)

$ErrorActionPreference = "Stop"

function Test-Tool {
    param([string]$Name)
    return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

Write-Host "Checking local JUCE VST3 toolchain requirements..."

$hasGit = Test-Tool "git"
$hasCmake = Test-Tool "cmake"
$hasWinget = Test-Tool "winget"
$hasCl = Test-Tool "cl"
$hasMsbuild = Test-Tool "msbuild"
$vsDevCmd = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
$hasVsDevCmd = Test-Path $vsDevCmd

Write-Host ("git:      " + ($(if ($hasGit) { "OK" } else { "MISSING" })))
Write-Host ("cmake:    " + ($(if ($hasCmake) { "OK" } else { "MISSING" })))
Write-Host ("winget:   " + ($(if ($hasWinget) { "OK" } else { "MISSING" })))
Write-Host ("cl:       " + ($(if ($hasCl) { "OK" } else { "MISSING" })))
Write-Host ("msbuild:  " + ($(if ($hasMsbuild) { "OK" } else { "MISSING" })))
Write-Host ("VsDevCmd: " + ($(if ($hasVsDevCmd) { "OK" } else { "MISSING" })))

if (-not $hasGit -or -not $hasCmake) {
    throw "Install missing base tools (git, cmake) before continuing."
}

if ((-not $hasCl -or -not $hasMsbuild) -and $hasVsDevCmd -and -not $InstallBuildTools) {
    Write-Warning "cl/msbuild are not in PATH, but VsDevCmd is available. Build scripts will load it automatically."
    exit 0
}

if ((-not $hasCl -or -not $hasMsbuild) -and -not $hasVsDevCmd -and -not $InstallBuildTools) {
    Write-Warning "C++ build tools are not visible in this shell."
    Write-Host "Run this script with -InstallBuildTools to install Visual Studio Build Tools."
    exit 0
}

if ((-not $hasCl -or -not $hasMsbuild) -and -not $hasVsDevCmd -and $InstallBuildTools) {
    if (-not $hasWinget) {
        throw "winget is required for automated Build Tools install."
    }

    Write-Host "Installing Visual Studio 2022 Build Tools (C++ workload)..."
    winget install --id Microsoft.VisualStudio.2022.BuildTools `
        --exact `
        --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
    if ($LASTEXITCODE -ne 0) {
        throw "Build Tools install failed. Re-run install or install manually from Visual Studio Installer."
    }
    Write-Host "Install complete. Reopen terminal and re-run setup."
}

Write-Host "Toolchain check completed."
