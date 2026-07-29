param(
    [int]$Frames = 6000,
    [string]$Output = (Join-Path $PSScriptRoot '..\artifacts\attract-complete.png')
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$exe = Join-Path $root 'build\SuperMarioWorldRecomp.exe'
if (-not (Test-Path -LiteralPath $exe)) {
    throw "Missing $exe. Generate and build the project first."
}

$outputPath = [System.IO.Path]::GetFullPath($Output)
$outputDir = Split-Path -Parent $outputPath
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

# The whole-program interpreter is the correctness oracle shipped by
# GBARecomp. It also provides a deterministic smoke test while native static
# coverage for this title is still being expanded.
$previousForceInterp = $env:GBARECOMP_FORCE_INTERP
try {
    $env:GBARECOMP_FORCE_INTERP = '1'
    Push-Location $root
    try {
        & $exe --frames $Frames --dump-png $outputPath
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    } finally {
        Pop-Location
    }
} finally {
    $env:GBARECOMP_FORCE_INTERP = $previousForceInterp
}

if (-not (Test-Path -LiteralPath $outputPath)) {
    throw "The run completed without producing $outputPath"
}
Write-Host "Attract-demo smoke passed ($Frames frames): $outputPath"
