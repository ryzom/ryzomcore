#!/usr/bin/env bash
#
# Sets up (creating it if missing) the .venv for the Ryzom Forgery tool suite
# and runs a given app script with it.
#
# Usage: ./dev.sh <path/to/app.py> [args...]

set -e

cd "$(dirname "$0")"


VIRTUAL_ENV=~/.cache/venvs/forgery.ryzom.com

# pynel lives in its own sibling worktree (../../../../pynel/nel/tools/pynel) since
# Forgery's sparse-checkout no longer contains nel/tools/pynel, but fall back to the
# old same-checkout layout (../pynel) in case this is run from a full checkout.
PYNEL_DIR=""
for candidate in ../../../../pynel/nel/tools/pynel ../pynel; do
	if [[ -f "$candidate/pyproject.toml" ]] && grep -qm1 '^name = "pynel"' "$candidate/pyproject.toml"; then
		PYNEL_DIR="$candidate"
		break
	fi
done
if [[ -z "$PYNEL_DIR" ]]; then
	echo "Could not find pynel's directory (looked for a pyproject.toml with name = \"pynel\" in ../../../../pynel/nel/tools/pynel and ../pynel)" >&2
	exit 1
fi

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
	$VIRTUAL_ENV/bin/pip install -e "$PYNEL_DIR" -q
	$VIRTUAL_ENV/bin/pip install -e . -q
fi

# pynel is developed alongside Forgery (../pynel, editable install) but on its own
# branch/release cadence -- an editable install picks up source changes for free, but
# NOT a version bump alone (pyproject.toml's own `version` field), since nothing else
# changed to trigger a re-resolve. Re-run `pip install -e` whenever the installed
# version drifts from ../pynel/pyproject.toml's, so a fresh `git pull`/branch switch
# there is never silently stale in this venv.
INSTALLED_PYNEL_VERSION=$($VIRTUAL_ENV/bin/python -c "import importlib.metadata as m; print(m.version('pynel'))" 2>/dev/null || echo "")
PYNEL_PYPROJECT_VERSION=$(grep -m1 '^version' "$PYNEL_DIR/pyproject.toml" | sed -E 's/version = "(.*)"/\1/')
if [[ "$INSTALLED_PYNEL_VERSION" != "$PYNEL_PYPROJECT_VERSION" ]]; then
	echo "pynel version changed ($INSTALLED_PYNEL_VERSION -> $PYNEL_PYPROJECT_VERSION), reinstalling..."
	$VIRTUAL_ENV/bin/pip install -e "$PYNEL_DIR" -q
fi

if [[ $# -eq 0 ]]; then
	echo "Usage: ./dev.sh <path/to/app.py> [args...]"
	exit 1
fi

exec $VIRTUAL_ENV/bin/python "$@"
