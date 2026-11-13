Scripts to auto-configure the build pipeline for every possible build target that can be built from the local machine

Run the following to configure the Docker containers:
- configure_docker.py

Run the scripts in this order to configure the toolchains:
- check_docker.py
- configure_toolchains.py
- configure_targets.py
- configure_pipeline.py
- configure_launchers.py

The configuration runs `0_setup.py -p`, which writes `.nel/buildsite_*.py` from the defaults. A root
configured before a newer pipeline lacks the settings that its new projects read, so run the
configuration again after updating. On a root that has already built its data, the configuration
skips the build steps, so install a new project from `cmd_build_gamedata.bat` afterwards. The
`common/examples` project, which installs the example scenarios and the NPC name lists of the Ring
editor (`ryzom/client/examples`) into the `examples` folder of the client, reads `ExamplesDirectory`:
`python 3_install.py -ipj common/examples`, then `python b1_client_dev.py` for the development client.

Python, MariaDB, nginx, PHP, phpMyAdmin, Perl, rrdtool, Dependencies and the command line utilities come
from the externals package (`Paths` in `config_default.json`, C:\2026q4_external_v143_x64 by default); the
quick start and the build pipeline run under its Python 3. Nothing is written into the externals folder: the
development web keeps its configuration, logs and phpMyAdmin copy in `pipeline/shard_dev/web`
(`configure_web.py`), and its database in `pipeline/shard_dev/mysql`.

The shard scripts (shard_mainland.bat and the others, web_on.bat) open servdash_gui from the tools
build. Set `"Headless": true` in `.nel/config.json` to get servdash_cli in the console instead, for a
box reached over SSH. One of them must be built, there is no fallback.
`.mcp.json` in the root registers `_servdash_mcp.bat` with Claude Code: the Model Context Protocol
server of the chosen dashboard tool, over every dashboard running for the user, so an agent can
start a shard from its ini, watch the logs, run service commands and stop it again.

Toolchains come from three scans: `find_vstudio.py` for the Visual Studio installations of the
native drive, `find_gcc.py` for GCC on the host and in every Docker image under `docker/`, and
`find_wine.py` for Windows toolchains under Wine, which asks each Docker image labelled
`dev.ryzom.quickstart.toolchain=wine` to probe the Wine prefixes of its volume (`dump_wine.py`,
sharing `vstudio_scan.py` with the native scan). A Wine toolchain needs a toolchain layer for its
toolset (`NeLWineToolchainLayers` in `configure_toolchains.py`, `tool/wine_vs2008` for v90) and
builds through Docker with Ninja and the layer's toolchain file; `configure_docker.py` downloads the
packaged prefixes (`WinePrefixes` in the configuration) into the volume. See `docker/README.md`.

Tools that are not C++ (servdash_cli, a Rust crate) are built by `build_docker/tools_xplat_build.bat`
in the `ryzombuild_xplat_tools` Docker image, for a generic Linux (static musl) and for Windows
(MSVC target through cargo-xwin), into `build_docker/tools_xplat/<target>/release`. The crates are
listed in `common.py` (`NeLXplatTools`). The Linux servdash_cli goes into `admin_install.tgz` as
`bin/servdash_cli` with the shard patch (`c1_shard_patch.py`).

To optimize your server build, check your native compiler options. Copy the -march and -mtune options, and add them to the CUSTOM_FLAGS option in CMake
```
gcc -march=native -E -v - </dev/null 2>&1 | grep cc1
```
