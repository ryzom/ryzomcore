#!/bin/bash
# Package the Max 2010 x86 plugin set in the expected install arrangement
# (mirrors tool/quick_start/win32/install/redist/nel_plugin_max_2010_x86.bat,
# minus code signing — sign separately where a certificate is available):
#
#   nel_plugin_max_2010_x86/
#     *.dll               support DLLs (nel + externals), loaded from Max root
#     plugins/            renamed plugin binaries + their .cfg files
#     scripts/            maxscripts (+ scripts/startup/)
#     macroscripts/       macroscripts
#     *.txt, object_viewer.cfg
#
# usage: package_max_plugin.sh <build-bin-dir> <externals-root> <out-dir>
set -e
BIN="${1:?build bin dir}"
EXT="${2:?externals root}"
OUT="${3:?output dir}/nel_plugin_max_2010_x86"
SRC="$(cd "$(dirname "$0")/../.." && pwd)"
PMAX="$SRC/nel/tools/3d/plugin_max"

rm -rf "$OUT"
mkdir -p "$OUT/plugins" "$OUT/scripts/startup" "$OUT/macroscripts"

# support DLLs from the build
for f in object_viewer_dll_r.dll nel_3dsmax_shared_r.dll nel_drv_opengl_win_r.dll nel_drv_xaudio2_win_r.dll; do
	cp "$BIN/$f" "$OUT/"
done
# support DLLs from the externals archive
cp "$EXT/ogg/bin/ogg.dll" "$OUT/"
cp "$EXT/libpng/bin/libpng16.dll" "$OUT/"
cp "$EXT/vorbis/bin/vorbis.dll" "$OUT/"
cp "$EXT/vorbis/bin/vorbisfile.dll" "$OUT/"
cp "$EXT/libjpeg/bin/jpeg62.dll" "$OUT/"
cp "$EXT/libxml2/bin/libxml2.dll" "$OUT/"
cp "$EXT/libiconv/bin/libiconv.dll" "$OUT/"
cp "$EXT/libiconv/bin/libcharset.dll" "$OUT/"
cp "$EXT/zlib/bin/zlib.dll" "$OUT/"
cp "$EXT/freetype/bin/freetype.dll" "$OUT/"

# plugin binaries, renamed per the historical install convention
cp "$BIN/nel_patch_converter_r.dlm" "$OUT/plugins/nelconvertpatch_r.dlm"
cp "$BIN/nel_export_r.dlu" "$OUT/plugins/nelexport_r.dlu"
cp "$BIN/nel_patch_paint_r.dlm" "$OUT/plugins/nelpaintpatch_r.dlm"
cp "$BIN/nel_patch_edit_r.dlm" "$OUT/plugins/neleditpatch_r.dlm"
cp "$BIN/tile_utility_r.dlu" "$OUT/plugins/neltileutility_r.dlu"
cp "$BIN/nel_vertex_tree_paint_r.dlm" "$OUT/plugins/nel_vertex_tree_paint_r.dlm"
cp "$BIN/ligoscape_utility_r.dlx" "$OUT/plugins/nelligoscapeutility_r.dlx"
cp "$BIN/mapext198m3.dlm" "$OUT/plugins/mapext198m3.dlm"
cp "$PMAX/nel_patch_paint/keys.cfg" "$OUT/plugins/keys.cfg"
cp "$SRC/nel/tools/3d/ligo/ligoscape.cfg" "$OUT/plugins/ligoscape.cfg"

# root docs and viewer config
cp "$PMAX"/max_*_support.txt "$OUT/"
cp "$PMAX/nel_water_material.txt" "$OUT/"
cp "$PMAX/resolve_troubles.txt" "$OUT/"
cp "$SRC/nel/tools/3d/object_viewer/object_viewer.cfg" "$OUT/"

# maxscripts
cp "$PMAX"/scripts/*.ms "$OUT/scripts/"
find "$PMAX/scripts/startup" -maxdepth 1 -type f -exec cp {} "$OUT/scripts/startup/" \;

# macroscripts
cp "$PMAX"/macroscripts/*.mcr "$OUT/macroscripts/"
cp "$SRC/nel/tools/3d/ligo/plugin_max/macroscripts/nel_ligoscape.mcr" "$OUT/macroscripts/"

echo "packaged: $OUT"
find "$OUT" -type f | wc -l
