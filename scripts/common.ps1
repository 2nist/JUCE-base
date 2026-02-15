$ErrorActionPreference = "Stop"

function Get-VsDevCmdPath {
    $candidates = New-Object System.Collections.Generic.List[string]

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($installPath)) {
            $candidates.Add((Join-Path $installPath "Common7\Tools\VsDevCmd.bat"))
        }
    }

    $candidates.Add("${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat")
    $candidates.Add("${env:ProgramFiles}\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat")
    $candidates.Add("${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat")
    $candidates.Add("${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat")
    $candidates.Add("${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat")
    $candidates.Add("${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat")
    $candidates.Add("${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat")
    $candidates.Add("${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat")

    foreach ($path in $candidates) {
        if (-not [string]::IsNullOrWhiteSpace($path) -and (Test-Path $path)) {
            return $path
        }
    }

    throw "VsDevCmd.bat was not found. Install Visual Studio 2022 Build Tools with C++ workload."
}

function Import-VsDevShellEnvironment {
    param(
        [ValidateSet("x64", "x86", "arm64")][string]$Arch = "x64"
    )

    $vsDevCmd = Get-VsDevCmdPath
    $lines = cmd.exe /c """$vsDevCmd"" -no_logo -arch=$Arch >nul && set"
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to import Visual Studio developer shell environment."
    }

    foreach ($line in $lines) {
        if ($line -match "^(.*?)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
        }
    }
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
