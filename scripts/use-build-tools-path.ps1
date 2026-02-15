param(
    [ValidateSet("x64", "x86", "arm64")][string]$Arch = "x64"
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

Import-VsDevShellEnvironment -Arch $Arch

$tools = @("cl", "nmake", "msbuild")
foreach ($tool in $tools) {
    $cmd = Get-Command $tool -ErrorAction SilentlyContinue
    if ($cmd) {
        Write-Host ("{0}: {1}" -f $tool, $cmd.Source)
    }
    else {
        Write-Warning ("{0}: not found in current PATH after loading VsDevCmd." -f $tool)
    }
}

Write-Host "Build tools environment loaded into this shell process."
