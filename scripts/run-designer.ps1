$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

$exe = Join-Path (Get-Location) "build_nmake\DesignerApp_artefacts\Debug\TwoNist Designer.exe"
if (-not (Test-Path $exe)) {
    throw "Designer executable not found. Build first with .\scripts\build.ps1"
}

& $exe
