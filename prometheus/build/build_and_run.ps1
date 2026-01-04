# Build and Run script for Prometheus

& "$PSScriptRoot\build.ps1"

if ($LASTEXITCODE -ne 0) {
    exit 1
}

& "$PSScriptRoot\run.ps1"