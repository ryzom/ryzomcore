#!/bin/bash
# Checks/installs the toolchain needed to build the Ryzom client for macOS:
# Xcode Command Line Tools, Rosetta 2 (Apple Silicon only), CMake, git.
#
# Meant to be run once by hand, interactively — not a CI script: where a
# tool is missing, this triggers Apple's normal installer (a GUI popup for
# the Command Line Tools) rather than trying to force a silent/headless
# install.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CMAKE_DIR="${SCRIPT_DIR}/cmake"
CMAKE_MIN_VERSION="4.4.3"
XCBEAUTIFY_DIR="${SCRIPT_DIR}/xcbeautify"
XCBEAUTIFY_VERSION="3.2.1"

echo "=============================================="
echo " Ryzom macOS build environment setup"
echo "=============================================="

# --- Xcode Command Line Tools (clang, make, git, libtool, lipo) -----------
if xcode-select -p >/dev/null 2>&1; then
	echo "[ok] Xcode Command Line Tools: $(xcode-select -p)"
else
	echo "[..] Xcode Command Line Tools not found, launching the installer..."
	xcode-select --install
	echo "A system installer window should have opened."
	echo "Re-run this script once the Command Line Tools install finishes."
	exit 1
fi

# --- Rosetta 2 (Apple Silicon only) ----------------------------------------
# Needed to run x86_64 test binaries produced by autotools-based configure
# scripts (OpenSSL, Breakpad) during build-externals.sh, when building on
# arm64 hardware.
if [ "$(uname -m)" = "arm64" ]; then
	if /usr/bin/pgrep -q oahd 2>/dev/null; then
		echo "[ok] Rosetta 2 is installed"
	else
		echo "[..] Rosetta 2 not found, installing..."
		softwareupdate --install-rosetta --agree-to-license
	fi
else
	echo "[ok] Rosetta 2 not needed (running on ${arch:-$(uname -m)})"
fi

# --- CMake ------------------------------------------------------------
# Our own dedicated install is checked BEFORE the PATH one: a machine with
# MacPorts/Homebrew installed also has its own (often older) cmake on PATH,
# which would otherwise always win here and make this re-download ours on
# every run even when it's already present and current.
CMAKE_BIN=""
if [ -x "${CMAKE_DIR}/CMake.app/Contents/bin/cmake" ]; then
	CMAKE_BIN="${CMAKE_DIR}/CMake.app/Contents/bin/cmake"
elif command -v cmake >/dev/null 2>&1; then
	CMAKE_BIN="$(command -v cmake)"
fi

CMAKE_OK=0
if [ -n "${CMAKE_BIN}" ]; then
	CMAKE_FOUND_VERSION="$("${CMAKE_BIN}" --version | head -1 | awk '{print $3}')"
	if [ "$(printf '%s\n' "${CMAKE_MIN_VERSION}" "${CMAKE_FOUND_VERSION}" | sort -V | head -1)" = "${CMAKE_MIN_VERSION}" ]; then
		CMAKE_OK=1
	fi
fi

if [ "${CMAKE_OK}" = "1" ]; then
	echo "[ok] CMake ${CMAKE_FOUND_VERSION}: ${CMAKE_BIN}"
else
	echo "[..] CMake >= ${CMAKE_MIN_VERSION} not found, downloading Kitware's universal2 build..."
	rm -rf "${CMAKE_DIR}"
	mkdir -p "${CMAKE_DIR}"
	curl -sL "https://github.com/Kitware/CMake/releases/download/v${CMAKE_MIN_VERSION}/cmake-${CMAKE_MIN_VERSION}-macos-universal.tar.gz" \
		-o "${CMAKE_DIR}/cmake.tar.gz"
	tar xzf "${CMAKE_DIR}/cmake.tar.gz" -C "${CMAKE_DIR}" --strip-components=1
	rm "${CMAKE_DIR}/cmake.tar.gz"
	echo "[ok] CMake installed under ${CMAKE_DIR}/CMake.app"
	echo "     Add this to your PATH or export it before running build.sh:"
	echo "     export PATH=\"${CMAKE_DIR}/CMake.app/Contents/bin:\$PATH\""
fi

# --- xcbeautify (optional, prettifies build.sh's xcodebuild output) -------
# Same PATH-shadowing reasoning as CMake above: check our own install first.
# The release zip extracts the binary under a "release/" subdirectory, not
# at its root.
if [ -x "${XCBEAUTIFY_DIR}/release/xcbeautify" ]; then
	echo "[ok] xcbeautify: ${XCBEAUTIFY_DIR}/release/xcbeautify"
elif command -v xcbeautify >/dev/null 2>&1; then
	echo "[ok] xcbeautify: $(command -v xcbeautify)"
else
	echo "[..] xcbeautify not found, downloading the universal build..."
	rm -rf "${XCBEAUTIFY_DIR}"
	mkdir -p "${XCBEAUTIFY_DIR}"
	curl -sL "https://github.com/cpisciotta/xcbeautify/releases/download/${XCBEAUTIFY_VERSION}/xcbeautify-${XCBEAUTIFY_VERSION}-universal-apple-macosx.zip" \
		-o "${XCBEAUTIFY_DIR}/xcbeautify.zip"
	unzip -q "${XCBEAUTIFY_DIR}/xcbeautify.zip" -d "${XCBEAUTIFY_DIR}"
	rm "${XCBEAUTIFY_DIR}/xcbeautify.zip"
	chmod +x "${XCBEAUTIFY_DIR}/release/xcbeautify"
	echo "[ok] xcbeautify installed under ${XCBEAUTIFY_DIR}/release"
fi

# --- git ------------------------------------------------------------------
if command -v git >/dev/null 2>&1; then
	echo "[ok] git: $(command -v git)"
else
	echo "[!!] git not found — it should have been installed with the Command Line Tools above."
	exit 1
fi

echo "=============================================="
echo " Environment ready."
echo "=============================================="
