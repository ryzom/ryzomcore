FROM ubuntu:24.04
RUN touch /var/mail/ubuntu && chown ubuntu /var/mail/ubuntu && userdel --remove ubuntu

RUN apt update && \
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
        wget \
        sudo


# work arround as there is no debug version available
RUN cp /lib/x86_64-linux-gnu/libcpptest.so /lib/x86_64-linux-gnu/libcpptestd.so

# install msquic
RUN wget --output-document=/tmp/microsoft.deb https://packages.microsoft.com/config/ubuntu/24.04/packages-microsoft-prod.deb \
    && dpkg --install /tmp/microsoft.deb \
    && apt update \
    && apt install --yes \
      libmsquic \
    && wget https://raw.githubusercontent.com/microsoft/msquic/main/src/inc/msquic.h \
    && wget https://raw.githubusercontent.com/microsoft/msquic/main/src/inc/msquic_posix.h \
    && wget https://raw.githubusercontent.com/microsoft/msquic/main/src/inc/quic_sal_stub.h \
    && sudo mv msquic.h msquic_posix.h quic_sal_stub.h /usr/include/


ARG CMAKE_VERSION=3.29.5
ARG CMAKE_INSTALL_DIR=/opt/cmake-$CMAKE_VERSION
ARG USERNAME=ryzom
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN wget --output-document=/tmp/cmake.sh https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh
RUN mkdir --parents "$CMAKE_INSTALL_DIR" \
    && sh /tmp/cmake.sh --skip-license --prefix="$CMAKE_INSTALL_DIR" \
    && ln --symbolic --force "$CMAKE_INSTALL_DIR/bin/"* /usr/local/bin

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install --yes sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME
