param(
    [switch]$NewWindow
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$workspace = Join-Path $repoRoot "2nist-dev.code-workspace"

$wrapperFiles = @(
    (Join-Path $repoRoot "src/PluginEditor.h"),
    (Join-Path $repoRoot "src/PluginEditor.cpp")
)

$soundMidiFiles = @(
    (Join-Path $repoRoot "src/PluginProcessor.h"),
    (Join-Path $repoRoot "src/PluginProcessor.cpp")
)

$componentFiles = @(
    (Join-Path $repoRoot "apps/designer/MainComponent.h"),
    (Join-Path $repoRoot "apps/designer/MainComponent.cpp")
)

$code = Get-Command code -ErrorAction SilentlyContinue
if (-not $code) {
    Write-Host "VS Code CLI 'code' was not found in PATH."
    Write-Host "In VS Code press Ctrl+Shift+P and run: Shell Command: Install 'code' command in PATH"
    exit 1
}

$windowArg = if ($NewWindow) { "--new-window" } else { "--reuse-window" }

function Invoke-Code {
    param([string[]]$Args)
    & $code.Source @Args
}

function Invoke-CodeCommand {
    param([string]$CommandId)
    Invoke-Code @($windowArg, "--command", $CommandId)
}

# 1) Open workspace and Wrapper group (left group)
Invoke-Code @($windowArg, $workspace)
Start-Sleep -Milliseconds 350
$wrapperArgs = @($windowArg) + $wrapperFiles
Invoke-Code -Args $wrapperArgs
Start-Sleep -Milliseconds 350

# 2) Create group 2 and load Sound/MIDI files
Invoke-CodeCommand "workbench.action.newGroupRight"
Start-Sleep -Milliseconds 300
$soundMidiArgs = @($windowArg) + $soundMidiFiles
Invoke-Code -Args $soundMidiArgs
Start-Sleep -Milliseconds 300

# 3) Create group 3 and load Component files
Invoke-CodeCommand "workbench.action.newGroupRight"
Start-Sleep -Milliseconds 300
$componentArgs = @($windowArg) + $componentFiles
Invoke-Code -Args $componentArgs
Start-Sleep -Milliseconds 200

# 4) Normalize group widths and return focus to group 1
Invoke-CodeCommand "workbench.action.evenEditorWidths"
Invoke-CodeCommand "workbench.action.focusFirstEditorGroup"

Write-Host "Opened 2nist workflow layout:"
Write-Host "  Group 1 (Wrapper): src/PluginEditor.*"
Write-Host "  Group 2 (Sound/MIDI): src/PluginProcessor.*"
Write-Host "  Group 3 (Components): apps/designer/MainComponent.*"
