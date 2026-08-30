# pynel

Python tools for Ryzom/NeL data formats (`.ig`, `.shape`, `.bnp`, `.primitive`, `.cmb`, `.packed_sheets`) and admin protocols.

> Development on this package happens on the `ryzom/pynel` branch. Forgery's own work
> branch is `ryzom/forgery` (see its README).

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

Installing the package also provides console scripts:

- `ryzom-ig` -- read/write `.ig` (Instance Group) files
- `ryzom-shape` -- read/write `.shape` (3D mesh) files
- `ryzom-anim` -- read `.anim` (skeletal animation) files
- `ryzom-bnp` -- read/write `.bnp` (Big File package) archives
- `ryzom-primitive` -- read/write `.primitive` (LIGO primitive tree) files
- `ryzom-cmb` -- read `.cmb` (indoor collision mesh interchange) files
- `ryzom-hlsbank` -- read/append `.hlsbank` (HLS-colorisable texture bank)
  files, see [`docs/hls_texture_bank.md`](docs/hls_texture_bank.md)
- `ryzom-cfg` -- read/write `.cfg` (`CConfigFile` format, e.g. `client.cfg`,
  `panoply_*.cfg`) files, see [`docs/config_file.md`](docs/config_file.md)
- `ryzom-packed-sheets` -- read `creature.packed_sheets` (Georges sheet binary
  cache) and `sheet_id.bin`, see [`docs/packed_sheets.md`](docs/packed_sheets.md)

Run any of them with `--help` for usage details.

## Library usage

Each tool is also usable as a Python module:

```python
from pynel import ryzom_ig, ryzom_shape, ryzom_bnp, ryzom_primitive, collision_mesh_build, ryzom_packed_sheets

ig = ryzom_ig.load_ig("street.ig")
shape = ryzom_shape.load_shape("object.shape")
bnp = ryzom_bnp.BnpReader("data.bnp")
pf = ryzom_primitive.load_primitive("dummy.primitive")
cmb = collision_mesh_build.load_cmb("apartment.cmb")
packed = ryzom_packed_sheets.load_creature_packed_sheets("creature.packed_sheets")
names = ryzom_packed_sheets.parse_sheet_id_bin(bnp.read_file("sheet_id.bin"))  # sheet_id.bin ships inside leveldesign.bnp
```

### Locating a user's Ryzom repository checkouts

`repository_paths` is a small per-user JSON settings file shared across
every pynel-based tool, so a user only has to point out where their
`ryzom-core`/`ryzom-data`/`ryzom-private-data`/`ryzom-docker` checkouts live
once. See [`docs/repository_paths.md`](docs/repository_paths.md).

```python
from pynel import repository_paths

if repository_paths.is_valid("ryzom-data"):
	data_root = repository_paths.get("ryzom-data")
```

### Following a log file

`LogFollower` tails a log file like `tail -F`, surviving rotation
(copytruncate or create/rename) and the file being briefly missing:

```python
from pynel.log_follower import LogFollower

for line in LogFollower("/var/log/foo.log"):
	if "ERROR" in line:
		print(line)
```

## Planned: Georges sheets (`.creature`, `.item`, ...)

Not implemented yet. See [`docs/georges_sheets.md`](docs/georges_sheets.md)
for investigation notes (format overview, why it's a bigger job than
`.primitive`, and the plan) — read that before starting the work.
