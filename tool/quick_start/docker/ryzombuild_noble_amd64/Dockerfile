FROM --platform=linux/amd64 ubuntu:noble
LABEL org.opencontainers.image.authors="Ryzom Core <https://wiki.ryzom.dev/>"

# the deb822 sources of 24.04 and later
RUN sed -i -e 's/archive\.ubuntu\.com/mirrors\.digitalocean\.com/' /etc/apt/sources.list.d/ubuntu.sources

ARG CMAKE_VERSION=3.31.11
# libmsquic from packages.microsoft.com, with the headers of the same release
ARG MSQUIC_VERSION=2.6.3

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  software-properties-common aria2 wget \
  && aria2c https://packages.microsoft.com/config/ubuntu/24.04/packages-microsoft-prod.deb \
  && dpkg -i packages-microsoft-prod.deb \
  && rm packages-microsoft-prod.deb \
  && apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  ninja-build curl build-essential ccache \
  bison autoconf automake \
  libpng-dev \
  libjpeg-dev \
  libgif-dev libfreetype-dev \
  freeglut3-dev \
  liblua5.2-dev libboost-dev libcpptest-dev \
  libogg-dev libvorbis-dev libopenal-dev \
  libavcodec-dev libavformat-dev libavdevice-dev libswscale-dev libpostproc-dev \
  libmysqlclient-dev \
  libxml2-dev \
  libcurl4-openssl-dev libssl-dev \
  libsquish-dev \
  liblzma-dev \
  libgsf-1-dev \
  qtbase5-dev qttools5-dev qttools5-dev-tools \
  libmsquic=${MSQUIC_VERSION} \
  && aria2c https://raw.githubusercontent.com/microsoft/msquic/v${MSQUIC_VERSION}/src/inc/msquic.h \
  && aria2c https://raw.githubusercontent.com/microsoft/msquic/v${MSQUIC_VERSION}/src/inc/msquic_posix.h \
  && aria2c https://raw.githubusercontent.com/microsoft/msquic/v${MSQUIC_VERSION}/src/inc/quic_sal_stub.h \
  && mv msquic.h /usr/include/msquic.h \
  && mv msquic_posix.h /usr/include/msquic_posix.h \
  && mv quic_sal_stub.h /usr/include/quic_sal_stub.h \
  && rm -rf /var/lib/apt/lists/*

RUN wget --output-document=/tmp/cmake.sh https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh \
  && mkdir --parents /opt/cmake-${CMAKE_VERSION} \
  && sh /tmp/cmake.sh --skip-license --prefix=/opt/cmake-${CMAKE_VERSION} \
  && ln --symbolic --force /opt/cmake-${CMAKE_VERSION}/bin/* /usr/local/bin \
  && rm /tmp/cmake.sh

# ryzom/luabind as a static library, built like .github/scripts/build-luabind.sh does for the CI jobs;
# the luabind package of Ubuntu does not work
ARG LUABIND_REV=6cb0688f78b753ce94df528ac640090251043759
RUN mkdir /tmp/luabind \
  && curl -L --fail --retry 3 https://github.com/ryzom/luabind/archive/${LUABIND_REV}.tar.gz | tar -xz -C /tmp/luabind --strip-components=1 \
  && cmake -S /tmp/luabind -B /tmp/luabind/build -DCMAKE_BUILD_TYPE=Release -DLUABIND_DYNAMIC_LINK=OFF -DBUILD_TESTING=OFF \
  && cmake --build /tmp/luabind/build --parallel $(nproc) \
  && cmake --install /tmp/luabind/build \
  && rm -rf /tmp/luabind

ENV CC=/usr/lib/ccache/gcc
ENV CXX=/usr/lib/ccache/g++
ENV CCACHE_SLOPPINESS=pch_defines,time_macros

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
