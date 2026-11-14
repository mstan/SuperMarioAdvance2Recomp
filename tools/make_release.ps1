<#
Build the SuperMarioAdvance2Recomp Windows x64 release archive.

The archive contains the stripped executable, recomp-ui assets, built-in mod
catalog, MinGW/SDL runtime DLLs, and the overlay toolchain. It never contains
a ROM, retail GBA BIOS, save data, generated source, or developer config.

Usage:
  pwsh tools\make_release.ps1 -Version 0.0.1
#>
param(
    [Parameter(Mandatory = $true)][string]$Version,
    [string]$BuildDir = 'build-release',
    [string]$GbarecompRoot = '',
    [string]$RecompUiRoot = '',
    [string]$GeneratedBiosDir = '',
    [ValidateRange(1, 32)][int]$Jobs = 4
)

$ErrorActionPreference = 'Stop'

if ($Version -notmatch '^\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?$') {
    throw "Version must look like 0.0.1 (received '$Version')."
}

$mingwBin = 'C:\msys64\mingw64\bin'
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir '..')).Path
if (-not $GbarecompRoot) { $GbarecompRoot = Join-Path $root 'gbarecomp' }
$engine = (Resolve-Path $GbarecompRoot).Path
if (-not $RecompUiRoot) { $RecompUiRoot = Join-Path $root 'recomp-ui' }
$uiRoot = (Resolve-Path -LiteralPath $RecompUiRoot).Path
if (-not $GeneratedBiosDir) { $GeneratedBiosDir = Join-Path $engine 'src/runtime/generated_bios' }
$generatedBios = (Resolve-Path -LiteralPath $GeneratedBiosDir).Path
$build = if ([IO.Path]::IsPathRooted($BuildDir)) { [IO.Path]::GetFullPath($BuildDir) } else { Join-Path $root $BuildDir }
$out = Join-Path $root 'release-stage'
$target = 'SuperMarioWorldRecomp'
$package = 'SuperMarioAdvance2Recomp'
$stageName = "$package-windows-x64-v$Version"
$stage = Join-Path $out $stageName
$zip = Join-Path $out "$stageName.zip"
$expectedSha1 = '5101ddf223d1d918928fe1f306b63a42ada14a5e'
$dlls = @(
    'SDL2.dll',
    'libgcc_s_seh-1.dll',
    'libstdc++-6.dll',
    'libwinpthread-1.dll'
)

foreach ($required in @(
    (Join-Path $mingwBin 'cc.exe'),
    (Join-Path $mingwBin 'c++.exe'),
    (Join-Path $mingwBin 'ninja.exe'),
    (Join-Path $mingwBin 'strip.exe'),
    (Join-Path $engine 'CMakeLists.txt'),
    (Join-Path $engine 'tools\fetch_tcc.ps1'),
    (Join-Path $generatedBios 'bios_recompiled.cpp'),
    (Join-Path $generatedBios 'bios_recompiled.h'),
    (Join-Path $uiRoot 'recomp_ui.cmake')
)) {
    if (-not (Test-Path -LiteralPath $required)) {
        throw "Required release dependency is missing: $required"
    }
}

