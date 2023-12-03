FROM ubuntu:20.04

RUN apt update;\
    apt install --yes software-properties-common; \
    apt update
RUN DEBIAN_FRONTEND=noninteractive apt install --yes \
        autoconf \
        automake \
        bison  \
        build-essential \
        cmake \
        freeglut3-dev \
        libavcodec-dev \
        libavdevice-dev \
        libavformat-dev \
        libcpptest-dev \
        libcurl4-openssl-dev \
        libfreetype6-dev \
        libgif-dev \
        libgsf-1-dev \
        libjpeg-dev \
        liblua5.2-dev \
        libluabind-dev \
        liblzma-dev \
        libmysqlclient-dev \
        libogg-dev \
        libopenal-dev \
        libpng-dev \
        libpostproc-dev \
        libsquish-dev \
        libssl-dev \
        libswscale-dev \
        libvorbis-dev \
        libxml2-dev \
        qtbase5-dev \
        qttools5-dev \
        qttools5-dev-tools

RUN apt install --yes \
        gdb \
        git \
        libxrandr-dev
# work arround as there is no debug version available
RUN cp /lib/x86_64-linux-gnu/libcpptest.so /lib/x86_64-linux-gnu/libcpptestd.so
