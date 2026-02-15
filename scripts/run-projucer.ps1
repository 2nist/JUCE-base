param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build_projucer",
    [switch]$NoBuild
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

if (-not $NoBuild) {
    & "$PSScriptRoot\build-projucer.ps1" -Config $Config -BuildDir $BuildDir
}

$searchRoots = @(
    (Join-Path (Get-Location) $BuildDir),
    (Join-Path (Get-Location) "$BuildDir\_deps\juce-build\extras\Projucer")
)

$projucerExe = $null
foreach ($root in $searchRoots) {
    if (-not (Test-Path $root)) {
        continue
    }

    $candidate = Get-ChildItem -Path $root -Recurse -Filter "Projucer*.exe" -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    if ($candidate) {
        $projucerExe = $candidate.FullName
        break
    }
}

if (-not $projucerExe) {
    throw "Projucer executable not found. Try: .\scripts\build-projucer.ps1 -Config $Config"
}

Write-Host "Launching: $projucerExe"
& $projucerExe
