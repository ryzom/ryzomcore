#!/bin/bash
# Prepare an extracted toolchain_v90_prefix wine prefix for building
# ryzomcore with VS2008 under Wine. Idempotent; run once after extraction.
#
#   NL_WINE_VS2008_PREFIX    the extracted prefix   (default $HOME/toolchain_v90_prefix)
#   NL_WINE_VS2008_SHADOW    shadow VC dir to create (default <prefix>-msvc-shadow)
#   NL_WINE_VS2008_EXTERNAL  externals root          (default <prefix>/drive_c/2019q4_external_v90_x86)
#
# What it does:
#   1. dosdevices links (packaged prefix ships without symlinks - 7z would
#      dereference them; z: -> / in particular would archive the whole fs)
#   2. shadow VC dir: CMakeModules/FindMSVC.cmake derives VC_DIR by regexing
#      "/bin/.+" off the compiler path, so the wrapper scripts must live in a
#      directory shaped like .../VC/bin/ with include/lib/atlmfc/redist beside
#   3. lowercase-alias symlink farm across every vendor include/lib tree
#      (case-inconsistent vendor #includes; see lowercase_alias.py)
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
PREFIX="${NL_WINE_VS2008_PREFIX:-$HOME/toolchain_v90_prefix}"
SHADOW="${NL_WINE_VS2008_SHADOW:-$PREFIX-msvc-shadow}"
EXTERNAL="${NL_WINE_VS2008_EXTERNAL:-$PREFIX/drive_c/2019q4_external_v90_x86}"
PFX="$PREFIX/drive_c"
VC="$PFX/Program Files/Microsoft Visual Studio 9.0/VC"

[ -f "$VC/bin/cl.exe" ] || { echo "error: no VS2008 at $VC" >&2; exit 1; }

# 1. dosdevices
mkdir -p "$PREFIX/dosdevices"
ln -sfn ../drive_c "$PREFIX/dosdevices/c:"
ln -sfn / "$PREFIX/dosdevices/z:"

# 2. shadow VC dir with wrapper scripts standing in for the binaries
mkdir -p "$SHADOW/VC/bin"
ln -sfn "$VC/include" "$SHADOW/VC/include"
ln -sfn "$VC/lib" "$SHADOW/VC/lib"
ln -sfn "$VC/atlmfc" "$SHADOW/VC/atlmfc"
ln -sfn "$VC/redist" "$SHADOW/VC/redist"
cp "$HERE/winecl-env" "$SHADOW/VC/bin/winecl-env"
cp "$HERE/winecl-cc" "$SHADOW/VC/bin/cl.exe"
cp "$HERE/winecl-link" "$SHADOW/VC/bin/link.exe"
cp "$HERE/winecl-lib" "$SHADOW/VC/bin/lib.exe"
chmod +x "$SHADOW/VC/bin/cl.exe" "$SHADOW/VC/bin/link.exe" "$SHADOW/VC/bin/lib.exe"

# 2b. maxscript SDK patch: maxsdk/include/maxscrpt/value.h declares a global
#     'extern ScripterExport Empty empty;' that collides with VC's ivec.h
#     global empty() (C2365) in any TU that reaches both. Comment the extern
#     out, as the historically-working local SDK copy did; plugin code never
#     references the global.
MAXVALUE="$PFX/Program Files/Autodesk/3ds Max 2010 SDK/maxsdk/include/maxscrpt/value.h"
if [ -f "$MAXVALUE" ]; then
	sed -i 's|^extern ScripterExport Empty empty;|// extern ScripterExport Empty empty;|' "$MAXVALUE"
fi

# 3. lowercase-alias farm over every tree vendor code touches
for d in \
  "$PFX/Program Files/Autodesk/3ds Max 2010 SDK/maxsdk/include" \
  "$PFX/Program Files/Autodesk/3ds Max 2010 SDK/maxsdk/lib" \
  "$VC/include" \
  "$VC/lib" \
  "$VC/atlmfc" \
  "$PFX/Program Files/Microsoft SDKs/Windows/v6.0A/Include" \
  "$PFX/Program Files/Microsoft SDKs/Windows/v6.0A/Lib" \
  "$PFX/Program Files/Microsoft DirectX SDK (June 2010)/Include" \
  "$PFX/Program Files/Microsoft DirectX SDK (June 2010)/Lib" \
  "$EXTERNAL"
do
	[ -d "$d" ] && python3 "$HERE/lowercase_alias.py" "$d"
done

echo "prefix ready: $PREFIX"
echo "shadow ready: $SHADOW"
echo "configure with: cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=$HERE/toolchain.cmake ..."
