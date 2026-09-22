# ryzom-core

## 2026-09-22 — ✨ Clip hairstyle mesh against equipped hat volume (HAT_SLOT)

`ajout_hat_slot__hair_clip_alpha.md`: wide hairstyles (`HEAD_SLOT`) visually poking
through hats (`HAT_SLOT`, added separately in `ajout_hat_slot.md`) are now fixed by
building, per character and only while both slots are occupied, a private (never
shape-bank-shared) copy of the hairstyle mesh with the geometry inside the hat's volume
actually cut away — a real geometric clip, computed once at equip time. New module
`ryzom/client/src/hairstyle_hat_clip.{h,cpp}`; see `docs/hairstyle_hat_clip.md` for the
full design (coordinate-space handling, the `<hatShapeName>_mask.shape` clipping-volume
convention, the clipping algorithm, and known limitations).

Driven from a new `CPlayerCL::updateVisible()` override (`player_cl.h`/`.cpp`), not from
`updateVisualPropertyVpa()`: the latter runs before the character's skeleton is fully
configured for the frame (bone world matrices still default/identity, character scale not
yet reflected), which produced three distinct "skeleton not ready" symptoms during
testing (unresolved bone matrices, halved-scale positions, and world-space coordinates
imprecise enough to swamp the sub-centimeter containment test on a large continent) before
landing on `updateVisible()` and an explicit recenter-on-the-bone-position step.

Two small public accessors added to the engine, both previously missing: `CMeshGeom::
getBonesName()` (`nel/include/nel/3d/mesh.h`) and `CMeshMRMSkinnedGeom::getLodGeomorphs()`
(`nel/include/nel/3d/mesh_mrm_skinned.h`, though `getGeomorphs()` already existed there
doing the same thing under a different name — worth deduplicating later, not done here).

`entity_cl.cpp`'s `SInstanceCL::updateCurrentFromLoading()` gained two `(IA_AGENT_DEBUG)`-
tagged log lines during testing (kept out of the final commit) to confirm
`KeepHiddenWhenLoaded` — already-existing engine plumbing — was being respected correctly
for hiding the normal (shared) hairstyle instance while a clipped one is active.

## 2026-09-16 — ✨ Add smoothing angle property to material

Added `CMaterial::_SmoothingAngle` (float, sentinel `-1.0` = never set) on
branch `feature/shape-smoothing-property`, purely as tool metadata for Ryzom
Forgery ("Patina") to store and retrieve the per-material normal-smoothing
angle the user set, directly inside the `.shape` itself, without an external
sidecar. Never read by rendering, which still relies solely on the
already-baked normals (`VertexBuffer.channels["Normal"]`).

`CMaterial::serial()` bumped from `serialVersion(9)` to `serialVersion(10)`,
serializing `_SmoothingAngle` only for `ver >= 10` (defaulting to `-1.0`
otherwise), following the same per-version branching pattern already used for
every other field added since version 2. A client older than version 10
refuses to load a `.shape` written with this new field (`ENewerStream`,
`_ThrowOnNewer` defaults to `true`) rather than silently corrupting it --
client/server rollout is handled entirely by Nuno.

Validated with a standalone round-trip test (`test_smoothing_angle.cpp`,
`CMemStream`, built via `ryzom-docker/build.sh tools`): default material ->
`getSmoothingAngle() == -1.0f`; round-trip of `42.5f` -> exact value
recovered. Test file and its temporary CMake target were removed after
validation, never committed. Required by
`project-todos/pynel/material_smoothing_angle_field.md` (Python mirror) and,
through it, `project-todos/forgery/smooth_normals.md` (Patina's per-material
Smoothing UI), both closed the same day this was validated end-to-end.

## 2026-09-09 — 🐛 Fix ryzom_export dragging in MFC on Windows

