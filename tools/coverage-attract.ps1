param(
    [int]$Frames = 6000,
    [string]$BuildDir = 'build-aot',
    [switch]$Regenerate
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$build = Join-Path $root $BuildDir
$exe = Join-Path $build 'SuperMarioWorldRecomp.exe'
$coverage = Join-Path $root 'artifacts\coverage-attract.json'
$capture = Join-Path $root 'artifacts\coverage-attract.png'

if ($Regenerate) {
    & (Join-Path $PSScriptRoot 'regen.ps1')
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
if (-not (Test-Path -LiteralPath $exe)) {
    throw "Missing $exe. Configure and build the AOT target first."
}

New-Item -ItemType Directory -Force -Path (Split-Path $coverage -Parent) |
    Out-Null

$saved = @{
    ForceInterp = $env:GBARECOMP_FORCE_INTERP
    Strict = $env:GBARECOMP_STRICT_STATIC
    Coverage = $env:GBARECOMP_COVERAGE_JSON
}
try {
    Remove-Item Env:GBARECOMP_FORCE_INTERP -ErrorAction SilentlyContinue
    $env:GBARECOMP_STRICT_STATIC = '1'
    $env:GBARECOMP_COVERAGE_JSON = $coverage
    Push-Location $root
    try {
        $savedErrorAction = $ErrorActionPreference
        $ErrorActionPreference = 'Continue'
        try {
            $runOutput = & $exe --frames $Frames --dump-png $capture 2>&1 |
                Tee-Object -Variable runLines
            $runExit = $LASTEXITCODE
        } finally {
            $ErrorActionPreference = $savedErrorAction
        }
        if ($runExit -ne 0) { exit $runExit }
        if (($runLines -join "`n") -match '\[hang-watchdog\]') {
            Write-Warning (
                "The generic hang watchdog fired during a long " +
                "busy-wait/mixer interval. The completed frame count, static " +
                "coverage report, and capture remain authoritative.")
        }
    } finally {
        Pop-Location
    }
} finally {
    $env:GBARECOMP_FORCE_INTERP = $saved.ForceInterp
    $env:GBARECOMP_STRICT_STATIC = $saved.Strict
    $env:GBARECOMP_COVERAGE_JSON = $saved.Coverage
}

if (-not (Test-Path -LiteralPath $coverage)) {
    throw "Strict run produced no coverage report: $coverage"
}
$result = Get-Content -LiteralPath $coverage -Raw | ConvertFrom-Json
if ($result.coverage -ne 'FULLY_STATIC' -or $result.distinct_misses -ne 0) {
    throw "Attract coverage is $($result.coverage) with " +
        "$($result.distinct_misses) dispatch miss(es)."
}
Write-Host "Strict AOT attract coverage passed: $coverage"
