CFLAGS = -Wall -Wextra -O2 -DDEBUG $(shell pkg-config --cflags raylib)
LIBS = $(shell pkg-config --libs raylib) -lm

all: ray

build/assets_packer: src/tools/assets_packer.c
	$(CC) -o build/assets_packer src/tools/assets_packer.c

main/assets.h: build/assets_packer assets
	build/assets_packer assets main/assets.h

build/colors.o: src/raycast/colors.c
	$(CC) -c src/raycast/colors.c -o build/colors.o

build/physics.o: src/raycast/physics.c
	$(CC) -c src/raycast/physics.c -o build/physics.o

build/renderer.o: src/raycast/platform/raylib/renderer.c
	$(CC) $(CFLAGS) -c src/raycast/platform/raylib/renderer.c -o build/renderer.o

build/inputs.o: src/raycast/platform/raylib/inputs.c
	$(CC) $(CFLAGS) -c src/raycast/platform/raylib/inputs.c -o build/inputs.o

build/raycast.o: src/raycast/raycast.c
	$(CC) -c src/raycast/raycast.c -o build/raycast.o

raycast_lib: build/colors.o build/physics.o build/renderer.o build/inputs.o build/raycast.o
	ar rcs build/libraycast.a build/colors.o build/physics.o build/renderer.o build/inputs.o build/raycast.o

ray: raycast_lib main/main.c main/assets.h
	$(CC) -Isrc/raycast main/main.c -o build/ray -Lbuild -lraycast $(LIBS)

run: ray
	build/ray