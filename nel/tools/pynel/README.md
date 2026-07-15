# pynel

Python tools for Ryzom/NeL data formats (`.ig`, `.shape`, `.bnp`) and admin protocols.

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

Installing the package also provides three console scripts:

- `ryzom-ig` -- read/write `.ig` (Instance Group) files
- `ryzom-shape` -- read/write `.shape` (3D mesh) files
- `ryzom-bnp` -- read/write `.bnp` (Big File package) archives

Run any of them with `--help` for usage details.

## Library usage

Each tool is also usable as a Python module:

```python
from pynel import ryzom_ig, ryzom_shape, ryzom_bnp

ig = ryzom_ig.load_ig("street.ig")
shape = ryzom_shape.load_shape("object.shape")
bnp = ryzom_bnp.BnpReader("data.bnp")
```
