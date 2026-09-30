# Ryzom Core Studio on Linux

Build studio yourself and point it at the Ryzom repositories. One script does all of it.

Tested on **Ubuntu 22.04 / 24.04**, **Debian 12 / 13** and **Arch / CachyOS**.

## 1. Get the three repositories

Clone them next to each other - the script finds them there on its own:

```
ryzom/
├── ryzom-core/           this repository, with studio
├── ryzom-data/           leveldesign, landscapes, zone banks, AI maps
└── ryzom-server-data/    primitives, game elements
```

The directory names may also end in `-git`. Elsewhere is fine too, then pass
`--data` and `--server-data`.

## 2. Run the script

```sh
cd ryzom-core
studio/linux/setup.sh --game ~/path/to/Ryzom
```

It will

1. install the build dependencies (`apt` or `pacman`, asks first),
2. build NeL and studio - the first time takes a while - and install them below
   `~/.local/opt/ryzom-studio`,
3. configure studio for the three repositories,
4. add the command `ryzom-studio` and an entry in the application menu.

`--game` is optional: the folder of your Ryzom installation (the one containing `data/`,
e.g. the Steam one). The PACS view then shows exactly what the live game uses. Without it,
the PACS come from the pipeline export in ryzom-data, which may differ from live.

**Arch:** luabind is not in the official repositories. Install it from the AUR first,
e.g. `yay -S luabind-rpavlik-git`.

## 3. Start

```sh
ryzom-studio
```

## Updating

`git pull` in the repositories, then run `studio/linux/setup.sh` again. It rebuilds only
what changed and rewrites only its own settings - window layout, show/hide switches and
the like stay as you left them.

Only the configuration, without building (e.g. after moving a repository):

```sh
studio/linux/setup.sh --config-only
```

## What the configuration does

The repositories were written for the Windows tools of their time. `configure_studio.py`
bridges that without changing anything in them. Everything it creates lives in
`~/.local/share/ryzom-studio`:

| | |
|---|---|
| `continents/<name>/` | Links to each `.land` with its zone bank, the layout studio and the `.worldedit` projects expect. Desert's zone bank is split between `graphics2/` and `pipeline/export/` in the repository. |
| `config/world_editor_classes_linux.xml` | The class file from ryzom-data with its Windows paths (`L:/leveldesign/Game_elem/...`) turned into real ones, case corrected. |
| `pacs/` | The PACS unpacked from the game's `*_pacs.bnp`, or links to the pipeline export. |

The settings go to `~/.config/RyzomCore/RyzomCoreStudio.ini`; the previous file is kept
as a `.bak-*` copy each time.

## Options

```
--data DIR          ryzom-data checkout
--server-data DIR   ryzom-server-data checkout
--game DIR          Ryzom game installation, for the live PACS
--prefix DIR        install location (default ~/.local/opt/ryzom-studio)
--build-dir DIR     build directory  (default ~/.cache/ryzom-studio/build)
--jobs N            parallel build jobs
--skip-deps         do not install packages
--config-only       only rewrite the configuration
-y, --yes           do not ask before installing packages
```

## Troubleshooting

* **Nothing shows in the map / "Pixmap not found" tiles** - run `--config-only` again and
  check its output for warnings about missing zone banks.
* **Wayland**: studio runs through XWayland (`QT_QPA_PLATFORM=xcb`, set by the launcher),
  because the NeL 3D views embed an X11 window.
* **Logs**: `~/.local/share/ryzom-studio/log.log`.
