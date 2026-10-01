#!/usr/bin/env bash
#
# Sets up (creating it if missing) the .venv for the Ryzom Forgery tool suite
# and runs a given app script with it.
#
# Usage: ./dev.sh <path/to/app.py> [args...]

set -e

cd "$(dirname "$0")"


VIRTUAL_ENV=~/.cache/venvs/forgery.ryzom.com

if [[ ! -d $VIRTUAL_ENV ]]; then
	echo "Creating Python virtual env..."
	# Prefer 3.12: some dependencies (e.g. assimp-py) only ship precompiled
	# wheels up to 3.13, and a too-new interpreter (e.g. a distro's bleeding-edge
	# python3) silently falls back to building them from source, which fails
	# outright for packages whose sdist doesn't bundle their native submodules.
	PYTHON=$(command -v python3.12 || command -v python3.13 || command -v python3)
	mkdir -p ~/.cache/venvs/
	"$PYTHON" -m venv $VIRTUAL_ENV
	$VIRTUAL_ENV/bin/pip install --upgrade pip -q
	$VIRTUAL_ENV/bin/pip install -e ../pynel -q
	$VIRTUAL_ENV/bin/pip install -e . -q
fi

if [[ $# -eq 0 ]]; then
	echo "Usage: ./dev.sh <path/to/app.py> [args...]"
	exit 1
fi

exec $VIRTUAL_ENV/bin/python "$@"
