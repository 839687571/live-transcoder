
ARG     CUDA_VERSION="10.0"
ARG     PKG_CONFIG_PATH=/opt/ffmpeg/lib/pkgconfig
ARG     LD_LIBRARY_PATH=/opt/ffmpeg/lib
ARG     PREFIX=/opt/ffmpeg
ARG     MAKEFLAGS="-j2"
#ARG     CUDA="--enable-cuda-sdk  --enable-cuvid --enable-libnpp" 
ARG     CUDA="" 

FROM nvidia/cuda:${CUDA_VERSION}-devel 


ENV         FFMPEG_VERSION=4.1     \
            LAME_VERSION=3.99.5       \
            OGG_VERSION=1.3.3         \
            OPUS_VERSION=1.2          \
            THEORA_VERSION=1.1.1      \
            VORBIS_VERSION=1.3.5      \
            VPX_VERSION=1.8.0         \
            X264_VERSION=20170226-2245-stable \   
            X265_VERSION=3.0          \
            SRC=/usr/local

RUN apt-get -yqq update && apt-get install curl -yq wget vim cmake --no-install-recommends ca-certificates expat  libgomp1 wget nano git build-essential nasm yasm pkg-config autoconf gettext-base  gettext && \
        rm -rf /var/lib/apt/lists/*


#build nvenc
RUN git clone   https://github.com/FFmpeg/nv-codec-headers /build/nv-codec-headers && \
  cd /build/nv-codec-headers &&\
  make  && \
  make install  && \
  cd /build && rm -rf nv-codec-headers


## x264 http://www.videolan.org/developers/x264.html
RUN \
        DIR=/build/x264 && \
        mkdir -p ${DIR} && \
        cd ${DIR} && \
        curl -sL https://download.videolan.org/pub/videolan/x264/snapshots/x264-snapshot-${X264_VERSION}.tar.bz2 | \
        tar -jx --strip-components=1 && \
        ./configure --prefix="${PREFIX}" --enable-shared --enable-pic --disable-cli && \
        make && \
        make install && \
        rm -rf ${DIR}
### x265 http://x265.org/
RUN \
        DIR=/build/x265 && \
        mkdir -p ${DIR} && \
        cd ${DIR} && \
        curl -sL https://download.videolan.org/pub/videolan/x265/x265_${X265_VERSION}.tar.gz  | \
        tar -zx && \
        cd x265_${X265_VERSION}/build/linux && \
        sed -i "/-DEXTRA_LIB/ s/$/ -DCMAKE_INSTALL_PREFIX=\${PREFIX}/" multilib.sh && \
        sed -i "/^cmake/ s/$/ -DENABLE_CLI=OFF/" multilib.sh && \
        ./multilib.sh && \
        make -C 8bit install && \
        rm -rf ${DIR}
### libvpx https://www.webmproject.org/code/
RUN \
        DIR=/build/vpx && \
        mkdir -p ${DIR} && \
        cd ${DIR} && \
        curl -sL https://codeload.github.com/webmproject/libvpx/tar.gz/v${VPX_VERSION} | \
        tar -zx --strip-components=1 && \
        ./configure --prefix="${PREFIX}" --enable-vp8 --enable-vp9 --enable-vp9-highbitdepth --enable-pic --enable-shared \
        --disable-debug --disable-examples --disable-docs --disable-install-bins  && \
        make && \
        make install && \
        rm -rf ${DIR}
### libmp3lame http://lame.sourceforge.net/
RUN \
        DIR=/build/lame && \
        mkdir -p ${DIR} && \
        cd ${DIR} && \
        curl -sL https://kent.dl.sourceforge.net/project/lame/lame/$(echo ${LAME_VERSION} | sed -e 's/[^0-9]*\([0-9]*\)[.]\([0-9]*\)[.]\([0-9]*\)\([0-9A-Za-z-]*\)/\1.\2/')/lame-${LAME_VERSION}.tar.gz | \
        tar -zx --strip-components=1 && \
        ./configure --prefix="${PREFIX}" --bindir="${PREFIX}/bin" --enable-shared --enable-nasm --enable-pic --disable-frontend && \
        make && \
        make install && \
        rm -rf ${DIR}

## ffmpeg https://ffmpeg.org/
RUN  \
        DIR=/build/ffmpeg && mkdir -p ${DIR} && cd ${DIR} && \
        curl -sLO https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.bz2 && \
        tar -jx --strip-components=1 -f ffmpeg-${FFMPEG_VERSION}.tar.bz2

RUN \
        DIR=/build/ffmpeg && mkdir -p ${DIR} && cd ${DIR} && \
        ./configure \
        --disable-debug \
        --disable-doc \
        --disable-ffplay \
        --enable-shared \ 
        --enable-gpl \
        --enable-libmp3lame \
        --enable-libvpx \
        --enable-libx264 \
        --enable-nonfree \
        --enable-postproc \
        --enable-version3 \
        ${CUDA} \
        --extra-cflags="-I${PREFIX}/include" \
        --extra-ldflags="-L${PREFIX}/lib" \
        --extra-cflags="-I/usr/local/cuda/include/" \
        --extra-ldflags=-L/usr/local/cuda/lib64/ \
        --extra-libs=-ldl \
        --prefix="${PREFIX}" && \
        make && \
        make install

ENV  FFMPEG_LIB_DIR  "/build/ffmpeg"

WORKDIR /build

COPY . .