#!/bin/bash
# Builds every third-party dependency needed by the Ryzom client as a
# universal2 (x86_64 + arm64) static lib, installed under EXTERNAL_PATH.
# Mirrors ryzom-docker's client_linux/client_windows Dockerfiles (same
# libs/versions), adapted for native macOS builds via clang's multi-arch
# support instead of Docker.
#
# Safe to re-run: each dependency is skipped if already built (see
# is_built/mark_built below), so only missing/new libs get compiled.
#
# Requires Xcode Command Line Tools (clang, make, libtool, lipo), CMake and
# git on PATH.

set -e

# Neutralizes pkg-config for this whole script: on a machine with
# MacPorts/Homebrew installed, its pkg-config binary (found first on PATH)
# has its own hard-coded default search path baked in — CMAKE_SYSTEM_IGNORE_PATH
# has no effect on that, since it's a separate process CMake just shells out
# to. Emptying these forces pkg-config to report nothing found anywhere, so
# every dependency falls back to our own explicit -D hints instead.
export PKG_CONFIG_LIBDIR=""
export PKG_CONFIG_PATH=""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXTERNAL_PATH="${EXTERNAL_PATH:-${SCRIPT_DIR}/external}"
BUILD_TMP="${SCRIPT_DIR}/tmp"
JOBS="${JOBS:-$(sysctl -n hw.ncpu)}"

# clang reads this as an implicit -mmacosx-version-min for every invocation
# below (direct clang calls, autotools/make, and CMake's own default for
# CMAKE_OSX_DEPLOYMENT_TARGET) — without it, object files get built against
# the host's own macOS version (e.g. 15.7) instead of the 11.0 floor this
# whole universal2 build targets.
export MACOSX_DEPLOYMENT_TARGET="11.0"

# Force our own CMake (installed by setup-environment.sh) ahead of any
# system one (e.g. MacPorts' /opt/local/bin/cmake) — relying on the user's
# shell PATH being set up right is what caused the wrong cmake to be picked
# up before.
OWN_CMAKE_BIN="${SCRIPT_DIR}/cmake/CMake.app/Contents/bin"
if [ -x "${OWN_CMAKE_BIN}/cmake" ]; then
	export PATH="${OWN_CMAKE_BIN}:${PATH}"
fi
OSX_ARCHITECTURES="x86_64;arm64"

ZLIB_VERSION=v1.3.1
LIBXML2_VERSION=v2.12.6
LIBPNG_VERSION=v1.6.43
OPENSSL_VERSION=openssl-3.2.1
CURL_VERSION=curl-8_9_1
LUA_VERSION=5.3.6
BOOST_VERSION=1.74.0
FREETYPE_VERSION=VER-2-12-1
JPEG_VERSION=3.0.4
GIFLIB_VERSION=5.2.2
OGG_VERSION=v1.3.5
VORBIS_VERSION=v1.3.7
BREAKPAD_COMMIT=b988fa74ec18de6214b18f723e48331d9a7802ae

mkdir -p "${EXTERNAL_PATH}/include" "${EXTERNAL_PATH}/lib" "${EXTERNAL_PATH}/bin" "${BUILD_TMP}"

# A dependency is considered built once its marker file exists under
# EXTERNAL_PATH/.built/<name> — lets a build be forced again by just
# deleting that one marker, without guessing which output files a given
# lib produces.
mark_built()
{
	local name="$1"
	mkdir -p "${EXTERNAL_PATH}/.built"
	touch "${EXTERNAL_PATH}/.built/${name}"
}

is_built()
{
	local name="$1"
	[ -f "${EXTERNAL_PATH}/.built/${name}" ]
}

fetch_src()
{
	local repo="$1" tag="$2" dest="$3"
	rm -rf "$dest"
	git clone --depth 1 --branch "$tag" "$repo" "$dest"
}

