# Quick Start Docker Images

These Docker images are part of the quick start build environment.

Documentation for developer reference only.

To add another distribution; simply add a folder with a Dockerfile here or under `.nel/docker`, and re-run the quick start configure script.

## Prepare Ubuntu

Just build the Docker images like this. The quick start scripts will generate the proper command line.

```
docker build -t "ryzombuild_bionic_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_bionic_x86_64\Dockerfile" Y:\ryzomcore4\code
docker build -t "ryzombuild_bionic_gcc8_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_bionic_gcc8_x86_64\Dockerfile" Y:\ryzomcore4\code
docker build -t "ryzombuild_focal_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_focal_x86_64\Dockerfile" Y:\ryzomcore4\code
docker build -t "ryzombuild_scout_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_scout_x86_64\Dockerfile" Y:\ryzomcore4\code
docker build -t "ryzombuild_scout_gcc5_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_scout_gcc5_x86_64\Dockerfile" Y:\ryzomcore4\code
docker build -t "ryzombuild_scout_gcc9_x86_64" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_scout_gcc9_x86_64\Dockerfile" Y:\ryzomcore4\code
docker build -t "ryzombuild_scout_gcc9_x86" -f "Y:\ryzomcore4\code\tool\quick_start\docker\ryzombuild_scout_gcc9_x86\Dockerfile" Y:\ryzomcore4\code
```

## Prepare Steam Runtime

```
cd /d Y:\ryzomcore4\.nel\temp
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.Dockerfile
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.tar.gz
docker build  --platform linux/amd64 -f com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.Dockerfile -t steamrt_scout_amd64:latest .
del "com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.Dockerfile"
del "com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot.tar.gz"
```

```
cd /d Y:\ryzomcore4\.nel\temp
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.Dockerfile
..\..\distribution\utils\aria2c https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.tar.gz
docker build --platform linux/386 -f com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.Dockerfile -t steamrt_scout_i386:latest .
del "com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.Dockerfile"
del "com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.tar.gz"
```

## Check CMake Version

```
docker run --rm ryzombuild_bionic_x86_64 cmake --version
docker run --rm ryzombuild_focal_x86_64 cmake --version
docker run --rm steamrt_scout_amd64 cmake --version
```

## Check GCC Version

```
docker run --rm ryzombuild_bionic_x86_64 gcc -v
docker run --rm ryzombuild_bionic_gcc8_x86_64 gcc -v
docker run --rm ryzombuild_focal_x86_64 gcc -v
docker run --rm ryzombuild_scout_x86_64 gcc -v
docker run --rm ryzombuild_scout_gcc5_x86_64 gcc -v
docker run --rm ryzombuild_scout_gcc9_x86_64 gcc -v
docker run --rm ryzombuild_scout_gcc9_x86_64 cc -v
docker run --rm ryzombuild_scout_gcc9_x86_64 c++ -v
docker run --rm ryzombuild_scout_gcc9_x86 c++ -v
```

## Build Client

```
docker run --rm -v ryzombuild_focal_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_focal" ryzombuild_focal_x86_64 cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DHUNTER_ENABLED=ON -DHUNTER_STATUS_DEBUG=ON -DHUNTER_JOBS_NUMBER=9 -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON -DWITH_RYZOM_PATCH=ON -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=ON -DWITH_DRIVER_OPENGL=ON -DWITH_DRIVER_OPENAL=ON -DWITH_SSE3=OFF ../code
docker run --rm -v ryzombuild_focal_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_focal" ryzombuild_focal_x86_64 ninja -j9
```

```
docker run --rm -v ryzombuild_bionic_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_bionic" ryzombuild_bionic_x86_64 cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON -DWITH_RYZOM_PATCH=ON -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=ON -DWITH_DRIVER_OPENGL=ON -DWITH_DRIVER_OPENAL=ON -DWITH_SSE3=OFF -DWITH_STATIC=ON -DWITH_STATIC_DRIVERS=ON ../code
docker run --rm -v ryzombuild_bionic_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_bionic" ryzombuild_bionic_x86_64 ninja -j9
```

## Test Steam Runtime directly with GCC 4.8.4

```
docker run --rm -v steamrt_scout_amd64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout" steamrt_scout_amd64 cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DHUNTER_ENABLED=ON -DHUNTER_STATUS_DEBUG=ON -DHUNTER_JOBS_NUMBER=9 -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON -DWITH_RYZOM_PATCH=ON -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=ON -DWITH_DRIVER_OPENGL=ON -DWITH_DRIVER_OPENAL=ON -DWITH_SSE3=OFF ../code
docker run --rm -v steamrt_scout_amd64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout" steamrt_scout_amd64 ninja -j9
```

## Test Steam Runtime with GCC 5

```
docker run --rm -v ryzombuild_scout_gcc5_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout_gcc5" ryzombuild_scout_gcc5_x86_64 cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DHUNTER_ENABLED=ON -DHUNTER_STATUS_DEBUG=ON -DHUNTER_JOBS_NUMBER=9 -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON -DWITH_RYZOM_PATCH=ON -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=ON -DWITH_DRIVER_OPENGL=ON -DWITH_DRIVER_OPENAL=ON -DWITH_SSE3=OFF ../code
docker run --rm -v ryzombuild_scout_gcc5_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout_gcc5" ryzombuild_scout_gcc5_x86_64 ninja -j9
```

## Test Steam Runtime with GCC 9

```
docker run --rm -v ryzombuild_scout_gcc9_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout_gcc9" ryzombuild_scout_gcc9_x86_64 cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DHUNTER_ENABLED=ON -DHUNTER_STATUS_DEBUG=ON -DHUNTER_JOBS_NUMBER=9 -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON -DWITH_RYZOM_PATCH=ON -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=ON -DWITH_DRIVER_OPENGL=ON -DWITH_DRIVER_OPENAL=ON -DWITH_SSE3=OFF ../code
docker run --rm -v ryzombuild_scout_gcc9_x86_64_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout_gcc9" ryzombuild_scout_gcc9_x86_64 ninja -j9
```

## Test Steam Runtime with GCC 9 x86

```
docker run --rm -v ryzombuild_scout_gcc9_x86_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout_gcc9_32" ryzombuild_scout_gcc9_x86 cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DHUNTER_ENABLED=ON -DHUNTER_STATUS_DEBUG=ON -DHUNTER_JOBS_NUMBER=9 -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON -DWITH_RYZOM_PATCH=ON -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=ON -DWITH_DRIVER_OPENGL=ON -DWITH_DRIVER_OPENAL=ON -DWITH_SSE3=OFF ../code
docker run --rm -v ryzombuild_scout_gcc9_x86_hunter:/root/.hunter --mount "type=bind,source=Y:\ryzomcore4,target=/mnt/nel" --workdir "/mnt/nel/build_client_scout_gcc9_32" ryzombuild_scout_gcc9_x86 ninja -j9
```
