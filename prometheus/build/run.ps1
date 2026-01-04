# Run script for Prometheus

$SettingsPath = "$PSScriptRoot\settings.json"
$Settings = Get-Content -Path $SettingsPath | ConvertFrom-Json

$OverwatchDir = $Settings.OverwatchDir
$ExeName = $Settings.ExeName

if ([string]::IsNullOrWhiteSpace($OverwatchDir)) {
    Write-Host "Overwatch directory not set." -ForegroundColor Red
    Write-Host "Run build.ps1 before running Prometheus." -ForegroundColor Yellow
    exit 1
}

$ExePath = Join-Path $OverwatchDir $ExeName

if (-not (Test-Path $ExePath)) {
    Write-Host "Executable not found: $ExePath" -ForegroundColor Red
    Write-Host "Run build.ps1 before running Prometheus." -ForegroundColor Yellow
    exit 1
}

Write-Host "Launching Overwatch..." -ForegroundColor Cyan
Start-Process -FilePath $ExePath -WorkingDirectory $OverwatchDir

Write-Host "Done!" -ForegroundColor Green