# Builds+installs a CMake-based dependency as a universal2 static lib in one
# pass — clang natively accepts multiple -arch flags, unlike MSVC, so no
# per-arch build + lipo is needed here.
# CMAKE_SYSTEM_IGNORE_PATH: some find_package modules (FindOpenSSL.cmake in
# particular) run their own internal find_path()/find_library() probes that
# don't honor an already-set *_INCLUDE_DIR/*_LIBRARY cache variable as a
# hint — on a machine with MacPorts/Homebrew installed, that probe can find
# an old/incompatible system lib (e.g. MacPorts' OpenSSL 1.0.2, missing
# BIO_up_ref/SSL_set0_rbio/wbio) ahead of ours, silently overriding it.
# Ignoring these prefixes outright for every find_* call is the only
# reliable way to force everything through EXTERNAL_PATH.
build_cmake_dep()
{
	local src_dir="$1" build_dir="$2"
	shift 2
	cmake -S "$src_dir" -B "$build_dir" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		-DCMAKE_OSX_ARCHITECTURES="${OSX_ARCHITECTURES}" \
		-DCMAKE_INSTALL_PREFIX="${EXTERNAL_PATH}" \
		-DCMAKE_PREFIX_PATH="${EXTERNAL_PATH}" \
		-DCMAKE_SYSTEM_IGNORE_PATH="/opt/local;/opt/local/include;/opt/local/lib;/usr/local;/usr/local/include;/usr/local/lib;/sw;/opt" \
		-DZLIB_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
		-DZLIB_LIBRARY="${EXTERNAL_PATH}/lib/libz.a" \
		-DCMAKE_C_FLAGS="-I${EXTERNAL_PATH}/include" \
		-DCMAKE_CXX_FLAGS="-I${EXTERNAL_PATH}/include" \
		-DBUILD_SHARED_LIBS=OFF \
		"$@"
	cmake --build "$build_dir" -j"${JOBS}"
	cmake --install "$build_dir"
}

echo "=============================================="
echo " Ryzom macOS externals build (universal2)"
echo "=============================================="
echo " External prefix : ${EXTERNAL_PATH}"
echo " Jobs             : ${JOBS}"
echo "=============================================="

# --- zlib -------------------------------------------------------------
if ! is_built zlib; then
	fetch_src https://github.com/madler/zlib.git "${ZLIB_VERSION}" "${BUILD_TMP}/zlib"
	build_cmake_dep "${BUILD_TMP}/zlib" "${BUILD_TMP}/zlib/build"
	mark_built zlib
fi

# --- libxml2 ------------------------------------------------------------
if ! is_built libxml2; then
	fetch_src https://gitlab.gnome.org/GNOME/libxml2.git "${LIBXML2_VERSION}" "${BUILD_TMP}/libxml2"
	build_cmake_dep "${BUILD_TMP}/libxml2" "${BUILD_TMP}/libxml2/build" \
		-DLIBXML2_WITH_ICONV=OFF \
		-DLIBXML2_WITH_LZMA=OFF \
		-DLIBXML2_WITH_PYTHON=OFF \
		-DLIBXML2_WITH_PROGRAMS=OFF \
		-DLIBXML2_WITH_TESTS=OFF
	mark_built libxml2
fi

# --- libpng ---------------------------------------------------------------
# PNG_ARM_NEON=off only stops libpng's CMakeLists.txt from adding its NEON
# *source* files (gated by its own TARGET_ARCH, itself derived from the host
# CPU, not each -arch slice) — it does NOT stop pngpriv.h's separate,
# independent auto-detection of PNG_ARM_NEON_OPT (based on the compiler's
# __ARM_NEON__ macro, which clang legitimately defines while compiling the
# arm64 slice). So the generic .c files still call the NEON functions, which
# never got compiled in: "Undefined symbols ... _png_do_expand_palette_rgb8_neon".
# The real fix is forcing PNG_ARM_NEON_OPT=0 as a preprocessor define
# (-DPNG_ARM_NEON_OPT=0 in CMAKE_C_FLAGS/CXX_FLAGS), which pngpriv.h honors
# directly and skips its own detection for entirely.
# PNG_FRAMEWORK=OFF: defaults ON on Apple independently of PNG_SHARED, building
# a macOS .framework bundle we don't need (only the static lib does).
if ! is_built libpng; then
	fetch_src https://github.com/pnggroup/libpng.git "${LIBPNG_VERSION}" "${BUILD_TMP}/libpng"
	build_cmake_dep "${BUILD_TMP}/libpng" "${BUILD_TMP}/libpng/build" \
		-DPNG_SHARED=OFF \
		-DPNG_STATIC=ON \
		-DPNG_TESTS=OFF \
		-DPNG_TOOLS=OFF \
		-DPNG_FRAMEWORK=OFF \
		-DPNG_ARM_NEON=off \
		-DCMAKE_C_FLAGS="-I${EXTERNAL_PATH}/include -DPNG_ARM_NEON_OPT=0" \
		-DCMAKE_CXX_FLAGS="-I${EXTERNAL_PATH}/include -DPNG_ARM_NEON_OPT=0"
	mark_built libpng
