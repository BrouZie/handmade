#!/bin/sh
cd "$(dirname "$0")"
mkdir -p build
gcc -g -O0 -Wall -Wno-unused-function -Wno-unused-variable \
    $(pkg-config --cflags sdl2) \
    src/sdl_handmade.cpp -o build/handmadehero \
    $(pkg-config --libs sdl2)
# ./build/handmadehero
