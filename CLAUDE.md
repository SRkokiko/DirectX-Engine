# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Layout
- `Source/`: engine code (mostly flat; new modules go in `Source/Modules/`), `Engine.sln`, `Engine.vcxproj`, and `Source/3rdParty/`. Keep VS filters mirroring disk folders; include subfolder headers as `"Modules/X.h"` (`Source` is an include dir) and keep `.cpp` names unique (shared obj dir).
- `Game/`: runtime working directory (`WinPixEventRuntime.dll`, `Assets/`).

## Build & run
- Windows only, Visual Studio 2022 (toolset v143), C++20, x64 only. Solution: `Source/Engine.sln` (projects: `Engine`, `DirectXTex`; configs Debug|x64, Release|x64). Win32 configs in Engine.vcxproj are stale.
- Build: `msbuild Source\Engine.sln /p:Configuration=Debug /p:Platform=x64`
- Output: `Source/build/out/x64/<Config>/`, intermediates `Source/build/tmp/` (gitignored).
- Run with working directory `Game/` (set in Engine.vcxproj.user); assets go in `Game/Assets/`.
- No tests, lint, CMake or CI.
- Rebuilding DirectXTex modifies tracked binaries under `Source/3rdParty/DirectXTex/Bin/` (.lib/.dll files are intentionally tracked).

## Build gotchas
- `Source/Engine.vcxproj` lists every file explicitly: add new files by hand.
- `Source/Globals.h` is the PCH (created by `Globals.cpp`); new engine .cpp files must `#include "Globals.h"` first. Third-party-style files (SimpleMath.cpp, DebugDrawPass.cpp) are marked NotUsing.
- vcxproj references nonexistent dirs (`Source/3rdParty/mikktspace`, `Source/3rdParty/directX`, `imgui-1.89.8/...`); tinygltf is present but not compiled yet.
- Debug defines `USE_PIX 1` enabling `BEGIN_EVENT/END_EVENT/SET_MARKER` macros (Globals.h); compiled out in Release.
- Don't edit vendored code: `Source/3rdParty/`, DirectXTK files (Keyboard, Mouse, GamePad, SimpleMath, ReadData.h), `par_shapes.h`, `debug_draw.hpp`.

## Architecture
- Entry: `wWinMain` in `Engine.cpp` creates global `Application* app`. Frames are driven by `WM_PAINT` -> `app->update()` (no idle pump yet). WndProc forwards to ImGui, then Keyboard/Mouse.
- `Application` owns `std::vector<Module*>` (`ModuleD3D12` then `ModuleInput` registered). `init` in registration order, `cleanUp` reversed; per frame: `update` -> `preRender` -> `render` -> `postRender` across all modules. Also tracks FPS/elapsed time.
- Render passes (`ImGuiPass`, `DebugDrawPass`) are standalone classes taking D3D12 objects; not yet wired into a module. `ModuleD3D12` (`Source/Modules/`) is an empty skeleton that will own D3D12 init/cleanup; no device/swapchain/queue/descriptor-heap objects exist yet.
- `Globals.h`: `LOG(fmt, ...)` (OutputDebugString), `FRAMES_IN_FLIGHT = 3`, `alignUp`, common includes/namespaces.
- Shaders: no .hlsl files; DebugDrawPass compiles inline HLSL via `D3DCompile` (vs_5_0/ps_5_0) at runtime.
- Legacy name "EngineDX" still appears in code (e.g. `IDC_ENGINEDX`).
