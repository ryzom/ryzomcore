# Quick Start Docker Images

These Docker images are part of the quick start build environment.

Documentation for developer reference only.

To add another distribution; simply add a folder with a Dockerfile, and re-run the quick start configure script.

## Prepare Ubuntu

Just build the Docker images like this. The quick start scripts will generate the proper command line.

```
docker build -t "ryzombuild_bionic_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_bionic_x86_64\Dockerfile" Y:\ryzomcore4\code
```

```
docker build -t "ryzombuild_focal_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_focal_x86_64\Dockerfile" Y:\ryzomcore4\code
```

## Prepare Steam Runtime

```
cd /d Y:\ryzomcore4\.nel\temp
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.Dockerfile
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.tar.gz
docker build -f com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.Dockerfile -t steamrt_scout_amd64:latest .
del "com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.Dockerfile"
del "com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.tar.gz"
```

```
cd /d Y:\ryzomcore4\.nel\temp
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.Dockerfile
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.tar.gz
docker build -f com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.Dockerfile -t steamrt_scout_i386:latest .
rm com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.Dockerfile
rm com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.sysroot.tar.gz
```

## Check CMake Version

```
docker run --rm ryzombuild_bionic_x86_64 cmake --version
docker run --rm ryzombuild_focal_x86_64 cmake --version
docker run --rm steamrt_scout_amd64 cmake --version
```

## Build Client

```
docker run --rm --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_focal" ryzombuild_focal_x86_64 cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DHUNTER_ENABLED=ON -DHUNTER_STATUS_DEBUG=ON -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=ON -DWITH_DRIVER_OPENGL=ON -DWITH_DRIVER_OPENAL=ON ../code
docker run --rm --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_focal" ryzombuild_focal_x86_64 ninja
```