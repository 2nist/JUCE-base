param(
    [switch]$NewWindow
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$workspace = Join-Path $repoRoot "2nist-dev.code-workspace"

$files = @(
    "src/PluginEditor.h",
    "src/PluginEditor.cpp",
    "src/PluginProcessor.h",
    "src/PluginProcessor.cpp",
    "apps/designer/MainComponent.h",
    "apps/designer/MainComponent.cpp"
) | ForEach-Object { Join-Path $repoRoot $_ }

$code = Get-Command code -ErrorAction SilentlyContinue
if (-not $code) {
    Write-Host "VS Code CLI 'code' was not found in PATH."
    Write-Host "In VS Code press Ctrl+Shift+P and run: Shell Command: Install 'code' command in PATH"
    Write-Host "Then run:"
    Write-Host "  code `"$workspace`""
    Write-Host "  code $($files -join ' ')"
    exit 1
}

$args = @()
if ($NewWindow) {
    $args += "--new-window"
}
else {
    $args += "--reuse-window"
}

$args += $workspace
$args += $files

& $code.Source @args
