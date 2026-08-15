#!/usr/bin/env bash
#
# Sets up (creating it if missing) the .venv for the Ryzom Forgery tool suite
# and runs a given app script with it.
#
# Usage: ./dev.sh <path/to/app.py> [args...]

set -e

cd "$(dirname "$0")"

if [[ ! -d .venv ]]; then
	echo "Creating Python virtual env..."
	python3 -m venv .venv
	.venv/bin/pip install --upgrade pip -q
	.venv/bin/pip install -e ../pynel -q
	.venv/bin/pip install -e . -q
fi

if [[ $# -eq 0 ]]; then
	echo "Usage: ./dev.sh <path/to/app.py> [args...]"
	exit 1
fi

exec .venv/bin/python "$@"
