#!/usr/bin/env bash
# Build the wasm dependency prefix nl_sample_gui needs (lua5.1 + luabind +
# libxml2, plus boost/openssl/curl HEADERS and nel_wasm_stubs archived as
# libcurl.a/libssl.a/libcrypto.a). Mirrors what ships in ~/nelgui_deps/wasm
# on the dev box. Idempotent: safe to re-run when the actions/cache misses.
#
# Args:
#   $1  destination prefix (created if absent). Default: $PWD/nelgui_deps/wasm
#   $2  path to ryzomcore source root (needs nel/samples/gui/emscripten/
#       nel_wasm_stubs.c). Default: $PWD
#
# emsdk must already be sourced (emcc/emcmake/emar on PATH).

set -euo pipefail

PREFIX="${1:-$PWD/nelgui_deps/wasm}"
SRC_ROOT="${2:-$PWD}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# Pinned versions — mirror the dev-box prefix.
LUA_VER=5.1.5
LIBXML2_VER=2.12.10
BOOST_VER=1.83.0
BOOST_VER_UND="${BOOST_VER//./_}"
OPENSSL_VER=3.0.13
CURL_VER=8.5.0
LUABIND_REV=0ae9bd6   # ryzom/luabind

mkdir -p "$PREFIX/lib" "$PREFIX/include"

echo "== emcc" && emcc --version | head -1

# ---- lua 5.1 ----------------------------------------------------------------
if [ ! -f "$PREFIX/lib/liblua.a" ]; then
	echo "== lua $LUA_VER"
	cd "$WORK"
	curl -L --fail --retry 3 -O "https://www.lua.org/ftp/lua-${LUA_VER}.tar.gz"
	tar xf "lua-${LUA_VER}.tar.gz"
	cd "lua-${LUA_VER}/src"
	# Build the static core + libs with emcc. Skip lua/luac front-ends.
	LUA_SRC="lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c \
	         lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c \
	         ltable.c ltm.c lundump.c lvm.c lzio.c \
	         lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c \
	         ltablib.c lstrlib.c loadlib.c linit.c"
	for f in $LUA_SRC; do emcc -O2 -c "$f" -o "${f%.c}.o"; done
	emar rcs liblua.a *.o
	cp liblua.a "$PREFIX/lib/"
	cp lua.h lualib.h lauxlib.h luaconf.h "$PREFIX/include/"
	# lua.hpp is a C++ convenience header shipped with the tree
	cp ../etc/lua.hpp "$PREFIX/include/" 2>/dev/null || echo '#include "lua.h"' > "$PREFIX/include/lua.hpp"
fi

# ---- libxml2 ---------------------------------------------------------------
if [ ! -f "$PREFIX/lib/libxml2.a" ]; then
	echo "== libxml2 $LIBXML2_VER"
	cd "$WORK"
	curl -L --fail --retry 3 -O "https://download.gnome.org/sources/libxml2/${LIBXML2_VER%.*}/libxml2-${LIBXML2_VER}.tar.xz"
	tar xf "libxml2-${LIBXML2_VER}.tar.xz"
	mkdir -p build-libxml2 && cd build-libxml2
	emcmake cmake "../libxml2-${LIBXML2_VER}" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$PREFIX" \
		-DBUILD_SHARED_LIBS=OFF \
		-DLIBXML2_WITH_ICONV=OFF \
		-DLIBXML2_WITH_ICU=OFF \
		-DLIBXML2_WITH_LZMA=OFF \
		-DLIBXML2_WITH_PYTHON=OFF \
		-DLIBXML2_WITH_ZLIB=OFF \
		-DLIBXML2_WITH_TESTS=OFF \
		-DLIBXML2_WITH_PROGRAMS=OFF \
		-DLIBXML2_WITH_FTP=OFF \
		-DLIBXML2_WITH_LEGACY=OFF
	emmake make -j"$(nproc)" install
fi

# ---- boost headers (headers-only usage) ------------------------------------
if [ ! -d "$PREFIX/../boost-headers/boost" ]; then
	echo "== boost $BOOST_VER (headers only)"
	cd "$WORK"
	curl -L --fail --retry 3 -o "boost.tar.gz" \
		"https://archives.boost.io/release/${BOOST_VER}/source/boost_${BOOST_VER_UND}.tar.gz"
	tar xf boost.tar.gz
	mkdir -p "$PREFIX/../boost-headers"
	mv "boost_${BOOST_VER_UND}/boost" "$PREFIX/../boost-headers/boost"