fi

# --- OpenSSL ----------------------------------------------------------
# No CMake / no multi-arch Configure target — built once per arch, then
# merged into a single fat .a with lipo.
if ! is_built openssl; then
	fetch_src https://github.com/openssl/openssl.git "${OPENSSL_VERSION}" "${BUILD_TMP}/openssl-x86_64"
	cp -r "${BUILD_TMP}/openssl-x86_64" "${BUILD_TMP}/openssl-arm64"

	(
		cd "${BUILD_TMP}/openssl-x86_64"
		./Configure darwin64-x86_64-cc no-shared no-tests no-apps \
			--prefix="${BUILD_TMP}/openssl-install-x86_64"
		make -j"${JOBS}"
		make install_sw
	)
	(
		cd "${BUILD_TMP}/openssl-arm64"
		./Configure darwin64-arm64-cc no-shared no-tests no-apps \
			--prefix="${BUILD_TMP}/openssl-install-arm64"
		make -j"${JOBS}"
		make install_sw
	)

	lipo -create \
		"${BUILD_TMP}/openssl-install-x86_64/lib/libssl.a" \
		"${BUILD_TMP}/openssl-install-arm64/lib/libssl.a" \
		-output "${EXTERNAL_PATH}/lib/libssl.a"
	lipo -create \
		"${BUILD_TMP}/openssl-install-x86_64/lib/libcrypto.a" \
		"${BUILD_TMP}/openssl-install-arm64/lib/libcrypto.a" \
		-output "${EXTERNAL_PATH}/lib/libcrypto.a"
	cp -r "${BUILD_TMP}/openssl-install-x86_64/include/." "${EXTERNAL_PATH}/include/"
	mark_built openssl
fi

# --- CURL ---------------------------------------------------------------
# CMAKE_INCLUDE_DIRECTORIES_BEFORE=ON: CMake's default macOS system search
# path includes /opt/local (MacPorts) and /usr/local (Homebrew) ahead of our
# own -D hints — if either has an old OpenSSL installed (e.g. MacPorts'
# pre-1.1.0), its headers shadow ours and curl fails to compile against
# newer OpenSSL APIs (BIO_up_ref, SSL_set0_rbio/wbio, ...).
if ! is_built curl; then
	fetch_src https://github.com/curl/curl.git "${CURL_VERSION}" "${BUILD_TMP}/curl"
	build_cmake_dep "${BUILD_TMP}/curl" "${BUILD_TMP}/curl/build" \
		-DCMAKE_INCLUDE_DIRECTORIES_BEFORE=ON \
		-DBUILD_CURL_EXE=OFF \
		-DBUILD_TESTING=OFF \
		-DBUILD_LIBCURL_DOCS=OFF \
		-DBUILD_MISC_DOCS=OFF \
		-DCURL_USE_OPENSSL=ON \
		-DOPENSSL_ROOT_DIR="${EXTERNAL_PATH}" \
		-DOPENSSL_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
		-DOPENSSL_CRYPTO_LIBRARY="${EXTERNAL_PATH}/lib/libcrypto.a" \
		-DOPENSSL_SSL_LIBRARY="${EXTERNAL_PATH}/lib/libssl.a" \
		-DCURL_ZLIB=ON \
		-DCURL_USE_LIBPSL=OFF \
		-DCURL_USE_LIBSSH2=OFF \
		-DCURL_BROTLI=OFF \
		-DCURL_ZSTD=OFF \
		-DUSE_NGHTTP2=OFF \
		-DCURL_DISABLE_LDAP=ON \
		-DCURL_DISABLE_LDAPS=ON
	mark_built curl
