FROM ubuntu:18.04
ARG USER_ID
ARG GROUP_ID
ARG USER_NAME

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y apt-utils

RUN apt-get update -qq 
RUN apt-get -y install \
  autoconf \
  automake \
  build-essential \
  cmake \
  git-core \
  libass-dev \
  libfreetype6-dev \
  libgnutls28-dev \
  libmp3lame-dev \
  libsdl2-dev \
  libtool \
  libva-dev \
  libvdpau-dev \
  libvorbis-dev \
  libxcb1-dev \
  libxcb-shm0-dev \
  libxcb-xfixes0-dev \
  meson \
  ninja-build \
  pkg-config \
  texinfo \
  wget \
  yasm \
  zlib1g-dev \
  locales \
  nasm \
  libx264-dev \
  libx265-dev libnuma-dev \
  libvpx-dev \
  libfdk-aac-dev \
  libopus-dev \
  python3-pip
  
RUN pip3 install --user meson
RUN apt-get -y install sudo
RUN groupadd -g "$GROUP_ID" -o "$USER_NAME"
RUN useradd -r -u "$USER_ID" -m -g "$GROUP_ID" "$USER_NAME"

RUN adduser $USER_NAME sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

EXPOSE 8088

RUN chown $USER_NAME /home/$USER_NAME -R
RUN chmod 0750 /home/$USER_NAME -R

RUN apt-get -y install vim-common --fix-missing
RUN apt-get -y install texinfo cmake pxz intltool help2man
RUN apt-get -y install gawk
RUN apt-get -y install libtool-bin
RUN apt-get -y install libncurses-dev
RUN apt-get install -y openssh-client

RUN mkdir -p /home/$USER_NAME/.ssh
RUN chown -R $USER_NAME:$USER_NAME /home/$USER_NAME/.ssh

RUN locale-gen en_US.UTF-8
RUN sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && \
    locale-gen
ENV LANG en_US.UTF-8  
ENV LANGUAGE en_US:en  
ENV LC_ALL en_US.UTF-8

USER $USER_NAME

ENV PATH="${PATH}:/home/${USER_NAME}/bin:/home/${USER_NAME}/depot_tools"
RUN echo "export PATH = ${PATH}"
RUN echo "export PS1=\"[Container \\\\t \\\\w] \"" >> /home/${USER_NAME}/.bashrc
RUN mkdir -p ~/ffmpeg_sources ~/bin

RUN cd ~/ffmpeg_sources && \
	git clone https://github.com/FFmpeg/FFmpeg.git

#libaom
RUN cd ~/ffmpeg_sources && \
	git -C aom pull 2> /dev/null || git clone --depth 1 https://aomedia.googlesource.com/aom && \
	mkdir -p aom_build && \
	cd aom_build && \
	PATH="$HOME/bin:$PATH" cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$HOME/ffmpeg_build" -DENABLE_TESTS=OFF -DENABLE_NASM=on ../aom && \
	PATH="$HOME/bin:$PATH" make && \
	make install

#libsvtav1
RUN cd ~/ffmpeg_sources && \
	git -C SVT-AV1 pull 2> /dev/null || git clone https://gitlab.com/AOMediaCodec/SVT-AV1.git && \
	mkdir -p SVT-AV1/build && \
	cd SVT-AV1/build && \
	PATH="$HOME/bin:$PATH" cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$HOME/ffmpeg_build" -DCMAKE_BUILD_TYPE=Release -DBUILD_DEC=OFF -DBUILD_SHARED_LIBS=OFF .. && \
	PATH="$HOME/bin:$PATH" make && \
	make install

RUN sudo apt-get update
RUN sudo apt-get install libccd-dev -y
	
#libvmaf
#RUN cd ~/ffmpeg_sources && \
#	wget https://github.com/Netflix/vmaf/archive/v2.1.1.tar.gz && \
#	tar xvf v2.1.1.tar.gz && \
#	mkdir -p vmaf-2.1.1/libvmaf/build &&\
#	cd vmaf-2.1.1/libvmaf/build && \
#	meson setup -Denable_tests=false -Denable_docs=false --buildtype=release --default-library=static .. --prefix "$HOME/ffmpeg_build" --bindir="$HOME/bin" --libdir="$HOME/ffmpeg_build/lib" && \
#	ninja && \
#	ninja install

RUN sudo apt-get -y install libunistring-dev
	
#FFmpeg
RUN cd ~/ffmpeg_sources && \
wget -O ffmpeg-snapshot.tar.bz2 https://ffmpeg.org/releases/ffmpeg-snapshot.tar.bz2 && \
tar xjvf ffmpeg-snapshot.tar.bz2 && \
cd ffmpeg && \
PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" ./configure \
	  --prefix="$HOME/ffmpeg_build" \
	  --pkg-config-flags="--static" \
	  --extra-cflags="-I$HOME/ffmpeg_build/include" \
	  --extra-ldflags="-L$HOME/ffmpeg_build/lib" \
	  --extra-libs="-lpthread -lm" \
	  --ld="g++" \
	  --bindir="$HOME/bin" \
	  --enable-gpl \
	  --enable-gnutls \
	  --enable-libaom \
	  --enable-libass \
	  --enable-libfdk-aac \
	  --enable-libfreetype \
	  --enable-libmp3lame \
	  --enable-libopus \
	  --enable-libsvtav1 \
	  --disable-libdav1d \
	  --enable-libvorbis \
	  --enable-libvpx \
	  --enable-libx264 \
	  --enable-libx265 \
	  --enable-shared \
	  --disable-static \
	  --enable-nonfree && \
	PATH="$HOME/bin:$PATH" make && \
	make install && \
	hash -r