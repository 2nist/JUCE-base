param(
    [Parameter(Mandatory = $true)][string]$ThemeId
)

$ErrorActionPreference = "Stop"

$themesRoot = Join-Path (Get-Location) "resources\ui\themes"
$themeDir = Join-Path $themesRoot $ThemeId
$manifestPath = Join-Path $themeDir "manifest.json"

if (-not (Test-Path $manifestPath)) {
    throw "Theme manifest not found: $manifestPath"
}

$active = [ordered]@{
    activeThemeId = $ThemeId
    activeThemePath = "resources/ui/themes/$ThemeId/manifest.json"
    updatedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
}

$activePath = Join-Path $themesRoot "active-theme.json"
$active | ConvertTo-Json -Depth 5 | Set-Content $activePath

Write-Host "Active theme set to: $ThemeId"
Write-Host "Metadata: $activePath"

