#! /usr/bin/sh

cc main.c -ISDL3/include -LSDL3/lib -lSDL3 -g -Wall -Wextra -Wpedantic -o main && export LD_LIBRARY_PATH=./SDL3/lib && ./main
