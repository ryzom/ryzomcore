#!/usr/bin/env bash
# Ryzom Core Studio - build, install and configure on Linux.
#
# Run from a ryzom-core checkout. Installs the build dependencies (asks first), builds NeL
# and studio, installs them below ~/.local/opt/ryzom-studio, and points studio at the
# ryzom-data and ryzom-server-data checkouts. Running it again updates: it rebuilds what
# changed and rewrites only the settings it owns. See studio/linux/README.md.
#
# Tested on Ubuntu 22.04/24.04, Debian 12/13 and Arch/CachyOS.

set -euo pipefail

CORE="$(cd "$(dirname "$0")/../.." && pwd)"
DATA=""
SERVER=""
GAME=""
PREFIX="$HOME/.local/opt/ryzom-studio"
BUILD="$CORE/build-studio"
STATE="$HOME/.local/share/ryzom-studio"
JOBS="$(nproc 2>/dev/null || echo 2)"
DO_DEPS=1
DO_BUILD=1
ASSUME_YES=0

usage() {
	cat <<EOF
Usage: studio/linux/setup.sh [options]

  --data DIR          ryzom-data checkout         (default: next to ryzom-core)
  --server-data DIR   ryzom-server-data checkout  (default: next to ryzom-core)
  --game DIR          Ryzom game installation, for the live PACS (optional)
  --prefix DIR        install location            (default: $PREFIX)
  --build-dir DIR     build directory             (default: $BUILD)
  --jobs N            parallel build jobs         (default: $JOBS)
  --skip-deps         do not install packages
  --config-only       only (re)write the configuration, do not build
  -y, --yes           do not ask before installing packages
EOF
}

while [ $# -gt 0 ]; do
	case "$1" in
		--data) DATA="$2"; shift 2 ;;
		--server-data) SERVER="$2"; shift 2 ;;
		--game) GAME="$2"; shift 2 ;;
		--prefix) PREFIX="$2"; shift 2 ;;
		--build-dir) BUILD="$2"; shift 2 ;;
		--jobs) JOBS="$2"; shift 2 ;;
		--skip-deps) DO_DEPS=0; shift ;;
		--config-only) DO_DEPS=0; DO_BUILD=0; shift ;;
		-y|--yes) ASSUME_YES=1; shift ;;
		-h|--help) usage; exit 0 ;;
		*) echo "unknown option: $1" >&2; usage >&2; exit 1 ;;
	esac
done

step() { printf '\n== %s\n' "$*"; }
die() { printf 'error: %s\n' "$*" >&2; exit 1; }

# ---------------------------------------------------------------------------------------
# The two data checkouts: given, or next to ryzom-core under their usual names.

find_checkout() {
	local marker="$1"; shift
	local name
	for name in "$@"; do
		if [ -d "$(dirname "$CORE")/$name/$marker" ]; then
			(cd "$(dirname "$CORE")/$name" && pwd)
			return 0
		fi
	done
	return 1
}

[ -n "$DATA" ] || DATA="$(find_checkout leveldesign ryzom-data ryzom-data-git)" || \
	die "ryzom-data not found next to $CORE - pass --data DIR"
[ -n "$SERVER" ] || SERVER="$(find_checkout primitives ryzom-server-data ryzom-server-data-git)" || \
	die "ryzom-server-data not found next to $CORE - pass --server-data DIR"
DATA="$(cd "$DATA" && pwd)"
SERVER="$(cd "$SERVER" && pwd)"

echo "ryzom-core:        $CORE"
echo "ryzom-data:        $DATA"
echo "ryzom-server-data: $SERVER"
echo "game (PACS):       ${GAME:-none - PACS from the pipeline export}"
echo "install to:        $PREFIX"

# ---------------------------------------------------------------------------------------
# Build dependencies

APT_PACKAGES="build-essential cmake pkg-config python3
	qtbase5-dev qttools5-dev qttools5-dev-tools libqt5opengl5-dev
	libxml2-dev libfreetype-dev libfontconfig-dev libgif-dev libjpeg-dev libpng-dev zlib1g-dev
	liblua5.1-0-dev libluabind-dev libboost-dev
	libogg-dev libvorbis-dev libopenal-dev
	libgl-dev libglu1-mesa-dev libx11-dev libxrandr-dev libxxf86vm-dev libxcursor-dev libxrender-dev
	libcurl4-openssl-dev libssl-dev"

PACMAN_PACKAGES="base-devel cmake python
	qt5-base qt5-tools
	libxml2 freetype2 fontconfig giflib libjpeg-turbo libpng zlib
	lua51 boost
	libogg libvorbis openal
	mesa glu libx11 libxrandr libxxf86vm libxcursor libxrender
	curl openssl"

as_root() {
	if [ "$(id -u)" -eq 0 ]; then "$@"; else sudo "$@"; fi
}

