#!/usr/bin/env bash

set -e

mkdir -p build

# download raylib if it doesn't exist in external/win
if [ ! -d "external/win" ]; then
    echo "Downloading raylib for Windows..."
    mkdir -p external/win
    curl -L https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip -o external/win/raylib.zip
    unzip external/win/raylib.zip -d external/win
    mv external/win/raylib-5.5_win64_mingw-w64 external/win/raylib
    rm external/win/raylib.zip
fi

CFLAGS="-Wall -Wextra -O2 -static -Isrc/yari -Iexternal/win/raylib/include"
LIBS="-Lexternal/win/raylib/lib/ -l:libraylib.a -lopengl32 -lgdi32 -lwinmm"
x86_64-w64-mingw32-gcc $CFLAGS -o build/ray.exe example/fps/main/main.c src/yari/*.c src/yari/platform/raylib/*.c $LIBS