FROM --platform=linux/amd64 ubuntu:bionic
MAINTAINER Ryzom Core <https://wiki.ryzom.dev/>

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  software-properties-common \
  ninja-build curl wget build-essential \
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
  && curl -sSL https://apt.kitware.com/kitware-archive.sh | bash \
  && apt-get install -y cmake \
  && rm -rf /var/lib/apt/lists/*

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  gcc-8 g++-8 \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60 \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
