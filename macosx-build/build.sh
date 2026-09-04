#!/bin/bash
# Configures and compiles the Ryzom client as a universal2 (x86_64 + arm64)
# macOS binary, targeting macOS 11.0. Entry point for the whole macOS build
# pipeline: makes sure the environment and third-party libs are ready, then
# builds ryzom_client itself.
#
# Usage: ./build.sh [fv|steam]   (defaults to fv)

set -e
set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
export EXTERNAL_PATH="${EXTERNAL_PATH:-${SCRIPT_DIR}/external}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu)}"

# Modern macOS SDKs don't ship system libs as real .dylib files on disk
# (they only live in the dyld shared cache) — linking against them needs
# the SDK's .tbd stub instead. Resolved dynamically rather than hard-coding
# an SDK version that would break on the next Xcode update.
SDK_PATH="$(xcrun --sdk macosx --show-sdk-path)"

# Force our own CMake (installed by setup-environment.sh) ahead of any
# system one (e.g. MacPorts' /opt/local/bin/cmake) — relying on the user's
# shell PATH being set up right is what caused the wrong cmake to be picked
# up before.
OWN_CMAKE_BIN="${SCRIPT_DIR}/cmake/CMake.app/Contents/bin"
if [ -x "${OWN_CMAKE_BIN}/cmake" ]; then
	export PATH="${OWN_CMAKE_BIN}:${PATH}"
fi

# Same for xcbeautify (installed by setup-environment.sh), if present.
OWN_XCBEAUTIFY_DIR="${SCRIPT_DIR}/xcbeautify/release"
if [ -x "${OWN_XCBEAUTIFY_DIR}/xcbeautify" ]; then
	export PATH="${OWN_XCBEAUTIFY_DIR}:${PATH}"
fi

# Same MacPorts/Homebrew shadowing issue as build-externals.sh: without
# this, CMake finds /opt/local's own (x86_64-only) zlib/libpng/libjpeg/...
# ahead of our universal2 static libs under EXTERNAL_PATH.
export PKG_CONFIG_LIBDIR=""
export PKG_CONFIG_PATH=""
CMAKE_SYSTEM_IGNORE_PATH="/opt/local;/opt/local/include;/opt/local/lib;/usr/local;/usr/local/include;/usr/local/lib;/sw;/opt"

RYZOM_CLIENT_TYPE="${1:-fv}"
case "${RYZOM_CLIENT_TYPE}" in
	fv)
		FINAL_VERSION=ON
		RYZOM_STEAM=OFF
		RYZOM_PATCH=ON
		;;
	steam)
		FINAL_VERSION=ON
		RYZOM_STEAM=ON
		RYZOM_PATCH=OFF
		# CMakeModules/FindSteam.cmake resolves the SDK purely via
		# $STEAM_DIR — same mechanism as ryzom-docker's Linux build script.
		# The SDK itself is Valve-licensed and can't be fetched here.
		if [ -z "${STEAM_DIR:-}" ] || { [ ! -f "${STEAM_DIR}/public/steam_api.h" ] && [ ! -f "${STEAM_DIR}/public/steam/steam_api.h" ]; }; then
			echo "ERROR: Steamworks SDK not found." >&2
			echo "       Set STEAM_DIR to a local Steamworks SDK checkout, e.g.:" >&2
			echo "       STEAM_DIR=/path/to/steamworks_sdk ./build.sh steam" >&2
			exit 1
		fi
		;;
	*)
		echo "ERROR: unknown build type '${RYZOM_CLIENT_TYPE}' (expected fv|steam)" >&2
		exit 1
		;;
esac

# Separate build dir per type: sharing one dir would let CMake's cached
# FIND_PATH/FIND_LIBRARY results (e.g. STEAM_INCLUDE_DIR) go stale across
# fv/steam reconfigures, silently keeping a wrong SDK path.
BUILD_DIR="${BUILD_DIR:-${SCRIPT_DIR}/build-${RYZOM_CLIENT_TYPE}}"

# DESCRIBE (branch/revision/commit build identifier), used by the client
# to display its version string. Mirrors ryzom-docker's client_linux/
# client_windows build scripts so the version string is consistent across
# platforms.
YEAR=$(date +%y)
MONTH=$(date +%m)
REVISION=$(cd "${REPO_ROOT}/ryzom/client/src" && git rev-list HEAD --count .)
COMMIT=$(cd "${REPO_ROOT}" && git rev-list --abbrev-commit HEAD -n 1)
BRANCH=$(cd "${REPO_ROOT}" && git rev-parse --abbrev-ref HEAD)
case "${BRANCH}" in
	main/yubo-dev)
		DOMAIN="Alpha /"
		;;
	main/gingo-test)
		DOMAIN="Beta /"
		;;
	*)
		DOMAIN="Omega /"
		;;
esac
DESCRIBE="${DOMAIN} v${YEAR}.${MONTH}.${REVISION} #${COMMIT}"

echo "=============================================="
echo " Ryzom macOS client build"
echo "=============================================="
echo " Type     : ${RYZOM_CLIENT_TYPE}"
echo " Jobs     : ${JOBS}"
echo " Describe : ${DESCRIBE}"
echo "=============================================="

if ! command -v cmake >/dev/null 2>&1; then
	echo "ERROR: cmake not found on PATH." >&2
	echo "       Run ./macosx-build/setup-environment.sh first, then export the PATH it prints." >&2
	exit 1
fi

