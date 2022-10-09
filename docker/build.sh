#!/bin/sh

export U=$(id -u)
export G=$(id -g)
export UN=$(id -nu)
sudo docker build -t build-ffmpeg-sys --build-arg USER_ID=${U} --build-arg GROUP_ID=${G} --build-arg USER_NAME=${UN} . 

