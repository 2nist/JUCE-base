param(
    [Parameter(Mandatory = $true)][string]$ThemeId
)

$ErrorActionPreference = "Stop"

$baseDir = Join-Path (Get-Location) "resources\ui\themes"
$source = Join-Path $baseDir "base"
$target = Join-Path $baseDir $ThemeId

if (-not (Test-Path $source)) {
    throw "Base theme folder not found at $source"
}

if (Test-Path $target) {
    throw "Theme '$ThemeId' already exists: $target"
}

Copy-Item -Path $source -Destination $target -Recurse

$manifestPath = Join-Path $target "manifest.json"
$manifest = Get-Content $manifestPath | ConvertFrom-Json
$manifest.themeId = $ThemeId
$manifest.name = "$ThemeId Theme"
$manifest.description = "Theme cloned from base. Edit assets in external graphics tools."
$manifest.author = "TwoNistLabs"
$manifest.version = "1.0.0"
$manifest | ConvertTo-Json -Depth 10 | Set-Content $manifestPath

Write-Host "Created theme: $ThemeId"
Write-Host "Path: $target"

