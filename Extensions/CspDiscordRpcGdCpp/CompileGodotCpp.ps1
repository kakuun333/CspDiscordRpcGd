param(
    [ValidateSet("windows", "macos", "linux")]
    [string]$Platform,

    [int]$Jobs = [Environment]::ProcessorCount,

    [switch]$SkipClean,

    [switch]$DebugSymbolsForRelease
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Select-GodotCppPlatform {
    Write-Host ""
    Write-Host "Select platform to build godot-cpp:"
    Write-Host ""
    Write-Host "  1) windows"
    Write-Host "  2) macos"
    Write-Host "  3) linux"
    Write-Host ""

    while ($true) {
        $Selection = Read-Host "Enter choice [1-3]"

        switch ($Selection.Trim()) {
            "1" { return "windows" }
            "windows" { return "windows" }
            "win" { return "windows" }

            "2" { return "macos" }
            "macos" { return "macos" }
            "mac" { return "macos" }

            "3" { return "linux" }
            "linux" { return "linux" }

            default {
                Write-Host "Invalid choice. Please enter 1, 2, 3, windows, macos, or linux."
            }
        }
    }
}

function Invoke-GodotCppBuild {
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("windows", "macos", "linux")]
        [string]$Platform,

        [Parameter(Mandatory = $true)]
        [string]$Target,

        [Parameter(Mandatory = $true)]
        [bool]$DevBuild,

        [Parameter(Mandatory = $true)]
        [bool]$DebugSymbols,

        [Parameter(Mandatory = $true)]
        [int]$Jobs
    )

    $SconsArgs = @(
        "platform=$Platform",
        "target=$Target",
        "dev_build=$(if ($DevBuild) { 'yes' } else { 'no' })",
        "use_static_cpp=yes",
        "debug_symbols=$(if ($DebugSymbols) { 'yes' } else { 'no' })",
        "-j$Jobs"
    )

    Write-Host ""
    Write-Host "Starting godot-cpp build: platform=$Platform, target=$Target"
    & scons @SconsArgs

    if ($LASTEXITCODE -ne 0) {
        throw "SCons build failed for platform '$Platform', target '$Target' with exit code $LASTEXITCODE."
    }
}

if ([string]::IsNullOrWhiteSpace($Platform)) {
    $Platform = Select-GodotCppPlatform
}

$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$GodotCppDir = Join-Path (Join-Path $RepoRoot "Dependencies") "godot-cpp"

if (-not (Test-Path -LiteralPath $GodotCppDir -PathType Container)) {
    throw "Cannot find directory: $GodotCppDir"
}

Push-Location $GodotCppDir
try {
    if (-not $SkipClean) {
        Write-Host "Cleaning old build artifacts..."

        foreach ($Path in @("bin", "gen")) {
            if (Test-Path -LiteralPath $Path) {
                Remove-Item -LiteralPath $Path -Recurse -Force
            }
        }

        if (Test-Path -LiteralPath ".sconsign.dblite") {
            Remove-Item -LiteralPath ".sconsign.dblite" -Force
        }
    }

    Invoke-GodotCppBuild -Platform $Platform -Target "template_debug" -DevBuild $true -DebugSymbols $true -Jobs $Jobs
    Invoke-GodotCppBuild -Platform $Platform -Target "template_release" -DevBuild $false -DebugSymbols $DebugSymbolsForRelease.IsPresent -Jobs $Jobs

    Write-Host ""
    Write-Host "========================================"
    Write-Host "godot-cpp build completed successfully."
    Write-Host "Platform: $Platform"
    Write-Host "========================================"
}
finally {
    Pop-Location
}

Write-Host ""
Read-Host "Build finished. Press Enter to exit"
