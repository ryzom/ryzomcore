Scripts to auto-configure the build pipeline for every possible build target that can be built from the local machine

Run the following to configure the Docker containers:
- configure_docker.py

Run the scripts in this order to configure the toolchains:
- check_docker.py
- configure_toolchains.py
- configure_targets.py
- configure_pipeline.py
- configure_launchers.py

The shard scripts (shard_mainland.bat and the others, web_on.bat) open servdash_gui from the tools
build. Set `"Headless": true` in `.nel/config.json` to get servdash_cli in the console instead, for a
box reached over SSH. The Win32 servdash of external/utils is the fallback when neither is built.

Tools that are not C++ (servdash_cli, a Rust crate) are built by `build_docker/tools_xplat_build.bat`
in the `ryzombuild_xplat_tools` Docker image, for a generic Linux (static musl) and for Windows
(MSVC target through cargo-xwin), into `build_docker/tools_xplat/<target>/release`. The crates are
listed in `common.py` (`NeLXplatTools`). The Linux servdash_cli goes into `admin_install.tgz` as
`bin/servdash_cli` with the shard patch (`c1_shard_patch.py`).

To optimize your server build, check your native compiler options. Copy the -march and -mtune options, and add them to the CUSTOM_FLAGS option in CMake
```
gcc -march=native -E -v - </dev/null 2>&1 | grep cc1
```