Building the NeL/Ryzom tools for Windows (`ryzom-docker`'s new `tools_win64` target) failed on `ryzom_export` with `fatal error C1083: Cannot open include file: 'afxwin.h'`. Its `master/easy_cfg.cpp`/`ContinentCfg.cpp` sources (added to the target by the previous `land_export` port above) are genuinely platform-independent, but on Windows they still `#include "stdafx.h"`, which resolves to `master/StdAfx.h` — the 3ds Max plugin's own precompiled header, which unconditionally pulls in `afxwin.h`/`afxext.h`/etc. unless `_CONSOLE` is defined (the standard MFC escape hatch for non-MFC targets sharing that file). `ryzom_export` never defined it.

Fixed in `ryzom/tools/leveldesign/export/CMakeLists.txt` by adding `TARGET_COMPILE_DEFINITIONS(ryzom_export PRIVATE _CONSOLE)`. `ryzom_export` isn't an MFC application and neither `easy_cfg.cpp` nor `ContinentCfg.cpp` actually use MFC, so this has no effect beyond skipping that unconditional include block — no need to install the optional MFC Visual Studio component in any build toolchain.

Validated end-to-end via `ryzom-docker`'s `tools_win64` target: the full NeL/Ryzom tools set (including `ryzom_export`, `land_export`, `zviewer`, and everything else) now builds successfully for Windows.

## 2026-09-06 — 🐧 Port land_export/ryzom_export to Linux

`ryzom_export` (the NeL/Ryzom level-design export library, `ryzom/tools/leveldesign/export/`) was only ever added to the CMake build under `IF(WIN32)`, so `land_export`/`ryzom_landexport` — which link against it for `CTools`'s file/directory utilities — failed to link on Linux with `cannot find -lryzom_export`.

Ported `CTools` (`export/tools.cpp`): every method now has a POSIX implementation alongside the existing Windows one (`#ifdef NL_OS_WINDOWS`/`#else`), built on `NLMISC::CPath`/`NLMISC::CFile` (`getCurrentPath`/`setCurrentPath`, `createDirectoryTree`, `fileExists`, `getFileModificationDate`, `getPathContent`, `copyFile`). `fileDateCmp`'s FILETIME overload converts the raw Windows FILETIME components (persisted in zone-region data) to a Unix epoch via the standard `(filetime - 116444736000000000) / 10000000` formula.

Ported `CExport::getAllFiles`/`CExport::searchFile` and the ad-hoc directory listing in `CExport::newExport` (`export.cpp`) from `WIN32_FIND_DATA`/`FindFirstFile`/`FindNextFile` to `NLMISC::CPath::getPathContent`-based enumeration, and replaced the remaining raw `GetCurrentDirectory`/`SetCurrentDirectory` calls in `doExport`/`generateIGFromFlora` with the now-portable `CTools::pwd`/`CTools::chdir`. Fixed several hardcoded `"\\"` path separators to `"/"` (`_OutIGDir`, `_InLandscapeDir`, `sContinentDir`, the plant-lookup path in `generateIGFromFlora`, and — found only once Nuno tested the tool for real — four more in `land_export_lib/export.cpp`'s `CExport::treatPattern`).

Discovered mid-port that `CExport::newExport` depends on `SContinentCfg`/`IEasyCFG` (`ryzom/tools/leveldesign/master/ContinentCfg.cpp`/`easy_cfg.cpp`), which live in the `master/` MFC app and were never compiled by any CMake target on any platform. Both files are otherwise fully portable (their `#include "stdafx.h"` is the only Windows-specific line, now guarded); added them directly to `ryzom_export`'s CMake sources instead of pulling in all of `master/`.

`ryzom/tools/leveldesign/CMakeLists.txt`'s `ADD_SUBDIRECTORY(export)` is now gated only by `IF(WITH_LIGO)` (previously also required `IF(WIN32)`); `land_export_lib/CMakeLists.txt` needed no change since it already linked `ryzom_export` unconditionally.

Validated end-to-end: `ryzom_export`, `ryzom_landexport` and `land_export` all build and link successfully via `ryzom-docker`'s tools build, and Nuno confirmed `land_export` runs correctly against real data.

## 2026-09-06 — 🐛 Fix Linux CPU affinity stack corruption and uninitialized main thread handle

While validating `zone_lighter`/`zone_welder` orchestration from pynel (`project-todos/pynel/zone_read_write.md` step 4), `zone_lighter` crashed on every invocation, right at its first line of `light()` (`nel/src/3d/zone_lighter.cpp:929`, `currentThread->getCPUMask()`), before reaching any lighting logic.

Two distinct bugs in `nel/src/misc/p_thread.cpp`, both Linux-only (the Windows equivalents in `win_thread.cpp` were never affected):

1. **Stack corruption in the CPU-affinity functions.** `CPThread::getCPUMask()`/`setCPUMask()` and `CPProcess::getCPUMask()`/`setCPUMask()` all called glibc's `pthread_getaffinity_np`/`pthread_setaffinity_np`/`sched_getaffinity`/`sched_setaffinity` with `sizeof(uint64)` (8 bytes) as the `cpu_set_t` size, while reinterpreting a local `uint64` as a `cpu_set_t*`. A real `cpu_set_t` on this system is 128 bytes (`CPU_SETSIZE` = 1024 bits) — glibc read/wrote past the 8-byte stack buffer believing it had a full `cpu_set_t`, corrupting the caller's stack. `CPThread::getCPUMask()`/`setCPUMask()` even carried a `nlwarning("This code does not work. May cause a segmentation fault...")` acknowledging the bug without ever fixing it. Fixed all four functions to use a real `cpu_set_t` (`CPU_ZERO`/`CPU_SET`/`CPU_ISSET`), converting to/from the existing `uint64` bitmask API for the first 64 CPUs.
2. **`CPThread::_ThreadHandle` never initialized.** Fixing bug 1 alone didn't stop the crash: the minidump still pointed inside `pthread_getaffinity_np` itself. `CPThread`'s constructor never sets `_ThreadHandle`, and `CPMainThread` (the static global wrapper representing the main thread, `p_thread.cpp:44`) is zero-initialized before its constructor runs — so `_ThreadHandle` was `0`, an invalid `pthread_t`, for every call made from the main thread. Fixed by explicitly setting `_ThreadHandle = pthread_self();` in `CPMainThread`'s constructor.

Validated end-to-end: rebuilt `zone_welder`/`zone_lighter` via `ryzom-docker`'s tools build, ran the full pipeline on a real `.zone` (welded, then lighted) — no crash, `Number of CPU used: 1` logged correctly, and the resulting `.zonel` parses successfully with `pynel.ryzom_zone.load_zone()`.
