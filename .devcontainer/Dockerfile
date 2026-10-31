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
        libboost-dev \
        libcurl4-openssl-dev \
        libfreetype6-dev \
        libgif-dev \
        libgsf-1-dev \
        libjpeg-dev \
        liblua5.2-dev \
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
        libxslt1-dev \
        xsltproc \
        mingw-w64 \
        mingw-w64-tools \
        nasm \
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


# vcpkg
RUN apt install --yes \
        curl zip unzip tar

# install msquic
RUN wget --output-document=/tmp/microsoft.deb https://packages.microsoft.com/config/ubuntu/24.04/packages-microsoft-prod.deb \
    && dpkg --install /tmp/microsoft.deb \
    && apt update \
    && apt install --yes \
      libmsquic=2.6.3 \
    && wget https://raw.githubusercontent.com/microsoft/msquic/v2.6.3/src/inc/msquic.h \
    && wget https://raw.githubusercontent.com/microsoft/msquic/v2.6.3/src/inc/msquic_posix.h \
    && wget https://raw.githubusercontent.com/microsoft/msquic/v2.6.3/src/inc/quic_sal_stub.h \
    && sudo mv msquic.h msquic_posix.h quic_sal_stub.h /usr/include/


ARG CMAKE_VERSION=3.31.11
ARG CMAKE_INSTALL_DIR=/opt/cmake-$CMAKE_VERSION
ARG USERNAME=ryzom
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN wget --output-document=/tmp/cmake.sh https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh
RUN mkdir --parents "$CMAKE_INSTALL_DIR" \
    && sh /tmp/cmake.sh --skip-license --prefix="$CMAKE_INSTALL_DIR" \
    && ln --symbolic --force "$CMAKE_INSTALL_DIR/bin/"* /usr/local/bin

# ryzom/luabind as a static library, built like .github/scripts/build-luabind.sh does for the CI jobs;
# the luabind package of Ubuntu does not work
ARG LUABIND_REV=6cb0688f78b753ce94df528ac640090251043759
RUN mkdir /tmp/luabind \
    && curl -L --fail --retry 3 https://github.com/ryzom/luabind/archive/${LUABIND_REV}.tar.gz | tar -xz -C /tmp/luabind --strip-components=1 \
    && cmake -S /tmp/luabind -B /tmp/luabind/build -DCMAKE_BUILD_TYPE=Release -DLUABIND_DYNAMIC_LINK=OFF -DBUILD_TESTING=OFF \
    && cmake --build /tmp/luabind/build --parallel $(nproc) \
    && cmake --install /tmp/luabind/build \
    && rm -rf /tmp/luabind

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install --yes sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME

ENV VCPKG_ROOT="/home/$USERNAME/vcpkg"
ENV PATH="$PATH:$VCPKG_ROOT"

RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
    && $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics \
    && vcpkg --version
