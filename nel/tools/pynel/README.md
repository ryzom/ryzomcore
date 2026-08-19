# pynel

Python tools for Ryzom/NeL data formats (`.ig`, `.shape`, `.bnp`, `.primitive`, `.cmb`) and admin protocols.

## Installation

Directly from GitLab, without cloning the whole `ryzom-core` repository:

```bash
pip install "git+https://gitlab.com/ryzom/ryzom-core.git#subdirectory=nel/tools/pynel"
```

Or from a local checkout:

```bash
pip install -e nel/tools/pynel
```

## Command-line tools

Installing the package also provides five console scripts:

- `ryzom-ig` -- read/write `.ig` (Instance Group) files
- `ryzom-shape` -- read/write `.shape` (3D mesh) files
- `ryzom-bnp` -- read/write `.bnp` (Big File package) archives
- `ryzom-primitive` -- read/write `.primitive` (LIGO primitive tree) files
- `ryzom-cmb` -- read `.cmb` (indoor collision mesh interchange) files

Run any of them with `--help` for usage details.

## Library usage

Each tool is also usable as a Python module:

```python
from pynel import ryzom_ig, ryzom_shape, ryzom_bnp, ryzom_primitive, collision_mesh_build

ig = ryzom_ig.load_ig("street.ig")
shape = ryzom_shape.load_shape("object.shape")
bnp = ryzom_bnp.BnpReader("data.bnp")
pf = ryzom_primitive.load_primitive("dummy.primitive")
cmb = collision_mesh_build.load_cmb("apartment.cmb")
```

## Planned: Georges sheets (`.creature`, `.item`, ...)

Not implemented yet. See [`docs/georges_sheets.md`](docs/georges_sheets.md)
for investigation notes (format overview, why it's a bigger job than
`.primitive`, and the plan) — read that before starting the work.
