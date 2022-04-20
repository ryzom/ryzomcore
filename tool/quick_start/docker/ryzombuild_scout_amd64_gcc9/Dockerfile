FROM --platform=linux/amd64 steamrt_scout_amd64:latest
MAINTAINER Ryzom Core <https://wiki.ryzom.dev/>

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90 \
  && update-alternatives --set gcc "/usr/bin/gcc-9" \
  && update-alternatives --set g++ "/usr/bin/g++-9"

RUN rm /usr/bin/cc \
  && ln -s /usr/lib/gcc-9/bin/gcc-9 /usr/bin/cc

RUN update-alternatives --install /usr/bin/ld ld /usr/bin/ld.gold-2.30 90 \
  && update-alternatives --set ld "/usr/bin/ld.gold-2.30"

RUN rm /usr/bin/addr2line && ln -s /usr/bin/addr2line-2.30 /usr/bin/addr2line \
  && rm /usr/bin/ar && ln -s /usr/bin/ar-2.30 /usr/bin/ar \
  && rm /usr/bin/as && ln -s /usr/bin/as-2.30 /usr/bin/as \
  && rm /usr/bin/c++filt && ln -s /usr/bin/c++filt-2.30 /usr/bin/c++filt \
  && ln -s /usr/bin/dwp-2.30 /usr/bin/dwp \
  && rm /usr/bin/elfedit && ln -s /usr/bin/elfedit-2.30 /usr/bin/elfedit \
  && rm /usr/bin/gold && ln -s /usr/bin/gold-2.30 /usr/bin/gold \
  && rm /usr/bin/gprof && ln -s /usr/bin/gprof-2.30 /usr/bin/gprof \
  && rm /usr/bin/nm && ln -s /usr/bin/nm-2.30 /usr/bin/nm \
  && rm /usr/bin/objcopy && ln -s /usr/bin/objcopy-2.30 /usr/bin/objcopy \
  && rm /usr/bin/objdump && ln -s /usr/bin/objdump-2.30 /usr/bin/objdump \
  && rm /usr/bin/ranlib && ln -s /usr/bin/ranlib-2.30 /usr/bin/ranlib \
  && rm /usr/bin/readelf && ln -s /usr/bin/readelf-2.30 /usr/bin/readelf \
  && rm /usr/bin/size && ln -s /usr/bin/size-2.30 /usr/bin/size \
  && rm /usr/bin/strings && ln -s /usr/bin/strings-2.30 /usr/bin/strings \
  && rm /usr/bin/strip && ln -s /usr/bin/strip-2.30 /usr/bin/strip

RUN mkdir -p /mnt/nel

WORKDIR /mnt/nel
