FROM --platform=linux/amd64 ubuntu:focal
MAINTAINER Ryzom Core <https://wiki.ryzom.dev/>

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  software-properties-common \
  cmake ninja-build curl wget build-essential \
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

RUN mkdir -p hunter_dependencies/cmake

COPY tool/quick_start/docker/hunter_dependencies/CMakeLists.txt hunter_dependencies/CMakeLists.txt
COPY CMakeModules/HunterGate.cmake hunter_dependencies/cmake/HunterGate.cmake
COPY CMakeModules/HunterConfig.cmake hunter_dependencies/cmake/HunterConfig.cmake

RUN mkdir -p hunter_dependencies/build \
  && cd hunter_dependencies/build \
  && cmake -G "Ninja" -DHUNTER_JOBS_NUMBER=$((`nproc` * 3 / 4)) .. \
  && cd ../.. \
  && rm -rf hunter_dependencies

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
