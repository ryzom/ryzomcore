# Build version info (DESCRIBE / BUILD_DATE)

The client's version string (`RYZOM_VERSION`) and build date (`BUILD_DATE`) are produced at build time, not at configure time.

## How it works

- `CMakeModules/BuildInfo.cmake` runs in script mode (`cmake -P`) and writes `${CMAKE_BINARY_DIR}/ryzom_build_info.h`, which redefines `RYZOM_VERSION` and `BUILD_DATE` (overriding the configure-time values from `config.h`).
- The custom target `ryzom_build_info` (`ryzom/CMakeLists.txt`, only with `WITH_RYZOM_CLIENT`) runs that script on every build. `ryzom_client` and `ryzom_client_patcher` depend on it, since both compile `ryzom/client/src/user_agent.cpp`, the only file including the header.
- `BUILD_DATE` has second precision, so each build recompiles `user_agent.cpp` and relinks both executables.

## DESCRIBE

- If the `DESCRIBE` CMake variable is set at configure time (`-DDESCRIBE=...`, as the Linux/Windows build scripts do), it is used as is.
- Otherwise it is computed from git: `<domain> v<yy>.<mm>.<revision> #<commit>`, where `<revision>` is the commit count of `ryzom/client/src`, and `<domain>` is `Alpha /` on `main/yubo-dev`, `Beta /` on `main/gingo-test`, `Omega /` elsewhere.
- Without `DESCRIBE` and without git, `RYZOM_VERSION` keeps its `config.h` value.
- The bundle's `Info.plist` version (`MACOSX_BUNDLE_LONG_VERSION_STRING`) is still set at configure time.

## macOS build script

`macosx-build/build.sh` only runs the CMake configure step when needed:

- `build-<type>/CMakeCache.txt` does not exist,
- `FORCE_CONFIGURE=1` is set,
- or the cache holds a `DESCRIBE` entry (it would override the build-time value); that configure drops it with `-UDESCRIBE`.

A configure rewrites `config.h` (its `BUILD_DATE` changes), which recompiles the handful of files including it. The configure is run with `-Wno-deprecated` to hide the `CMAKE_MINIMUM_REQUIRED(VERSION 2.6)` deprecation warnings.
