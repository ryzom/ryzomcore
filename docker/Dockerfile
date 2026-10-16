FROM ubuntu:22.04

ARG USERNAME=ryzom
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG APP_HOME=/app

RUN groupadd --gid $USER_GID $USERNAME \
 && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

ENV LD_LIBRARY_PATH=/usr/local/lib

# install dependencies
RUN apt update; \
    apt install --yes \
        libcurl4 \
        libfreetype6 \
        libgif7 \
        libgsf-1-114 \
        libjpeg8 \
        liblua5.2-0 \
        libluabind0.9.1d1 \
        libmysqlclient21 \
        libpng16-16 \
        libqt5widgets5 \
        libsquish0 \
        libvorbis0a \
        libvorbisfile3 \
        libxml2


COPY build/ryzomcore-*.sh /tmp/ryzomcore.sh

RUN mkdir --parent "$APP_HOME" \
 && sh /tmp/ryzomcore.sh --skip-license


USER $USERNAME
WORKDIR $APP_HOME
