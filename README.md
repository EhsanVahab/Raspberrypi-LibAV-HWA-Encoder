
![myimagetitle](https://github.com/EhsanVahab/Raspberrypi-LibAV-HWA-Encoder/blob/main/logo.jpg?raw=true "myimagetitle")
# Raspberrypi-LibAV-HWA-Encoder
Encoding Raspberrypi Cameras's Frame to H264 with LibAV and HWA of Raspberrypi

1. install raspbian on your SD Card.
2. install dependencies FFMPEG as following:

```bash
sudo sudo apt-get -y install \
    autoconf \
    automake \
    build-essential \
    cmake \
    doxygen \
    git \
    graphviz \
    imagemagick \
    libasound2-dev \
    libass-dev \
    libavcodec-dev \
    libavdevice-dev \
    libavfilter-dev \
    libavformat-dev \
    libavutil-dev \
    libdrm-dev \
    libfreetype6-dev \
    libgmp-dev \
    libmp3lame-dev \
    libopencore-amrnb-dev \
    libopencore-amrwb-dev \
    libopus-dev \
    librtmp-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-net-dev \
    libsdl2-ttf-dev \
    libsnappy-dev \
    libsoxr-dev \
    libssh-dev \
    libtool \
    libv4l-dev \
    libva-dev \
    libvdpau-dev \
    libvo-amrwbenc-dev \
    libvorbis-dev \
    libwebp-dev \
    libx264-dev \
    libx265-dev \
    libxcb-shape0-dev \
    libxcb-shm0-dev \
    libxcb-xfixes0-dev \
    libxcb1-dev \
    libxml2-dev \
    lzma-dev \
    meson \
    nasm \
    pkg-config \
    python3-dev \
    python3-pip \
    texinfo \
    wget \
    yasm \
    zlib1g-dev
```

#### To disable remove --enable-libkvazaar

   ```bash
 git clone --depth 1 "https://github.com/ultravideo/kvazaar.git" 
        && ./autogen.sh \
        && ./configure \
        && make -j$(2) \
        && sudo make install
```

#### To disable, remove --enable-libzimg

```bash
wget https://github.com/sekrit-twc/zimg/archive/refs/tags/release-3.0.1.tar.gz
tar -xvzf release-3.0.1.tar.gz
cd zimg-release-3.0.1

    ./autogen.sh \
      && ./configure \
      && make -j$(2) \
      && sudo make install
```



3. download and extract ffmpeg **4.3.1**
4. cp omx.c  /yourpath/FFmpeg-n4.3.1/libavcodec/omx.c #pathing the ffmpeg bug to use HWA
5. cd /yourpath/FFmpeg-n4.3.1 and run the following command:

```bash
./configure \
    --extra-cflags="-I/usr/local/include" \
    --extra-ldflags="-L/usr/local/lib" \
    --extra-libs="-lpthread -lm -latomic" \
    --arch=armel \
    --enable-gmp \
    --enable-gpl \
    --enable-libfreetype \
    --enable-avresample \
    --enable-libkvazaar \
    --enable-libmp3lame \
    --enable-libopencore-amrnb \
    --enable-libopencore-amrwb \
    --enable-libopus \
    --enable-librtmp \
    --enable-libsnappy \
    --enable-libsoxr \
    --enable-libvorbis \
    --enable-libv4l2 \
    --enable-libzimg \
    --enable-libwebp \
    --enable-libx264 \
    --enable-libx265 \
    --enable-libxml2 \
    --enable-mmal \
    --enable-nonfree \
    --enable-omx \
    --enable-omx-rpi \
    --enable-version3 \
    --target-os=linux \
    --enable-pthreads \
    --enable-openssl \
    --enable-shared \
    --enable-hardcoded-tables
```


6. make && sudo make install
7.  sudo apt-get install libopencv-dev
8. Install raspberrypi camera c++ library from following link https://github.com/cedricve/raspicam/
9. git clone https://github.com/EhsanVahab/Raspberrypi-LibAV-HWA-Encoder.git
11. cd  Raspberrypi-LibAV-HWA-Encoder
12. make
13. ./Encoder

the file ***sample.h264*** is ready to use


## Benchmark on Raspberrypi 3b+
| Resolution  |   Bit Rate|  CPU Usage | Frame Rate  |
| :------------: | :------------: | :------------: | :------------: |
| 1280x960  | 100,000 | ~21%  | 16  |
| 800x600  | 100,000  | ~19%  | 36  |
| 640x480  | 100,000  | ~18%  | 50  |
| 1280x960  | 200,000 | ~24%  | 16  |
| 800x600  | 200,000  |  ~22% | 36  |
| 640x480  | 200,000  |  ~20% |  50 |




