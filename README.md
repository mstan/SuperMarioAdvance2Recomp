# SuperMarioWorldRecomp

Static recompilation scaffold for **Super Mario Advance 2: Super Mario World**
(USA, Australia) using a dedicated GBARecomp worktree.

The ROM, generated sources, build outputs, and saves remain local and ignored.

```powershell
pwsh tools/regen.ps1
C:\msys64\mingw64\bin\cmake.exe -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe `
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe
C:\msys64\mingw64\bin\cmake.exe --build build --target SuperMarioWorldRecomp --parallel
```

## Attract-demo smoke test

The bounded smoke test runs 6,000 no-input frames through GBARecomp's
whole-program interpreter oracle and captures the resulting title screen after
the attract demo:

```powershell
pwsh tools/smoke-attract.ps1
```

The expected capture is `artifacts/attract-complete.png`. The interpreter
backend is intentional for this first correctness milestone; native static
coverage is scaffolded and can be expanded independently.

## Strict AOT coverage

`game.toml` enables the bounded AOT scanner over the executable ROM range. It
combines direct CFG discovery with reachable callback-table harvesting while
retaining reviewed RAM code-copy evidence. Full static-resume mode also emits a
native entry for every interruptible instruction boundary.

After generating and building the `build-aot` target, run:

```powershell
.\tools\coverage-attract.ps1
```

The gate disables cached overlays and interpreter bridging. It succeeds only
when all 6,000 attract frames complete with `FULLY_STATIC` coverage and zero
dispatch misses.
