#! /usr/bin/sh

SDL_VERSION="3.2.8"
SYS_INFO="$(uname -rmo | tr ' ' '_' | tr '/' '-')"
SCRIPT_DIR="$(dirname -- $(readlink -f -- "$0"))"

if [ $# -eq 0 ]; then
    echo $(readlink -f "$SCRIPT_DIR/SDL_${SDL_VERSION}-$SYS_INFO")
elif [ "$1" = "version" ]; then
    echo "$SDL_VERSION"
elif [ "$1" = "system" ]; then
    echo "$SYS_INFO"
fi
