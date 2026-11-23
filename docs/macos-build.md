# macOS client build

Two layers: `macosx-build/` in ryzom-core (runs on a Mac) and the assembly done by `ryzom_build_client.sh` in `ryzom-private-data/infra` (`templates/ribs/scripts/build/`).

## ryzom-core: `macosx-build/`

- `setup-environment.sh`: one-time toolchain setup (Xcode CLT, Rosetta 2, CMake, xcbeautify, git).
- `build-externals.sh`: builds third-party libs as universal2 static libs into `macosx-build/external/` (skips libs already built).
- `build.sh [fv|steam]`: builds `Ryzom.app` universal2 (x86_64 + arm64), deployment target 11.0. `steam` needs `STEAM_DIR`.
- `lipo-merge.sh <x86_64> <arm64> <out>`: merges two already linked Mach-O executables (runs on Linux, no relink).

Xcode 16+ cannot target below macOS 11.0, hence the two-machine pipeline below.

## Distribution pipeline (infra)

`compile_clients()` -> `build_macos_clients $TYPE`:

1. `build_macos_newarch`: ssh to `$MACOS_NEWARCH_SSH_SERVER`, `git pull`, `macosx-build/build.sh $TYPE`, fetches `Ryzom.app` (always run, live logs).
2. `build_macos_oldarch` (`fv` only): ssh to `$MACOS_OLDARCH_SSH_SERVER`, x86_64 build targeting 10.8. Runs in parallel with 1; its log is shown only on failure.
3. `macos_lipo_merge` (`fv` only, if both succeeded): for `Ryzom` and `RyzomClientPatcher`, merges old-arch x86_64 + `lipo -thin arm64` of the new-arch one; sets `LSMinimumSystemVersion` to `10.8` in `Info.plist` (python3 + plistlib); re-signs the bundle ad hoc (`codesign --force --deep -s -`, then `--verify --deep --strict`) on the new-arch Mac, since any bundle edit invalidates the signature.
4. If old-arch failed: warning, the universal2 11.0 bundle is delivered as is. If merge or signing fails: error, no deliverable.

`steam`: never merged (no Ryzom player below macOS 12 per the Steamworks Hardware Survey); the new-arch bundle is the deliverable.