fi

# --- Lua 5.3 ------------------------------------------------------------
# No CMake — compiled directly with clang, which (unlike cl.exe) accepts
# multiple -arch flags in one invocation, so the fat .o/.a comes out of a
# single pass.
if ! is_built lua; then
	rm -rf "${BUILD_TMP}/lua"
	mkdir -p "${BUILD_TMP}/lua"
	curl -sL "https://www.lua.org/ftp/lua-${LUA_VERSION}.tar.gz" -o "${BUILD_TMP}/lua.tar.gz"
	tar xzf "${BUILD_TMP}/lua.tar.gz" -C "${BUILD_TMP}/lua" --strip-components=1
	(
		cd "${BUILD_TMP}/lua/src"
		clang -O2 -arch x86_64 -arch arm64 -DLUA_USE_MACOSX -c ./*.c
		rm -f lua.o luac.o
		libtool -static -o liblua.a ./*.o
	)
	cp "${BUILD_TMP}/lua/src/liblua.a" "${EXTERNAL_PATH}/lib/"
	cp "${BUILD_TMP}/lua/src/"lua.h "${BUILD_TMP}/lua/src/"luaconf.h "${BUILD_TMP}/lua/src/"lualib.h "${BUILD_TMP}/lua/src/"lauxlib.h "${BUILD_TMP}/lua/src/"lua.hpp "${EXTERNAL_PATH}/include/"
	mark_built lua
fi

# --- Boost (headers only) ------------------------------------------------
if ! is_built boost; then
	curl -sL "https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION//./_}.tar.gz" -o "${BUILD_TMP}/boost.tar.gz"
	rm -rf "${BUILD_TMP}/boost" "${EXTERNAL_PATH}/include/boost"
	mkdir -p "${BUILD_TMP}/boost"
	tar xzf "${BUILD_TMP}/boost.tar.gz" -C "${BUILD_TMP}/boost" --strip-components=1
	mv "${BUILD_TMP}/boost/boost" "${EXTERNAL_PATH}/include/boost"
	mark_built boost
fi

# --- Luabind --------------------------------------------------------------
# No CMake install rule of its own (see the "luabind" target-only build
# below) — headers and lib are copied manually, same as the Linux/Windows
# Dockerfiles. Output is renamed to luabind_lua53.lib, the name
# CMakeModules/FindLuabind.cmake looks for when WITH_LUA53=ON.
if ! is_built luabind; then
	fetch_src https://github.com/nimetu/luabind.git master "${BUILD_TMP}/luabind"
	cmake -S "${BUILD_TMP}/luabind" -B "${BUILD_TMP}/luabind/build" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		-DCMAKE_OSX_ARCHITECTURES="${OSX_ARCHITECTURES}" \
		-DBoost_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
		-DLUA_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
		-DLUA_LIBRARIES="${EXTERNAL_PATH}/lib/liblua.a"
	cmake --build "${BUILD_TMP}/luabind/build" --target luabind -j"${JOBS}"
	mkdir -p "${EXTERNAL_PATH}/include/luabind"
	cp -r "${BUILD_TMP}/luabind/luabind/." "${EXTERNAL_PATH}/include/luabind/"
	find "${BUILD_TMP}/luabind/build" -iname "build_information.hpp" -exec cp {} "${EXTERNAL_PATH}/include/luabind/" \;
	find "${BUILD_TMP}/luabind/build" -iname "libluabind*.a" -exec cp {} "${EXTERNAL_PATH}/lib/libluabind_lua53.a" \;
	mark_built luabind
fi

# --- FreeType -------------------------------------------------------------
if ! is_built freetype; then
	fetch_src https://gitlab.freedesktop.org/freetype/freetype.git "${FREETYPE_VERSION}" "${BUILD_TMP}/freetype"
	build_cmake_dep "${BUILD_TMP}/freetype" "${BUILD_TMP}/freetype/build" \
		-DFT_DISABLE_ZLIB=OFF \
		-DFT_DISABLE_BZIP2=ON \
		-DFT_DISABLE_PNG=ON \
		-DFT_DISABLE_HARFBUZZ=ON \
		-DFT_DISABLE_BROTLI=ON
	mark_built freetype
fi

# --- libjpeg-turbo --------------------------------------------------------
# libjpeg-turbo's own CMakeLists.txt hard-refuses more than one value in
# CMAKE_OSX_ARCHITECTURES ("contains assembly code") unconditionally, before
# WITH_SIMD is even read — so unlike the other CMake deps above, this one
# can't be built fat in a single pass regardless of SIMD settings. Built
# once per arch (SIMD still off: x86_64 SIMD needs a Windows/Linux-style
# nasm/yasm toolchain this script doesn't set up) then merged with lipo,
# same as OpenSSL/Breakpad.
if ! is_built jpeg; then
	fetch_src https://github.com/libjpeg-turbo/libjpeg-turbo.git "${JPEG_VERSION}" "${BUILD_TMP}/jpeg"
	for arch in x86_64 arm64; do
		cmake -S "${BUILD_TMP}/jpeg" -B "${BUILD_TMP}/jpeg/build-${arch}" \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
			-DCMAKE_OSX_ARCHITECTURES="${arch}" \
			-DCMAKE_INSTALL_PREFIX="${BUILD_TMP}/jpeg/install-${arch}" \
			-DCMAKE_SYSTEM_IGNORE_PATH="/opt/local;/opt/local/include;/opt/local/lib;/usr/local;/usr/local/include;/usr/local/lib;/sw;/opt" \
			-DENABLE_SHARED=OFF \
			-DENABLE_STATIC=ON \
			-DWITH_SIMD=OFF \
			-DWITH_TURBOJPEG=OFF \
			-DWITH_JAVA=OFF
		cmake --build "${BUILD_TMP}/jpeg/build-${arch}" -j"${JOBS}"
		cmake --install "${BUILD_TMP}/jpeg/build-${arch}"
	done
	lipo -create \
		"${BUILD_TMP}/jpeg/install-x86_64/lib/libjpeg.a" \
		"${BUILD_TMP}/jpeg/install-arm64/lib/libjpeg.a" \
		-output "${EXTERNAL_PATH}/lib/libjpeg.a"
	cp -r "${BUILD_TMP}/jpeg/install-x86_64/include/." "${EXTERNAL_PATH}/include/"
	mark_built jpeg
fi

# --- giflib -----------------------------------------------------------
# No CMake (nor any Makefile beyond a Unix-only one) — same direct-clang
# approach as Lua above. Only the library's own 8 sources are compiled, not
# the CLI utilities (gifsponge, giftool, ...) that live alongside them.
if ! is_built giflib; then
	fetch_src https://git.code.sf.net/p/giflib/code "${GIFLIB_VERSION}" "${BUILD_TMP}/giflib"
	(
		cd "${BUILD_TMP}/giflib"
		clang -O2 -arch x86_64 -arch arm64 -c dgif_lib.c egif_lib.c gifalloc.c gif_err.c gif_font.c gif_hash.c openbsd-reallocarray.c quantize.c
		libtool -static -o libgif.a ./*.o
	)
	cp "${BUILD_TMP}/giflib/libgif.a" "${EXTERNAL_PATH}/lib/"
	cp "${BUILD_TMP}/giflib/gif_lib.h" "${EXTERNAL_PATH}/include/"
	mark_built giflib
fi

# --- libogg -----------------------------------------------------------
if ! is_built ogg; then
	fetch_src https://github.com/xiph/ogg.git "${OGG_VERSION}" "${BUILD_TMP}/ogg"
	build_cmake_dep "${BUILD_TMP}/ogg" "${BUILD_TMP}/ogg/build" \
		-DINSTALL_DOCS=OFF \
		-DINSTALL_PKG_CONFIG_MODULE=OFF \
		-DINSTALL_CMAKE_PACKAGE_MODULE=OFF
	mark_built ogg
fi

# --- libvorbis --------------------------------------------------------
if ! is_built vorbis; then
	fetch_src https://github.com/xiph/vorbis.git "${VORBIS_VERSION}" "${BUILD_TMP}/vorbis"
	build_cmake_dep "${BUILD_TMP}/vorbis" "${BUILD_TMP}/vorbis/build" \
		-DOGG_INCLUDE_DIR="${EXTERNAL_PATH}/include" \
		-DOGG_LIBRARY="${EXTERNAL_PATH}/lib/libogg.a" \
		-DINSTALL_CMAKE_PACKAGE_MODULE=OFF \
		-DINSTALL_PKG_CONFIG_MODULE=OFF
	mark_built vorbis
fi

# --- Google Breakpad ------------------------------------------------------
# Not currently linked by the macOS build (see nel/src/misc/debug.cpp and
# nel/src/misc/CMakeLists.txt's empty IF(APPLE) branch) — compiled anyway in
# anticipation of that support being added later. Autotools' configure runs
# compiled test programs during its checks, which is unreliable against a
# single fat multi-arch binary, so built once per arch and merged with lipo,
# same as OpenSSL above.
#
# Only libbreakpad.a comes out of this generic `./configure && make install`
# on Darwin — unlike Linux, it doesn't also produce libbreakpad_client.a:
# that one comes from client/linux/ on Linux, while the macOS equivalent
# (client/mac/, an Objective-C++ exception handler) is built through
# Breakpad's own separate Xcode project, not this top-level Makefile. Adding
# real macOS crash-handler support later means building that piece
# separately, not through this block.
if ! is_built breakpad; then
	rm -rf "${BUILD_TMP}/breakpad"
	git clone https://chromium.googlesource.com/breakpad/breakpad.git "${BUILD_TMP}/breakpad"
	(cd "${BUILD_TMP}/breakpad" && git checkout "${BREAKPAD_COMMIT}")
	git clone --depth 1 https://chromium.googlesource.com/linux-syscall-support "${BUILD_TMP}/breakpad/src/third_party/lss"

	# autotools' configure runs a compiled test program to probe the
	# compiler — for the arch that isn't the host machine's own, that test
	# binary can't execute locally (there's no arm64-on-x86_64 Rosetta
	# direction), so configure must be told explicitly it's cross-compiling
	# via --host/--build, or it aborts with "cannot run C compiled programs".
	HOST_ARCH="$(uname -m)"
	HOST_GNU_ARCH="${HOST_ARCH}"
	[ "${HOST_ARCH}" = "arm64" ] && HOST_GNU_ARCH="aarch64"

	for arch in x86_64 arm64; do
		rm -rf "${BUILD_TMP}/breakpad-${arch}"
		cp -r "${BUILD_TMP}/breakpad" "${BUILD_TMP}/breakpad-${arch}"
		gnu_arch="${arch}"
		[ "${arch}" = "arm64" ] && gnu_arch="aarch64"
		configure_host_flags=()
		if [ "${arch}" != "${HOST_ARCH}" ]; then
			configure_host_flags=(--host="${gnu_arch}-apple-darwin" --build="${HOST_GNU_ARCH}-apple-darwin")
		fi
		(
			cd "${BUILD_TMP}/breakpad-${arch}"
			CFLAGS="-arch ${arch}" CXXFLAGS="-arch ${arch}" LDFLAGS="-arch ${arch}" \
				./configure --prefix="${BUILD_TMP}/breakpad-install-${arch}" "${configure_host_flags[@]}"
			make -j"${JOBS}"
			make install
		)
	done

	mkdir -p "${EXTERNAL_PATH}/include/breakpad"
	lipo -create \
		"${BUILD_TMP}/breakpad-install-x86_64/lib/libbreakpad.a" \
		"${BUILD_TMP}/breakpad-install-arm64/lib/libbreakpad.a" \
		-output "${EXTERNAL_PATH}/lib/libbreakpad.a"
	cp -r "${BUILD_TMP}/breakpad-install-x86_64/include/." "${EXTERNAL_PATH}/include/"
	mark_built breakpad
fi

rm -rf "${BUILD_TMP}"

echo "=============================================="
echo " Done."
echo "=============================================="
