# Super Mario Advance 2 Recomp

Experimental static recompilation of **Super Mario Advance 2: Super Mario
World** (USA, Australia) for Windows x64, built with
[gbarecomp](https://github.com/mstan/gbarecomp) and the shared
[recomp-ui](https://github.com/mstan/recomp-ui) launcher.

This repository never includes Nintendo game data. Supply your own legally
obtained retail GBA BIOS and matching cartridge dump.

## Status

- The 6,000-frame attract route passes in strict static mode with zero dispatch
  misses or interpreted instructions.
- The pre-boot launcher supports ROM/BIOS selection, display, audio, input, and
  mod configuration.
- Adaptive Widescreen is included as an optional, disabled-by-default mod.
- This remains an experimental preview. Back up important saves and expect
  incomplete scene-specific widescreen handling.

## Required files

- Super Mario Advance 2: Super Mario World (USA, Australia)
  - SHA-1: `5101ddf223d1d918928fe1f306b63a42ada14a5e`
- Retail 16 KiB `gba_bios.bin`
  - SHA-1: `300c20df6731a33952ded8c436f7f186d25d3492`

ROMs, BIOS images, generated ROM-derived source, saves, caches, and build
outputs are ignored by Git and excluded from release archives.

## Clone and build

```powershell
git clone --recurse-submodules `
  https://github.com/mstan/SuperMarioAdvance2Recomp.git
cd SuperMarioAdvance2Recomp

# Put your verified ROM at roms/super_mario_world_usa.gba and your BIOS at
# gbarecomp/bios/gba_bios.bin. Build the local generator, then generate the
# ignored BIOS and cartridge sources.
C:\msys64\mingw64\bin\cmake.exe -S gbarecomp -B gbarecomp/build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release
C:\msys64\mingw64\bin\cmake.exe `
  --build gbarecomp/build --target gba_recompile --parallel
pwsh tools/regen.ps1

C:\msys64\mingw64\bin\cmake.exe -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe `
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe
C:\msys64\mingw64\bin\cmake.exe `
  --build build --target SuperMarioWorldRecomp --parallel
```

To create the sanitized Windows archive:

```powershell
pwsh tools/make_release.ps1 -Version 0.0.1
```

The packager rejects ROMs, BIOS images, saves, generated source, developer
configuration, and diagnostic artifacts before creating the zip.

## Validation

Run the strict native acceptance gate after generating and building
`build-aot`:

```powershell
pwsh tools/coverage-attract.ps1 -Frames 6000
```

The gate disables interpreter and overlay fallback. It succeeds only when all
6,000 frames complete with `FULLY_STATIC` coverage and zero dispatch misses.

## Adaptive Widescreen

Native 240x160 remains the faithful default. Open **Mods** and enable
**Adaptive Widescreen** to let the logical view follow the live window or
fullscreen aspect ratio up to 288x160 (9:5).

The adapter exposes SMA2's nearby streamed background columns while keeping
menus, HUD elements, and the sprite-composed title in the original safe area.
Some scenes retain the native view because the stock game does not populate
enough off-screen level or actor data.

This project is part of the R.A.I.D. static-recompilation community.
