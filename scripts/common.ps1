$ErrorActionPreference = "Stop"

function Get-VsDevCmdPath {
    $path = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $path)) {
        throw "VsDevCmd.bat was not found. Install Visual Studio 2022 Build Tools with C++ workload."
    }
    return $path
}

function Invoke-InVsDevShell {
    param(
        [Parameter(Mandatory = $true)][string]$Command
    )

    $vsDevCmd = Get-VsDevCmdPath
    cmd.exe /c """$vsDevCmd"" -arch=x64 && $Command"
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed in VS developer shell: $Command"
    }
}