$generated = @(Get-ChildItem -LiteralPath (Join-Path $root 'generated') `
    -Filter 'recompiled_*.cpp' -File -ErrorAction SilentlyContinue)
if ($generated.Count -eq 0) {
    throw 'Generated guest shards are missing. Supply the verified ROM and run tools\regen.ps1 first.'
}

$env:PATH = "$mingwBin;$env:PATH"
New-Item -ItemType Directory -Force -Path $out | Out-Null

& cmake -S $root -B $build -G Ninja `
    -DCMAKE_C_COMPILER="$mingwBin/cc.exe" `
    -DCMAKE_CXX_COMPILER="$mingwBin/c++.exe" `
    -DCMAKE_MAKE_PROGRAM="$mingwBin/ninja.exe" `
    -DCMAKE_BUILD_TYPE=Release `
    "-DGAME_BUILD_VERSION=$Version" `
    "-DGBARECOMP_GENERATED_BIOS_DIR=$generatedBios" `
    "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG" `
    -DGBARECOMP_ROOT="$engine" `
    -DRECOMP_UI_ROOT="$uiRoot" `
    -DGBARECOMP_RUNTIME_UI_ROOT="$uiRoot" `
    -DGBARECOMP_BUILD_ORACLE=OFF `
    -DGBARECOMP_MINGW_PREFIX_UNIX='/c/msys64/mingw64' `
    -DSDL2_INCLUDE_DIR='C:/msys64/mingw64/include/SDL2' `
    -DSDL2_LIBRARY='C:/msys64/mingw64/lib/libSDL2.dll.a'
if ($LASTEXITCODE -ne 0) { throw "Release configure failed ($LASTEXITCODE)." }

& cmake --build $build --target $target --parallel $Jobs
if ($LASTEXITCODE -ne 0) { throw "Release build failed ($LASTEXITCODE)." }

$exe = Join-Path $build "$target.exe"
if (-not (Test-Path -LiteralPath $exe)) {
    throw "Expected executable is missing: $exe"
}
& (Join-Path $mingwBin 'strip.exe') $exe
if ($LASTEXITCODE -ne 0) { throw "strip failed ($LASTEXITCODE)." }

if (Test-Path -LiteralPath $stage) {
    $resolvedStage = (Resolve-Path -LiteralPath $stage).Path
    $resolvedOut = (Resolve-Path -LiteralPath $out).Path
    if (-not $resolvedStage.StartsWith(
            "$resolvedOut\", [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove stage outside release output: $resolvedStage"
    }
    $stageItems = @(Get-Item -LiteralPath $resolvedStage) +
        @(Get-ChildItem -LiteralPath $resolvedStage -Recurse -Force)
    if ($stageItems | Where-Object {
        ($_.Attributes -band [IO.FileAttributes]::ReparsePoint) -or
        $_.Name -eq '.git'
    }) { throw 'Refusing to remove a stage containing a link or repository.' }
    Remove-Item -LiteralPath $resolvedStage -Recurse
}
New-Item -ItemType Directory -Force -Path $stage | Out-Null
Copy-Item -LiteralPath $exe -Destination $stage
$hqx = Join-Path $build 'gbarecomp-hqx.dll'
$hqxSource = Join-Path $build 'third-party/hqx'
if (-not (Test-Path -LiteralPath $hqx) -or
    -not (Test-Path -LiteralPath (Join-Path $hqxSource 'COPYING'))) {
    throw 'The HQx shared library and corresponding source are required.'
}
Copy-Item -LiteralPath $hqx -Destination $stage
New-Item -ItemType Directory -Path (Join-Path $stage 'third-party') | Out-Null
Copy-Item -LiteralPath $hqxSource -Destination (Join-Path $stage 'third-party') -Recurse
foreach ($dll in $dlls) {
    $source = Join-Path $mingwBin $dll
    if (-not (Test-Path -LiteralPath $source)) {
        throw "Runtime DLL is missing: $source"
    }
    Copy-Item -LiteralPath $source -Destination $stage
}

$assets = Join-Path $build 'assets'
if (-not (Test-Path -LiteralPath (Join-Path $assets 'img'))) {
    throw "recomp-ui launcher assets are missing: $assets"
}
Copy-Item -LiteralPath $assets -Destination $stage -Recurse

$mods = Join-Path $build 'mods'
$modPackages = Join-Path $mods 'packages'
if (-not (Test-Path -LiteralPath $modPackages)) {
    throw "Preloaded mod catalog is missing: $mods"
}
$stagedMods = Join-Path $stage 'mods'
New-Item -ItemType Directory -Force -Path $stagedMods | Out-Null
Copy-Item -LiteralPath $modPackages -Destination $stagedMods -Recurse

& (Join-Path $engine 'tools\fetch_tcc.ps1') `
    -Toolchain (Join-Path $stage 'overlay_toolchain') -EngineRoot $engine -CacheDir (Join-Path $build 'release-toolchain-cache')
if ($LASTEXITCODE -ne 0) {
    throw "Overlay toolchain staging failed ($LASTEXITCODE)."
}

@"
# Super Mario Advance 2: Super Mario World - Windows x64

This experimental v$Version preview runs the USA/Australia retail game through
gbarecomp with the recomp-ui launcher, Adaptive Widescreen, and optional Pixel Smoothing.

## How to run

1. Extract the entire folder. Keep the executable, DLLs, assets, mods, and
   overlay_toolchain together.
2. Run $target.exe.
3. Select your legally obtained Super Mario Advance 2 ROM.
   Expected SHA-1: $expectedSha1
4. Select your legally obtained retail 16 KiB gba_bios.bin.
5. Configure display and controls, then select PLAY. Adaptive Widescreen is on
   for new installations. Enable Pixel Smoothing under Mods and select HQ2x,
   HQ3x or HQ4x before Play if desired; smoothing is off by default.

The ROM and retail GBA BIOS are not included. Supply your own dumps.

Project: https://github.com/mstan/SuperMarioAdvance2Recomp
Engine: https://github.com/mstan/gbarecomp
"@ | Out-File -LiteralPath (Join-Path $stage 'README.md') -Encoding utf8

# Preserve only the authored runtime display model, not development metadata.
$sourceToml = [IO.File]::ReadAllText((Join-Path $root 'game.toml'))
$video = [regex]::Match($sourceToml, '(?ms)^\[video\]\s*\r?\n(.*?)(?=^\[|\z)')
$screen = [regex]::Match($video.Groups[1].Value,
    '(?m)^\s*screen\s*=\s*"([A-Za-z0-9_-]+)"\s*(?:#.*)?$')
# No authored video.screen retains the runner's raw default.
$screenName = if ($screen.Success) { $screen.Groups[1].Value } else { 'raw' }
$runtimeConfig = "[video]`nscreen = `"$screenName`"`n"
[IO.File]::WriteAllText((Join-Path $stage 'game.toml'), $runtimeConfig)
$fontNotices = @('LatoLatin-OFL.txt', 'NotoSansSymbols2-OFL.txt', 'OpenMoji-LICENSE.txt')
$fontNoticeStage = Join-Path $stage 'licenses/fonts'
New-Item -ItemType Directory -Force -Path $fontNoticeStage | Out-Null
foreach ($notice in $fontNotices) {
    Copy-Item -LiteralPath (Join-Path $root "tools/release-licenses/$notice") -Destination $fontNoticeStage
}
& (Get-Command python.exe).Source (Join-Path $root 'tools/collect_licenses.py') `
    $engine $uiRoot (Join-Path $stage 'licenses') --mingw (Split-Path $mingwBin)
if ($LASTEXITCODE -ne 0) { throw 'Dependency notice staging failed.' }

$forbidden = Get-ChildItem -LiteralPath $stage -Recurse -File |
    Where-Object {
        $_.Extension.ToLowerInvariant() -in @(
            '.gba', '.agb', '.bin', '.sav', '.srm', '.apk'
        ) -or
        $_.Name -in @(
            'rom.cfg',
            'bios.cfg',
            'config.ini',
            'keybinds.ini',
            'mods.ini',
            'state.toml'
        )
    }
if ($forbidden) {
    throw "Forbidden release content detected: $($forbidden.FullName -join ', ')"
}

if (Test-Path -LiteralPath $zip) {
    Remove-Item -LiteralPath $zip -Force
}
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

# ZIP entry names must always use '/', regardless of the host OS.
# Compress-Archive preserves Windows backslashes, which POSIX extractors
# treat as literal filename characters rather than directory separators -- so
# a Linux / Steam Deck / Proton user gets files literally named
# "assets\fonts\LatoLatin-Regular.ttf" (and "mods\packages\..." where the
# game ships a mod catalog), the nested trees are never created, and the
# ImGui launcher finds neither its fonts nor its mods. Write portably here,
# then verify before anyone can publish it.
# (Convention ported from snesrecomp/SuperMarioWorldRecomp.)
$rzStageFull = [IO.Path]::GetFullPath((Resolve-Path -LiteralPath $stage).Path)
$rzZipFull = [IO.Path]::GetFullPath($zip)
$rzPrefix = $rzStageFull.TrimEnd('\') + '\'
$rzFiles = @(Get-ChildItem -LiteralPath $stage -File -Recurse |
    Sort-Object FullName)
$rzArchive = [IO.Compression.ZipFile]::Open(
    $rzZipFull, [IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($rzFile in $rzFiles) {
        $rzFull = [IO.Path]::GetFullPath($rzFile.FullName)
        if (-not $rzFull.StartsWith(
                $rzPrefix, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Refusing to archive a file outside the release stage: $rzFull"
        }
        $rzName = $rzFull.Substring($rzPrefix.Length).Replace('\', '/')
        if ($rzName.StartsWith('/') -or $rzName -match '(^|/)..(/|$)') {
            throw "Unsafe ZIP entry name: $rzName"
        }
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
            $rzArchive, $rzFull, $rzName,
            [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
} finally {
    $rzArchive.Dispose()
}

# Read the archive back and reject non-portable entry names outright, so a
# regression in the writer cannot ship a Windows-only zip again.
$rzArchive = [IO.Compression.ZipFile]::OpenRead($rzZipFull)
try {
    $rzBad = @($rzArchive.Entries | Where-Object {
        $_.FullName.Contains('\') -or
        $_.FullName.StartsWith('/') -or
        $_.FullName -match '(^|/)..(/|$)'
    })
    if ($rzBad.Count -ne 0) {
        throw "ZIP contains non-portable entry names: $(
            ($rzBad | ForEach-Object FullName) -join ', ')"
    }
    $runtimeEntry = $rzArchive.GetEntry('game.toml')
    if ($null -eq $runtimeEntry) { throw 'Runtime display config missing from ZIP.' }
    $reader = [IO.StreamReader]::new($runtimeEntry.Open(), [Text.Encoding]::UTF8)
    try { $packagedConfig = $reader.ReadToEnd() } finally { $reader.Dispose() }
    if ($packagedConfig -cne $runtimeConfig) { throw 'ZIP display config differs.' }
    foreach ($notice in $fontNotices) {
        if ($null -eq $rzArchive.GetEntry("licenses/fonts/$notice")) {
            throw "Bundled font notice missing: $notice"
        }
    }
    if ($rzArchive.Entries.Count -ne $rzFiles.Count) {
        throw "ZIP entry count mismatch: expected $($rzFiles.Count), got $(
            $rzArchive.Entries.Count)"
    }
} finally {
    $rzArchive.Dispose()
}
Write-Host "--- $stageName ---"
Get-ChildItem -LiteralPath $stage | Select-Object Name, Length | Out-Host
Get-Item -LiteralPath $zip | Select-Object Name, Length | Out-Host
Get-FileHash -LiteralPath $zip -Algorithm SHA256 |
    Select-Object Algorithm, Hash, Path | Out-Host
