FROM --platform=linux/amd64 ubuntu:jammy
MAINTAINER Ryzom Core <https://wiki.ryzom.dev/>

RUN sed -i -e 's/archive\.ubuntu\.com/mirrors\.digitalocean\.com/' /etc/apt/sources.list

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  software-properties-common aria2 wget \
  && aria2c https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb \
  && dpkg -i packages-microsoft-prod.deb \
  && rm packages-microsoft-prod.deb \
  && apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  ninja-build curl build-essential ccache \
  bison autoconf automake \
  libpng-dev \
  libjpeg-dev \
  libgif-dev libfreetype6-dev \
  freeglut3-dev \
  liblua5.2-dev libluabind-dev libcpptest-dev \
  libogg-dev libvorbis-dev libopenal-dev \
  libavcodec-dev libavformat-dev libavdevice-dev libswscale-dev libpostproc-dev \
  libmysqlclient-dev \
  libxml2-dev \
  libcurl4-openssl-dev libssl-dev \
  libsquish-dev \
  liblzma-dev \
  libgsf-1-dev \
  qtbase5-dev qttools5-dev qttools5-dev-tools \
  libmsquic \
  && aria2c https://raw.githubusercontent.com/microsoft/msquic/main/src/inc/msquic.h \
  && aria2c https://raw.githubusercontent.com/microsoft/msquic/main/src/inc/msquic_posix.h \
  && aria2c https://raw.githubusercontent.com/microsoft/msquic/main/src/inc/quic_sal_stub.h \
  && mv msquic.h /usr/include/msquic.h \
  && mv msquic_posix.h /usr/include/msquic_posix.h \
  && mv quic_sal_stub.h /usr/include/quic_sal_stub.h \
  && wget --output-document=/tmp/cmake.sh https://github.com/Kitware/CMake/releases/download/v3.27.9/cmake-3.27.9-linux-x86_64.sh \
  && mkdir /opt/cmake \
  && sh /tmp/cmake.sh --skip-license --prefix=/opt/cmake \
  && ln --symbolic --force /opt/cmake/bin/cmake /usr/local/bin/cmake \
  && rm -rf /var/lib/apt/lists/*

ENV CC=/usr/lib/ccache/gcc
ENV CXX=/usr/lib/ccache/g++
ENV CCACHE_SLOPPINESS=pch_defines,time_macros

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
