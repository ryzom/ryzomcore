Scripts to auto-configure the build pipeline for every possible build target that can be built from the local machine

Run the following to configure the Docker containers:
- configure_docker.py

Run the scripts in this order to configure the toolchains:
- check_docker.py
- configure_toolchains.py
- configure_targets.py
- configure_pipeline.py
- configure_launchers.py
