FROM ubuntu:22.04

RUN apt update; \
    DEBIAN_FRONTEND=noninteractive \
    apt install --yes \
        autoconf \
        automake \
        bison \
        build-essential \
        ccache \
        clang-15 \
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
        mingw-w64 \
        mingw-w64-tools \
        ninja-build \
        qtbase5-dev \
        qttools5-dev \
        qttools5-dev-tools \
        libqtpropertybrowser-dev \
        qt6-base-dev \
        qt6-l10n-tools \
        qt6-tools-dev \
        qt6-tools-dev-tools


RUN apt install --yes \
        doxygen \
        gdb \
        git \
        graphviz \
        libxrandr-dev \
        wget

ARG USERNAME=ryzom
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG APP_HOME=/app

RUN groupadd --gid $USER_GID $USERNAME \
 && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

COPY build/ryzomcore-*.sh /tmp/ryzomcore.sh

RUN mkdir --parent "$APP_HOME" \
 && /tmp/ryzomcore.sh --skip-license

USER $USERNAME
