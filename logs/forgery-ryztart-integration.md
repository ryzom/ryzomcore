# Changelog

## 2026-08-26 — 📦 Bundle Forgery's icon/splash/panoply data in the wheel

`forgery.png`/`splashscreen.png` (used by `app.py`'s window icon/splash, referenced via
`Path(__file__).resolve().parent.parent / "..."` from `ryzom_forgery/app.py`, i.e. one
level above the installed package) and `ryzom_forgery/panoply_colors.toml` weren't
included in the built wheel at all: setuptools only bundles `.py` files under a listed
package by default, and the two `.png`s lived outside the package directory entirely
(sibling to `ryzom_forgery/`, not inside it). A fresh `pip install` of the wheel would
therefore install with the object editor's icon/splash silently missing (guarded by
`.exists()` checks, so no crash, just no icon/splash) and `panoply_config.py`'s
`_TOML_PATH` raising `FileNotFoundError` at `list_apps()` time (breaks the whole
ryztart Forgery page, not just panoply).

Moved `forgery.png`/`splashscreen.png` into `ryzom_forgery/` itself (updated
`app.py`'s `_ICON_PATH`/`_SPLASH_PATH` from `.parent.parent` to `.parent` to match),
and declared all three as `[tool.setuptools.package-data]` in `pyproject.toml`
(`ryzom_forgery = ["panoply_colors.toml", "forgery.png", "splashscreen.png"]`).

## 2026-08-26 — ✨ Publish pynel wheel and reference it from Forgery's deps

`ryzom_forgery` imports `pynel` (`.bnp` reading, `.shape` parsing) at runtime, but
`pynel` isn't on PyPI (see `nel/tools/forgery/README.md`), so it wasn't in
`pyproject.toml`'s `dependencies` at all -- installing the `ryzom_forgery` wheel alone
(e.g. from ryztart's launcher) left `pynel` missing (`ModuleNotFoundError` the moment
any app module that touches `.bnp`/`.shape` I/O gets imported).

Added `build_pynel_wheel`/`upload_pynel_wheel` jobs to `.gitlab-ci.yml` (same shape as
the existing Forgery ones: build on `main/forgery-release` when `nel/tools/pynel/**`
changes, publish to the project's generic package registry under a fixed, branch-slug
keyed path, filename version pinned to `0` for the same PEP 440 reason as Forgery's own
wheel -- see below). Added `pynel @ <that wheel's URL>` as a direct-URL dependency in
Forgery's own `pyproject.toml`, so `pip install` of the Forgery wheel alone now pulls
pynel in too.

## 2026-08-26 — 🐛 Fix Forgery wheel filename to satisfy PEP 427/440

The `upload_forgery_wheel` CI job originally named the uploaded file after
`$CI_COMMIT_REF_SLUG` (`main-forgery-release`, since only that one branch triggers the
job -- see the job below). Two problems surfaced once actually installed by ryztart:

- A wheel filename's version field is `-`-delimited (PEP 427); `main-forgery-release`
  has extra hyphens, so pip parsed it as the wrong number of fields
  ("Invalid wheel filename (wrong number of parts)").
- Swapping hyphens for underscores fixed the parsing but not validity: the version
  field must also be a real PEP 440 version (starts with a digit) --
  `main_forgery_release` isn't one either ("Invalid wheel filename (invalid version)").

Settled on a fixed, meaningless `0` as the wheel filename's version field --
real version tracking happens separately, by ryztart's launcher comparing the
installed package's `importlib.metadata.version("ryzom_forgery")` against
`pyproject.toml`'s actual version fetched from GitLab's raw-file API (see the
ryztart-side log for `ryzom_forgery_launcher`), not via this filename at all. Also
switched the install command to `pip install --upgrade --force-reinstall`: since the
filename never changes even though its content does on every publish, plain
`--upgrade` let pip conclude nothing changed and skip reinstalling, silently keeping
stale dependencies (this is also why `nel/tools/forgery/pyproject.toml`'s `version`
must be bumped on every Forgery change from now on -- it's the only signal the
launcher's auto-update check has to notice a new build exists at all).

## 2026-08-26 — ✨ Publish ryzom_forgery wheel via GitLab CI

Added `.gitlab-ci.yml` at the `ryzom-core` repo root (didn't exist before): a dedicated
branch, `main/forgery-release`, builds `nel/tools/forgery`'s wheel (`python -m build`)
and publishes it to the project's own GitLab generic package registry whenever that
branch is pushed with `nel/tools/forgery/**` changes -- same family of mechanism as
`kyss.ryzom.com`'s own `whl/`/`file/` redirects (thin proxies in front of a GitLab
generic package registry and raw-file API), confirmed via `kyss4/docs/BUILD_AND_BOOTSTRAP.md`
that `kyss_wheel` in an app's `setup.cfg` is consumed only by Kyss's own build/bootstrap
tooling, not something ryztart's runtime code can reuse directly for an arbitrary
package -- so this is Forgery's own equivalent, hand-rolled, not reusing that field.

## 2026-08-26 — ✨ Expose ryzom_forgery.list_apps()/launch_app() and package apps/ properly

Two changes needed so ryztart can show a Forgery app list and launch one, without
knowing anything about Forgery's internal module layout:

- `apps/` (previously outside the `ryzom_forgery` package entirely, run as loose
  scripts via `./dev.sh apps/object_editor.py`) moved to `ryzom_forgery/apps/`, added
  to `pyproject.toml`'s `packages` -- otherwise a `pip install` of the wheel wouldn't
  ship them at all, only the shared scaffolding package.
- `ryzom_forgery/__init__.py` (previously empty) now exposes `list_apps()` (discovers
  every `ryzom_forgery.apps.*` module exposing an `APP_INFO` dict via `pkgutil`,
  returns their metadata) and `launch_app(app_id)` (imports and calls that module's
  `main()`). `object_editor.py`'s `ObjectEditorApp` gained an `APP_INFO` dict
  (`id`/`name`="Patina"/`subtitle`="Object Editor"/`description`) and a bare `main()`
  wrapping `ObjectEditorApp().run()`, matching this convention -- `shape_exporter.py`/
  `shape_importer.py` (CLI tools, not GUI apps) deliberately don't define `APP_INFO`,
  so they're skipped by `list_apps()`.