if [ ! -f "${EXTERNAL_PATH}/lib/libz.a" ]; then
	echo ">>> External dependencies not found under ${EXTERNAL_PATH}, building them first..."
	"${SCRIPT_DIR}/build-externals.sh"
fi

mkdir -p "${BUILD_DIR}"

echo ">>> Configuring with CMake..."
cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" \
	-GXcode \
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
	-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
	-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
	-DCMAKE_SYSTEM_IGNORE_PATH="${CMAKE_SYSTEM_IGNORE_PATH}" \
	-DCMAKE_PREFIX_PATH="${EXTERNAL_PATH}" \
	-DWITH_RYZOM_CLIENT=ON \
	-DWITH_RYZOM_LIVE=ON \
	-DFINAL_VERSION=${FINAL_VERSION} \
	-DWITH_RYZOM_STEAM=${RYZOM_STEAM} \
	-DWITH_RYZOM_PATCH=${RYZOM_PATCH} \
	-DWITH_RYZOM_SERVER=OFF \
	-DWITH_RYZOM_TOOLS=OFF \
	-DWITH_NEL_TOOLS=OFF \
	-DWITH_NEL_TESTS=OFF \
	-DWITH_TESTING=OFF \
	-DWITH_NEL_SAMPLES=OFF \
	-DWITH_LUA51=OFF \
	-DWITH_LUA53=ON \
	-DLUA_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
	-DLUA_LIBRARY="${EXTERNAL_PATH}/lib/liblua.a" \
	-DZLIB_LIBRARY="${EXTERNAL_PATH}/lib/libz.a" \
	-DZLIB_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
	-DLIBXML2_LIBRARY="${EXTERNAL_PATH}/lib/libxml2.a" \
	-DLIBXML2_INCLUDE_DIR="${EXTERNAL_PATH}/include/libxml2" \
	-DPNG_LIBRARY="${EXTERNAL_PATH}/lib/libpng16.a" \
	-DPNG_PNG_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
	-DGIF_LIBRARY="${EXTERNAL_PATH}/lib/libgif.a" \
	-DGIF_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
	-DJPEG_LIBRARY="${EXTERNAL_PATH}/lib/libjpeg.a" \
	-DJPEG_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
	-DICONV_LIBRARIES="${SDK_PATH}/usr/lib/libiconv.tbd" \
	-DICONV_INCLUDE_DIR="${SDK_PATH}/usr/include" \
	-DOPENSSL_ROOT_DIR="${EXTERNAL_PATH}" \
	-DOPENSSL_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
	-DOPENSSL_CRYPTO_LIBRARY="${EXTERNAL_PATH}/lib/libcrypto.a" \
	-DOPENSSL_SSL_LIBRARY="${EXTERNAL_PATH}/lib/libssl.a" \
	-DOGG_DIR="${EXTERNAL_PATH}" \
	-DVORBIS_DIR="${EXTERNAL_PATH}" \
	-DVORBISFILE_DIR="${EXTERNAL_PATH}" \
	-DLUABIND_DIR="${EXTERNAL_PATH}" \
	-DCURL_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
	-DCURL_LIBRARY="${EXTERNAL_PATH}/lib/libcurl.a" \
	-DWITH_PCH=OFF \
	-DWITH_SYMBOLS=ON \
	-DWITH_UNIX_STRUCTURE=OFF \
	-DWITH_INSTALL_LIBRARIES=OFF \
	-DOpenGL_GL_PREFERENCE=LEGACY \
	-DWITH_SSE2=OFF \
	-DWITH_SSE3=OFF \
	-DWITH_STATIC=ON \
	-DWITH_STATIC_DRIVERS=ON \
	-DDESCRIBE="${DESCRIBE}" \
	-DCMAKE_C_FLAGS="-Wno-everything" \
	-DCMAKE_CXX_FLAGS="-Wno-everything" \
	-DCMAKE_XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS=YES \
	-DCMAKE_XCODE_ATTRIBUTE_ENABLE_USER_SCRIPT_SANDBOXING=NO

echo ">>> Building (this will take a while)..."
# xcbeautify only reformats output lines it recognizes; anything it doesn't
# match is silently dropped, which can hide the real error in a parallel
# build. The full untouched output is always kept in BUILD_LOG so a failure
# can be inspected regardless of what xcbeautify chose to show.
BUILD_LOG="${BUILD_DIR}/build.log"
if command -v xcbeautify >/dev/null 2>&1; then
	BUILD_OK=1
	cmake --build "${BUILD_DIR}" --config Release -- -jobs "${JOBS}" 2>&1 | tee "${BUILD_LOG}" | xcbeautify || BUILD_OK=0
else
	BUILD_OK=1
	cmake --build "${BUILD_DIR}" --config Release -- -jobs "${JOBS}" 2>&1 | tee "${BUILD_LOG}" || BUILD_OK=0
fi

if [ "${BUILD_OK}" -eq 0 ]; then
	{
		echo "========== LAST BUILD LINES =========="
		tail -n 100 "${BUILD_LOG}"
		echo "======================================"
		echo "Full raw log: ${BUILD_LOG}"
	} >&2
	exit 1
fi

APP_PATH="$(find "${BUILD_DIR}" -maxdepth 6 -type d -name "Ryzom.app" | head -1)"

echo "=============================================="
echo " Build complete!"
if [ -n "${APP_PATH}" ]; then
	echo " App: ${APP_PATH}"
else
	echo " Ryzom.app not found under ${BUILD_DIR} — check the build output above."
fi
echo "=============================================="
