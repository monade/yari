set -e
cc -Wall -Wextra -O2 -DDEBUG -I/opt/homebrew/Cellar/raylib/5.5/include -Isrc/raycast -o build/ray main/main.c src/raycast/raycast.c src/raycast/platform/raylib/inputs_raylib.c src/raycast/platform/raylib/renderer_raylib.c src/raycast/physics.c src/raycast/colors.c -L/opt/homebrew/Cellar/raylib/5.5/lib -lraylib -lm
./build/ray