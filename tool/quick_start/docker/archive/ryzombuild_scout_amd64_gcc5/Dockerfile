FROM --platform=linux/amd64 steamrt_scout_amd64:latest
MAINTAINER Ryzom Core <https://wiki.ryzom.dev/>

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 50 \
  && update-alternatives --set gcc "/usr/bin/gcc-5" \
  && update-alternatives --set g++ "/usr/bin/g++-5"

RUN update-alternatives --install /usr/bin/ld ld /usr/bin/ld.gold 50 \
  && update-alternatives --set ld "/usr/bin/ld.gold"

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
