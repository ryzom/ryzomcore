# ryzom_forgery

Shared Panda3D + ImGui scaffolding for the "Ryzom Forgery" tool suite (see
`nel/tools/pynel/docs/roadmap.md`, "Ryzom Forgery" section).

Ryzom Forgery is a suite of separate, small tool apps (object editor,
collision viewer, ...), each combining `pynel` (Ryzom/NeL file format I/O),
Panda3D (rendering) and Dear ImGui (tool UI). This package holds the
scaffolding shared across those apps — window/app bootstrap, camera, input,
the standard sysinfo/explorer/panel layout — so each tool app only has to
implement its own tool-specific logic.

This package depends on `pynel` (the standard explorer browses `.bnp`
archive contents via `pynel.ryzom_bnp`). `pynel` isn't published to PyPI, so
it's not listed in `pyproject.toml`'s dependencies — `dev.sh` installs it as
an editable sibling checkout (`../pynel`) instead, same as `ryztart`/`kyss`.

`ryzom_forgery/apps/shape_exporter.py` is a command-line exception to that
(no GUI): it converts a single `.shape` to `.obj`/`.dae`/`.stl`/`.gltf`/`.glb`
(format picked from the output file's extension), reusing the same export
code as the object editor's "Export to..." commands.

```sh
./dev.sh ryzom_forgery/apps/shape_exporter.py path/to/aaa.shape path/to/bbb.obj
```

GUI apps (currently just the object editor) are discoverable at runtime via
`ryzom_forgery.list_apps()`/`ryzom_forgery.launch_app(app_id)`, used by
`ryztart` to build its Forgery app list without needing to know about
`ryzom_forgery`'s internal module layout.
