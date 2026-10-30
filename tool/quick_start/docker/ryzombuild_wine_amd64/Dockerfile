FROM --platform=linux/amd64 ubuntu:noble
LABEL org.opencontainers.image.authors="Ryzom Core <https://wiki.ryzom.dev/>"
# Windows toolchains under Wine: the quick start probes the Wine prefixes of the ryzombuild_wine_amd64_prefixes
# volume (dump_wine.py) instead of looking for GCC. The prefixes are downloaded by configure_wine.py, see
# "WinePrefixes" in config_default.json; tool/wine_vs2008 is the toolchain layer for Visual Studio 2008.
LABEL dev.ryzom.quickstart.toolchain="wine"

# the deb822 sources of 24.04 and later
RUN sed -i -e 's/archive\.ubuntu\.com/mirrors\.digitalocean\.com/' /etc/apt/sources.list.d/ubuntu.sources

ARG CMAKE_VERSION=3.31.11

RUN dpkg --add-architecture i386 \
  && apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
  wine wine32:i386 \
  ninja-build python3 p7zip-full curl ca-certificates wget \
  && rm -rf /var/lib/apt/lists/*

RUN wget --output-document=/tmp/cmake.sh https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh \
  && mkdir --parents /opt/cmake-${CMAKE_VERSION} \
  && sh /tmp/cmake.sh --skip-license --prefix=/opt/cmake-${CMAKE_VERSION} \
  && ln --symbolic --force /opt/cmake-${CMAKE_VERSION}/bin/* /usr/local/bin \
  && rm /tmp/cmake.sh

ENV WINEDEBUG=-all

RUN mkdir -p /mnt/nel /opt/wineprefixes

WORKDIR /mnt/nel
