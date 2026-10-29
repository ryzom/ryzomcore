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
box reached over SSH; it comes from external/utils, next to the Win32 servdash that is the fallback.

To optimize your server build, check your native compiler options. Copy the -march and -mtune options, and add them to the CUSTOM_FLAGS option in CMake
```
gcc -march=native -E -v - </dev/null 2>&1 | grep cc1
```
