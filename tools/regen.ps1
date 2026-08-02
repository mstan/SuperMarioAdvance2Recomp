param(
    [string]$Rom = (Join-Path $PSScriptRoot '..\roms\super_mario_world_usa.gba'),
    [string]$GbarecompRoot = (Join-Path $PSScriptRoot '..\gbarecomp'),
    [string]$Bios = '',
    [int]$MaxFunctions = 65536
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$romPath = (Resolve-Path $Rom).Path
$engine = (Resolve-Path $GbarecompRoot).Path
if (-not $Bios) { $Bios = Join-Path $engine 'bios\gba_bios.bin' }
$biosPath = (Resolve-Path $Bios).Path
$tool = Join-Path $engine 'build\gba_recompile.exe'
if (-not (Test-Path -LiteralPath $tool)) {
    throw "Missing $tool. Configure and build this game's gbarecomp worktree first."
}

$actual = (Get-FileHash -LiteralPath $romPath -Algorithm SHA1).Hash.ToLowerInvariant()
$expected = '5101ddf223d1d918928fe1f306b63a42ada14a5e'
if ($actual -ne $expected) {
    throw "Super Mario World ROM SHA-1 mismatch: got $actual expected $expected"
}

$biosActual = (Get-FileHash -LiteralPath $biosPath -Algorithm SHA1).Hash.ToLowerInvariant()
$biosExpected = '300c20df6731a33952ded8c436f7f186d25d3492'
if ($biosActual -ne $biosExpected) {
    throw "GBA BIOS SHA-1 mismatch: got $biosActual expected $biosExpected"
}

& $tool --bios $biosPath `
    --out (Join-Path $engine 'src\runtime\generated_bios')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $tool --rom $romPath --config (Join-Path $root 'game.toml') `
    --out (Join-Path $root 'generated') --max-functions $MaxFunctions
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
