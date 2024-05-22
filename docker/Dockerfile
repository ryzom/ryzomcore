FROM ubuntu:22.04

ARG USERNAME=ryzom
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG APP_HOME=/app

RUN groupadd --gid $USER_GID $USERNAME \
 && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

COPY build/ryzomcore-*.sh /tmp/ryzomcore.sh

RUN mkdir --parent "$APP_HOME" \
 && /tmp/ryzomcore.sh --skip-license

ENV LD_LIBRARY_PATH=/usr/local/lib

# install dependencies
RUN apt update; \
    apt install --yes \
        libgif7 \
        libjpeg8 \
        libpng16-16 \
        libxml2 \
        libfreetype6 \
        liblua5.2-0 \
    	libluabind0.9.1d1 \
    	libcurl4 \
        libvorbis0a \
        libvorbisfile3

USER $USERNAME
