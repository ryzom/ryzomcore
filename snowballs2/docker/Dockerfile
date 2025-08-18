FROM ubuntu:24.04
RUN touch /var/mail/ubuntu && chown ubuntu /var/mail/ubuntu && userdel --remove ubuntu

ARG USERNAME=snowballs
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG APP_HOME=/app

RUN groupadd --gid $USER_GID $USERNAME \
 && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

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


COPY build/snowballs/snowballs-*.sh /tmp/install.sh

RUN mkdir --parent "$APP_HOME/snowballs" \
 && sh /tmp/install.sh --skip-license --prefix="$APP_HOME/snowballs"

ENV LD_LIBRARY_PATH=$APP_HOME/snowballs/lib
ENV PATH=$APP_HOME/snowballs/sbin:$APP_HOME/snowballs/bin:$PATH

RUN mkdir --parent "$APP_HOME/logs" \
 && chown $USERNAME:$USERNAME "$APP_HOME/logs"

USER $USERNAME
WORKDIR $APP_HOME
