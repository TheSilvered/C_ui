#! /usr/bin/sh

sh SDL3/buildSDL.sh

if [ ! -d build ]; then
    mkdir build
fi

SDL_PATH="$(SDL3/SDLpath.sh)"
INCLUDE=${SDL_PATH}/include
LIB=${SDL_PATH}/lib
FLAGS="-g -Wall -Wextra -Wpedantic"

cc main.c -I$INCLUDE -L$LIB -lSDL3 $FLAGS -o build/main && export LD_LIBRARY_PATH=$LIB && ./build/main
