#!/usr/bin/env bash

set -e

script_path=${BASH_SOURCE[0]}
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $script_dir

docker build -t antimony-builder .
