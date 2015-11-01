#!/usr/bin/env bash

set -e

script_path=${BASH_SOURCE[0]}
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $script_dir

app_dir=/opt/builder/antimony/build/app

mkdir -p share
docker run --rm -i -t -v $PWD/share:$app_dir/share \
       --privileged \
       -e DISPLAY=$DISPLAY \
       -v /tmp/.X11-unix:/tmp/.X11-unix \
       -v /dev/dri:/dev/dri:rw \
       antimony-builder $app_dir/antimony
