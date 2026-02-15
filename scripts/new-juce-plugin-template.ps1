param(
    [Parameter(Mandatory = $true)][string]$ProjectName,
    [ValidateSet("fetchcontent", "cpm", "submodule")][string]$Mode = "fetchcontent",
    [string]$DestinationRoot = "experiments",
    [string]$CompanyName = "YourCompany",
    [string]$BundlePrefix = "com.example"
)

$ErrorActionPreference = "Stop"

function Replace-Tokens {
    param(
        [Parameter(Mandatory = $true)][string]$InputPath,
        [Parameter(Mandatory = $true)][string]$OutputPath,
        [Parameter(Mandatory = $true)][hashtable]$Tokens
    )

    $content = Get-Content -Path $InputPath -Raw
    foreach ($key in $Tokens.Keys) {
        $content = $content.Replace($key, $Tokens[$key])
    }
    $outputDir = Split-Path -Parent $OutputPath
    if (-not (Test-Path $outputDir)) {
        New-Item -ItemType Directory -Path $outputDir | Out-Null
    }
    Set-Content -Path $OutputPath -Value $content -NoNewline
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$templateRoot = Join-Path $repoRoot "templates\juce-plugin"
$targetName = ($ProjectName -replace "[^A-Za-z0-9_]", "_")
$className = ($targetName -replace "_", "")
$bundleId = "$BundlePrefix.$($targetName.ToLower())"
$destDir = Join-Path $repoRoot (Join-Path $DestinationRoot $targetName)

if (Test-Path $destDir) {
    throw "Target directory already exists: $destDir"
}

New-Item -ItemType Directory -Path $destDir | Out-Null

$tokens = @{
    "__TARGET_NAME__" = $targetName
    "__PRODUCT_NAME__" = $ProjectName
    "__CLASS_NAME__" = $className
    "__COMPANY_NAME__" = $CompanyName
    "__BUNDLE_ID__" = $bundleId
    "__DEPENDENCY_MODE__" = $Mode
}

Replace-Tokens -InputPath (Join-Path $templateRoot "CMakeLists.$Mode.template") `
    -OutputPath (Join-Path $destDir "CMakeLists.txt") `
    -Tokens $tokens

Replace-Tokens -InputPath (Join-Path $templateRoot "src\PluginProcessor.h.template") `
    -OutputPath (Join-Path $destDir "src\PluginProcessor.h") `
    -Tokens $tokens
Replace-Tokens -InputPath (Join-Path $templateRoot "src\PluginProcessor.cpp.template") `
    -OutputPath (Join-Path $destDir "src\PluginProcessor.cpp") `
    -Tokens $tokens
Replace-Tokens -InputPath (Join-Path $templateRoot "src\PluginEditor.h.template") `
    -OutputPath (Join-Path $destDir "src\PluginEditor.h") `
    -Tokens $tokens
Replace-Tokens -InputPath (Join-Path $templateRoot "src\PluginEditor.cpp.template") `
    -OutputPath (Join-Path $destDir "src\PluginEditor.cpp") `
    -Tokens $tokens
Replace-Tokens -InputPath (Join-Path $templateRoot "README.template.md") `
    -OutputPath (Join-Path $destDir "README.md") `
    -Tokens $tokens

if ($Mode -eq "cpm") {
    Replace-Tokens -InputPath (Join-Path $templateRoot "cmake\CPM.cmake.template") `
        -OutputPath (Join-Path $destDir "cmake\CPM.cmake") `
        -Tokens $tokens
}

if ($Mode -eq "submodule") {
    $notesPath = Join-Path $destDir "SUBMODULE_SETUP.txt"
    Set-Content -Path $notesPath -Value @"
Run these commands from ${destDir}:

git init
git submodule add https://github.com/juce-framework/JUCE.git external/JUCE
git submodule update --init --recursive
"@
}

Write-Host "Created JUCE template project:"
Write-Host "  Path: $destDir"
Write-Host "  Mode: $Mode"
Write-Host "  Target: $targetName"
