FROM --platform=linux/amd64 ubuntu:bionic
MAINTAINER Ryzom Core <https://wiki.ryzom.dev/>

RUN apt-get update && apt-get install -y \
  software-properties-common \
  && rm -rf /var/lib/apt/lists/*

# RUN add-apt-repository ppa:ubuntu-toolchain-r/test

RUN apt-get update && apt-get install -y \
  cmake build-essential \
  gcc-8 g++-8 \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60 \
  && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
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
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
