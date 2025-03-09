#! /usr/bin/sh

sh SDL3/buildSDL.sh

SDL_PATH="$(SDL3/SDLpath.sh)"
INCLUDE=${SDL_PATH}/include
LIB=${SDL_PATH}/lib

cc main.c -I$INCLUDE -L$LIB -lSDL3 -g -Wall -Wextra -Wpedantic -o build/main && export LD_LIBRARY_PATH=$LIB && ./build/main
