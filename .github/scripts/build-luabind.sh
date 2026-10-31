#!/usr/bin/env bash
# Build ryzom/luabind as a static library against the system Lua and install
# it into a prefix. The libluabind packages of Ubuntu do not work, so the Linux
# jobs that build the NeL GUI or the Ryzom client use this instead. The library
# is static (and built with -fPIC by luabind's own CMakeLists), so the shared
# NeL builds and the packages need no luabind at run time.
#
# luabind's find_package(Lua) takes the newest Lua installed, as the FindLua
# of the NeL build does after it, so both use whichever Lua the job installs.
#
# Args:
#   $1  destination prefix (created if absent)
#
# Needs a Lua development package, libboost-dev, curl and CMake. Idempotent:
# does nothing when the prefix already holds the library, so it can sit behind
# actions/cache.

set -euo pipefail

PREFIX="$1"
LUABIND_REV=6cb0688f78b753ce94df528ac640090251043759   # ryzom/luabind master

if [ -f "$PREFIX/lib/libluabind09.a" ]; then
	echo "== luabind already in $PREFIX"
	exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

echo "== luabind ryzom/luabind@$LUABIND_REV"
curl -L --fail --retry 3 "https://github.com/ryzom/luabind/archive/$LUABIND_REV.tar.gz" \
	| tar -xz -C "$WORK" --strip-components=1
cmake -S "$WORK" -B "$WORK/build" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX="$PREFIX" \
	-DLUABIND_DYNAMIC_LINK=OFF \
	-DBUILD_TESTING=OFF
cmake --build "$WORK/build" --parallel "$(nproc)"
cmake --install "$WORK/build"
