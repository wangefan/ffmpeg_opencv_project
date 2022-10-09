#!/bin/sh

export UN=$(id -nu)
FFMPEG_FOLDER=$(dirname "$PWD")
echo "FFMPEG_FOLDER = ${FFMPEG_FOLDER}"
FFMPEG_FOLDER_NAME=$(basename ${FFMPEG_FOLDER})
echo "FFMPEG_FOLDER_NAME = ${FFMPEG_FOLDER_NAME}"
sudo docker run -it \
    -v ~/.ssh:/home/${UN}/.ssh:ro \
    -v ~/.ssh_host:/home/${UN}/.ssh_host:ro \
	-v ~/bin:/home/${UN}/bin \
	-v ${FFMPEG_FOLDER}:/home/${UN}/${FFMPEG_FOLDER_NAME}/ \
    -w /home/${UN}/${FFMPEG_FOLDER_NAME} \
    build-ffmpeg-sys:latest /bin/bash
