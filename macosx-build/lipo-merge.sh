#!/bin/bash
# Merges a legacy VM x86_64/10.8 macOS build with the arm64/11.0 slice of the
# universal2 build produced by build.sh on a recent Mac, into one universal2
# binary. Runs on Linux — both inputs are already fully linked Mach-O
# executables, so this is a pure slice merge, no relinking, no codesign.
#
# Usage: ./lipo-merge.sh <x86_64_binary> <arm64_binary> <output_path>

set -e

if [ "$#" -ne 3 ]; then
	echo "Usage: $0 <x86_64_binary> <arm64_binary> <output_path>" >&2
	exit 1
fi

X86_64_BIN="$1"
ARM64_BIN="$2"
OUTPUT_PATH="$3"

if [ ! -f "${X86_64_BIN}" ]; then
	echo "ERROR: x86_64 binary not found: ${X86_64_BIN}" >&2
	exit 1
fi

if [ ! -f "${ARM64_BIN}" ]; then
	echo "ERROR: arm64 binary not found: ${ARM64_BIN}" >&2
	exit 1
fi

if command -v llvm-lipo >/dev/null 2>&1; then
	LIPO=llvm-lipo
elif command -v lipo >/dev/null 2>&1; then
	LIPO=lipo
else
	echo "ERROR: neither llvm-lipo nor lipo found on PATH." >&2
	echo "       Install LLVM's llvm-lipo, or use cctools-port/osxcross's lipo." >&2
	exit 1
fi

echo ">>> Merging with ${LIPO}..."
"${LIPO}" -create "${X86_64_BIN}" "${ARM64_BIN}" -output "${OUTPUT_PATH}"
chmod +x "${OUTPUT_PATH}"

echo "=============================================="
echo " Merge complete!"
echo " Output : ${OUTPUT_PATH}"
echo "=============================================="
"${LIPO}" -info "${OUTPUT_PATH}"
