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
