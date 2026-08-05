#!/bin/bash
# POST_BUILD manifest embedding for the NL_EMBED_SXS_MANIFEST_MT flow.
#   embed_manifest.sh <mt-tool> <binary> <resource-id>
# Embeds <binary>.manifest as RT_MANIFEST <resource-id> and removes the
# sidecar. A missing sidecar is success: newer CMake (3.31+ passes
# --msvc-ver to its vs_link tool) detects pre-VS2010 linkers and runs the
# classic mt pass itself, consuming the sidecar before this step runs.
set -e
MT="${1:?mt tool}"
BIN="${2:?binary}"
ID="${3:?resource id}"
[ -f "$BIN.manifest" ] || exit 0
"$MT" -nologo -manifest "$BIN.manifest" "-outputresource:$BIN;$ID"
rm -f "$BIN.manifest"
