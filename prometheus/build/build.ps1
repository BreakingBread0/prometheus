# Build script for Prometheus

$SettingsPath = "$PSScriptRoot\settings.json"
$DefaultSettingsPath = "$PSScriptRoot\settings_default.json"

if (-not (Test-Path $SettingsPath)) {
    if (Test-Path $DefaultSettingsPath) {
        Write-Host "settings.json not found. Initializing from default_settings.json..." -ForegroundColor Yellow
        Copy-Item -Path $DefaultSettingsPath -Destination $SettingsPath -Force
    } else {
        Write-Host "Error: Neither settings.json nor default_settings.json was found!" -ForegroundColor Red
        exit 1
    }
}

$Settings = Get-Content -Path $SettingsPath | ConvertFrom-Json

$OverwatchDir = $Settings.OverwatchDir
$SolutionName = $Settings.SolutionName
$ExeName = $Settings.ExeName

# Get the solution directory (two levels up from build folder)
$SolutionDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$SolutionPath = Join-Path $SolutionDir $SolutionName

# Patch-related paths
$PatchedExePath = if (-not [string]::IsNullOrWhiteSpace($OverwatchDir)) {
    Join-Path $OverwatchDir $ExeName
} else {
    $null
}

$PatcherDir = Join-Path $SolutionDir "patcher"
$PatcherProject = Join-Path $PatcherDir "patcher.csproj"
$PatcherBuildDir = Join-Path $PatcherDir "bin\x64\Release\net8.0-windows"
$PatcherDll = Join-Path $PatcherBuildDir "patcher.dll"

$NeedsPatcher =
    [string]::IsNullOrWhiteSpace($OverwatchDir) -or
    -not (Test-Path $PatchedExePath)

if ($NeedsPatcher) {
    Write-Host "Running patcher..." -ForegroundColor Yellow

    & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
        $PatcherProject `
        /p:Configuration=Release `
        /p:Platform=x64 `
        /m

    if ($LASTEXITCODE -ne 0) {
        Write-Host "Patcher build failed!" -ForegroundColor Red
        exit 1
    }

    if (-not (Test-Path $PatcherDll)) {
        Write-Host "Patcher DLL not found after build!" -ForegroundColor Red
        exit 1
    }

    Write-Host "Launching patcher..." -ForegroundColor Cyan

    # Capture stdout
    $patcherOutput = & dotnet "$PatcherDll"
    $exitCode = $LASTEXITCODE

    if ($exitCode -ne 0) {
        Write-Host "Patcher failed or was cancelled." -ForegroundColor Red
        exit 1
    }

    $SelectedDir = $patcherOutput | Select-Object -Last 1

    if ([string]::IsNullOrWhiteSpace($SelectedDir) -or -not (Test-Path $SelectedDir)) {
        Write-Host "Invalid directory returned from patcher." -ForegroundColor Red
        exit 1
    }

    Write-Host "Detected Overwatch directory: $SelectedDir" -ForegroundColor Green

    # Write back to settings.json if OverwatchDir was empty
    $Settings.OverwatchDir = $SelectedDir
    $Settings | ConvertTo-Json -Depth 5 | Set-Content $SettingsPath
    $OverwatchDir = $SelectedDir
    Write-Host "Updated settings.json" -ForegroundColor Cyan
}
else {
    Write-Host "Patched exe already exists. Skipping patcher." -ForegroundColor Gray
}

# Build the solution
Write-Host "Building Prometheus..." -ForegroundColor Cyan
Write-Host "Solution: $SolutionPath" -ForegroundColor Gray

& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
    $SolutionPath `
    /p:Configuration=Release `
    /p:Platform=x64 `
    /m

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Build succeeded!" -ForegroundColor Green

# Paths relative to solution directory
$ReleaseDir = Join-Path $SolutionDir "x64\Release"
$dllPath = Join-Path $ReleaseDir "prometheus.dll"
$injectPath = Join-Path $ReleaseDir "inject.dll"

# Rename prometheus.dll -> inject.dll
if (Test-Path $dllPath) {
    Remove-Item $injectPath -Force -ErrorAction SilentlyContinue
    Rename-Item $dllPath "inject.dll" -Force
    Write-Host "Renamed prometheus.dll to inject.dll" -ForegroundColor Green
}

# Copy build output to Overwatch directory
Write-Host "Copying files to Overwatch directory..." -ForegroundColor Cyan
Copy-Item -Path "$ReleaseDir\*" -Destination $OverwatchDir -Force

Write-Host "Build complete!" -ForegroundColor Green
