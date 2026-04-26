param(
    [int]$Jobs = $(if ($env:NUMBER_OF_PROCESSORS) { [int]$env:NUMBER_OF_PROCESSORS } else { 1 }),
    [switch]$SkipClean,
    [switch]$DebugSymbolsForRelease
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Invoke-GodotCppBuild {
    param(
        [string]$Target,
        [bool]$DevBuild,
        [bool]$DebugSymbols,
        [int]$Jobs
    )

    $args = @(
        "platform=windows",
        "target=$Target",
        "dev_build=$(if ($DevBuild) { 'yes' } else { 'no' })",
        "use_static_cpp=yes",
        "debug_symbols=$(if ($DebugSymbols) { 'yes' } else { 'no' })",
        "-j$Jobs"
    )

    Write-Host "Starting godot-cpp build: $Target"
    & scons @args
    if ($LASTEXITCODE -ne 0) {
        throw "SCons build failed for target '$Target' with exit code $LASTEXITCODE."
    }
}

$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$GodotCppDir = Join-Path $RepoRoot "Dependencies\godot-cpp"

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

    Invoke-GodotCppBuild -Target "template_debug" -DevBuild $true -DebugSymbols $true -Jobs $Jobs
    Invoke-GodotCppBuild -Target "template_release" -DevBuild $false -DebugSymbols $DebugSymbolsForRelease.IsPresent -Jobs $Jobs

    Write-Host ""
    Write-Host "========================================"
    Write-Host "godot-cpp build completed successfully."
    Write-Host "========================================"
}
finally {
    Pop-Location
}

Write-Host ""
Read-Host "Build finished. Press Enter to exit"