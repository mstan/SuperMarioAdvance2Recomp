# Super Mario Advance 2 Recomp

> **Experimental preview.** This recompilation is a byproduct of developing
> [gbarecomp](https://github.com/mstan/gbarecomp): the games are the proving
> ground, while the reusable framework is the larger goal. This is not a
> finished commercial port, so expect rough edges and please report problems.
> For more context, read
> [Recomp + AI: 5 Months Later »](https://1379.tech/recomp-ai-5-months-later/).

Static recompilation of **Super Mario Advance 2: Super Mario World** for
Windows. It runs the original Game Boy Advance game as a native desktop
application and includes an optional Adaptive Widescreen mod.

The game ROM and Nintendo GBA BIOS are **not included**. You must provide your
own legally obtained dumps.

## Status

The game boots and runs through its opening and attract sequence. The current
`v0.0.3` build is an experimental release, and the widescreen work is still
being refined scene by scene. Back up important saves and report any
repeatable gameplay or visual problems.

## Quick start

1. Download the Windows zip from [Releases](../../releases) and extract it.
2. Run `SuperMarioWorldRecomp.exe`.
3. In the launcher, select your **Super Mario Advance 2: Super Mario World
   (USA/Australia)** ROM and retail GBA BIOS.
4. Configure display, audio, controls, and mods, then select **Play**.

The launcher remembers valid files after the first setup. Enable **Skip
launcher on boot** if you want later launches to go directly into the game.

## Adaptive Widescreen

Open the launcher's **Mods** page to enable **Adaptive Widescreen**. It renders
additional game content at the sides instead of stretching the original
240×160 image.

The mod is disabled by default. Menus and HUD elements remain in the original
safe area, while supported gameplay scenes can expand up to 9:5. Some scenes
intentionally retain the original view where the game does not prepare enough
off-screen level or actor data.

## Features

- Native Windows x64 application
- ROM and BIOS setup through the shared
  [recomp-ui](https://github.com/mstan/recomp-ui) launcher
- Optional Adaptive Widescreen mod
- Keyboard and modern game-controller support
- Windowed and fullscreen play with configurable presentation
- In-game settings menu
- Cartridge saves and save states

## Controls

| GBA control | Keyboard |
|---|---|
| D-Pad | Arrow keys |
| A / B | X / Z |
| Start | Enter |
| Select | Right Shift |
| L / R | C / V |

Use **Shift+F1-F9** to save a state and **F1-F9** to load one. Controls can be
changed from the launcher.

## Building from source

Windows development requires CMake, Ninja, MSYS2 MinGW64, and SDL2:

```powershell
git clone --recurse-submodules `
  https://github.com/mstan/SuperMarioAdvance2Recomp.git
cd SuperMarioAdvance2Recomp

cmake -S gbarecomp -B gbarecomp/build -G Ninja
cmake --build gbarecomp/build --target gba_recompile
pwsh tools/regen.ps1
cmake -S . -B build -G Ninja
cmake --build build --target SuperMarioWorldRecomp
```

Generation requires the supported ROM revision and a retail GBA BIOS. Their
identities and local development paths are documented in
[`baserom.md`](baserom.md) and [`game.toml`](game.toml). ROM-derived generated
code, copyrighted inputs, saves, and build output remain local and are never
included in releases.

Contributors can run `pwsh tools/coverage-attract.ps1` for the automated native
acceptance route and `pwsh tools/make_release.ps1 -Version 0.0.3` to build a
sanitized Windows package.

## License

PolyForm Noncommercial 1.0.0 — see [`LICENSE`](LICENSE). Third-party
components retain their own licenses.

## Legal

This is an unofficial, non-commercial preservation and research project. It
is not affiliated with or endorsed by Nintendo. Mario and related names,
characters, artwork, and game data are trademarks or copyrights of their
respective owners.

No copyrighted game ROM or Nintendo BIOS data is distributed by this project.

---

Part of the **R.A.I.D. — Retro AI Development** static-recompilation
community.