fi

# ---- luabind (ryzom fork) --------------------------------------------------
if [ ! -f "$PREFIX/lib/libluabind09.a" ]; then
	echo "== luabind ryzom/luabind@$LUABIND_REV"
	cd "$WORK"
	git clone --depth 32 https://github.com/ryzom/luabind.git luabind
	cd luabind && git checkout "$LUABIND_REV" && cd ..
	mkdir -p build-luabind && cd build-luabind
	# luabind's CMakeLists calls find_package(Lua). Emscripten's toolchain
	# forces CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY, so CMake's stock FindLua
	# rejects the cache path we hand it (it's outside the emsdk sysroot),
	# find_library returns NOTFOUND, and the module reports missing
	# LUA_LIBRARIES even though we set it explicitly. Patch the call out —
	# LuaBind then consumes LUA_INCLUDE_DIR / LUA_LIBRARIES straight from
	# our cache values.
	sed -i 's|find_package(Lua)|set(LUA_FOUND TRUE)|' ../luabind/CMakeLists.txt
	emcmake cmake ../luabind \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$PREFIX" \
		-DBUILD_SHARED_LIBS=OFF \
		-DBoost_INCLUDE_DIR="$PREFIX/../boost-headers" \
		-DLUA_INCLUDE_DIR:PATH="$PREFIX/include" \
		-DLUA_INCLUDE_DIRS:PATH="$PREFIX/include" \
		-DLUA_LIBRARIES:FILEPATH="$PREFIX/lib/liblua.a" \
		-DLUA_LIBRARY:FILEPATH="$PREFIX/lib/liblua.a" \
		-DLUA_FOUND:BOOL=TRUE
	emmake make -j"$(nproc)" install
fi

# ---- openssl 3 + curl HEADERS (real headers, stub archives) ----------------
if [ ! -f "$PREFIX/../openssl-headers/openssl/x509.h" ]; then
	echo "== openssl $OPENSSL_VER (headers only)"
	cd "$WORK"
	curl -L --fail --retry 3 -O "https://www.openssl.org/source/openssl-${OPENSSL_VER}.tar.gz"
	tar xf "openssl-${OPENSSL_VER}.tar.gz"
	# Openssl needs a Configure pass to synthesize opensslv.h / opensslconf.h;
	# we do NOT actually build the library — everything is stubbed.
	cd "openssl-${OPENSSL_VER}"
	./Configure no-shared no-asm no-tests linux-generic32 >/dev/null
	mkdir -p "$PREFIX/../openssl-headers/openssl"
	cp -r include/openssl/. "$PREFIX/../openssl-headers/openssl/"
fi

if [ ! -f "$PREFIX/include/curl/curl.h" ]; then
	echo "== curl $CURL_VER (headers only)"
	cd "$WORK"
	curl -L --fail --retry 3 -O "https://curl.se/download/curl-${CURL_VER}.tar.xz"
	tar xf "curl-${CURL_VER}.tar.xz"
	mkdir -p "$PREFIX/include/curl"
	cp -r "curl-${CURL_VER}/include/curl/." "$PREFIX/include/curl/"
fi

# ---- nel_wasm_stubs → libcurl.a / libssl.a / libcrypto.a -------------------
if [ ! -f "$PREFIX/lib/libcurl.a" ]; then
	echo "== nel_wasm_stubs (curl/ssl/crypto/sysvipc stubs)"
	STUBS="$SRC_ROOT/nel/samples/gui/emscripten/nel_wasm_stubs.c"
	if [ ! -f "$STUBS" ]; then
		echo "!! missing $STUBS — pass ryzomcore source root as arg 2" >&2
		exit 1
	fi
	cd "$WORK"
	emcc -O2 -I"$PREFIX/include" -I"$PREFIX/../openssl-headers" \
		-c "$STUBS" -o nel_wasm_stubs.o
	# Same object archived under three names; the linker only cares about
	# symbol resolution, not which archive provides them.
	emar rcs libcurl.a nel_wasm_stubs.o
	emar rcs libssl.a nel_wasm_stubs.o
	emar rcs libcrypto.a nel_wasm_stubs.o
	cp libcurl.a libssl.a libcrypto.a "$PREFIX/lib/"
fi

echo "== done: $PREFIX"
ls "$PREFIX/lib"