confirm() {
	[ "$ASSUME_YES" -eq 1 ] && return 0
	local answer
	read -r -p "$1 [Y/n] " answer
	case "$answer" in [nN]*) return 1 ;; *) return 0 ;; esac
}

install_deps() {
	. /etc/os-release
	local family="${ID} ${ID_LIKE:-}"
	case "$family" in
		*debian*|*ubuntu*)
			echo "Packages: $(echo $APT_PACKAGES)"
			confirm "Install these with apt?" || { echo "skipped"; return; }
			as_root apt-get update
			as_root env DEBIAN_FRONTEND=noninteractive apt-get install -y $APT_PACKAGES
			;;
		*arch*)
			echo "Packages: $(echo $PACMAN_PACKAGES)"
			confirm "Install these with pacman?" || { echo "skipped"; return; }
			as_root pacman -S --needed --noconfirm $PACMAN_PACKAGES
			# luabind is not in the Arch repositories.
			if ! ls /usr/lib/libluabind*.so >/dev/null 2>&1; then
				echo
				echo "luabind is missing and comes from the AUR on Arch, for example:"
				echo "    yay -S luabind-rpavlik-git"
				die "install luabind, then run this script again"
			fi
			;;
		*)
			echo "Unknown distribution '$ID'. Install the equivalents of:"
			echo "  $(echo $APT_PACKAGES)"
			echo "then run again with --skip-deps."
			die "cannot install dependencies automatically"
			;;
	esac
}

if [ "$DO_DEPS" -eq 1 ]; then
	step "Build dependencies"
	install_deps
fi

# ---------------------------------------------------------------------------------------
# Build and install: NeL and studio only, not the game or the server.

if [ "$DO_BUILD" -eq 1 ]; then
	step "Configure"
	mkdir -p "$BUILD"
	# CMAKE_POLICY_VERSION_MINIMUM: the build scripts predate CMake 4, which refuses
	# them without it. Older CMake versions ignore the variable.
	cmake -S "$CORE" -B "$BUILD" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$PREFIX" \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		-DOpenGL_GL_PREFERENCE=LEGACY \
		-DWITH_STUDIO=ON -DWITH_QT5=ON -DWITH_QT=OFF \
		-DWITH_RYZOM=OFF -DWITH_NELNS=OFF -DWITH_SNOWBALLS=OFF \
		-DWITH_NEL_TOOLS=OFF -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF \
		-DWITH_STATIC=OFF \
		>"$BUILD/configure.log" 2>&1 || { tail -40 "$BUILD/configure.log"; die "configure failed, see $BUILD/configure.log"; }
	echo "ok (log: $BUILD/configure.log)"

	step "Build ($JOBS jobs - this takes a while the first time)"
	cmake --build "$BUILD" -j "$JOBS"

	step "Install"
	cmake --install "$BUILD" >"$BUILD/install.log" 2>&1 || { tail -20 "$BUILD/install.log"; die "install failed"; }
	echo "ok"
fi

[ -x "$PREFIX/bin/studio" ] || die "no studio in $PREFIX/bin - build it first (without --config-only)"

# ---------------------------------------------------------------------------------------
# Configuration

step "Configuration"
python3 "$CORE/studio/linux/configure_studio.py" \
	--core "$CORE" --data "$DATA" --server-data "$SERVER" \
	${GAME:+--game "$GAME"} --state "$STATE"

# ---------------------------------------------------------------------------------------
# Launcher and menu entry

step "Launcher"
mkdir -p "$HOME/.local/bin" "$HOME/.local/share/applications"
LAUNCHER="$HOME/.local/bin/ryzom-studio"
cat >"$LAUNCHER" <<EOF
#!/bin/sh
# Written by ryzom-core/studio/linux/setup.sh
export LD_LIBRARY_PATH="$PREFIX/lib:$PREFIX/lib/studio\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
# The NeL 3D views embed an X11 window; under Wayland run through XWayland.
export QT_QPA_PLATFORM="\${QT_QPA_PLATFORM:-xcb}"
# Studio writes log.log to the working directory.
cd "\$(cat "$STATE/workdir" 2>/dev/null || echo "$PREFIX")" 2>/dev/null || cd "$PREFIX"
exec "$PREFIX/bin/studio" "\$@"
EOF
chmod +x "$LAUNCHER"

cat >"$HOME/.local/share/applications/ryzom-studio.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Ryzom Core Studio
Comment=World editor and tools for Ryzom
Exec=$LAUNCHER
Icon=$CORE/studio/src/plugins/core/images/nel.png
Terminal=false
Categories=Development;Game;
EOF

echo "Start with: ryzom-studio   (or from the application menu)"
case ":$PATH:" in
	*":$HOME/.local/bin:"*) ;;
	*) echo "Note: $HOME/.local/bin is not in your PATH - start with $LAUNCHER" ;;
esac
