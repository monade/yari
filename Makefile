RAYCAST_CFLAGS = -Wall -Wextra -O2
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib) -lm

MAIN_CFLAGS = -Wall -Wextra -O2 -I./src/raycast
MAIN_LIBS = -Lbuild/raycast -lraycast_raylib

WASM_CC = clang
WASM_CFLAGS = --target=wasm32 -O2 -flto -DWASM -isystem ./src/raycast/platform/wasm/wasm_sysroot -I./src/raycast -nostdlib -ffreestanding
WASM_LIBS = --no-entry --allow-undefined --initial-memory=16777216 -z stack-size=65536 \
	--export=wasm_init --export=wasm_frame \
	--export=get_framebuffer --export=get_fb_width --export=get_fb_height

WASM_OBJS = build/raycast_wasm.o build/colors_wasm.o build/inputs_wasm.o build/renderer_wasm.o build/wasm_runtime.o build/physics_wasm.o

ESP32_HOME = ~/esp/v5.5.*/esp-idf

all: ray wasm esp32-build

# Raylib
build/raycast.o: src/raycast/raycast.c
	$(CC) $(RAYCAST_CFLAGS) -c src/raycast/raycast.c -o build/raycast.o
build/colors.o: src/raycast/colors.c
	$(CC) $(RAYCAST_CFLAGS) -c src/raycast/colors.c -o build/colors.o
build/inputs.o: src/raycast/platform/raylib/inputs.c
	$(CC) $(RAYCAST_CFLAGS) $(RAYLIB_CFLAGS) -c src/raycast/platform/raylib/inputs.c -o build/inputs.o
build/renderer.o: src/raycast/platform/raylib/renderer.c
	$(CC) $(RAYCAST_CFLAGS) $(RAYLIB_CFLAGS) -c src/raycast/platform/raylib/renderer.c -o build/renderer.o
build/physics.o: src/raycast/physics.c
	$(CC) $(RAYCAST_CFLAGS) -c src/raycast/physics.c -o build/physics.o
build/raycast/libraycast_raylib.a: build/raycast.o build/colors.o build/inputs.o build/renderer.o build/physics.o
	@mkdir -p build/raycast
	ar rcs build/raycast/libraycast_raylib.a build/raycast.o build/colors.o build/inputs.o build/renderer.o build/physics.o

# WASM
build/raycast_wasm.o: src/raycast/raycast.c
	$(WASM_CC) $(WASM_CFLAGS) -c src/raycast/raycast.c -o build/raycast_wasm.o
build/colors_wasm.o: src/raycast/colors.c
	$(WASM_CC) $(WASM_CFLAGS) -c src/raycast/colors.c -o build/colors_wasm.o
build/renderer_wasm.o: src/raycast/platform/wasm/renderer.c
	$(WASM_CC) $(WASM_CFLAGS) -c src/raycast/platform/wasm/renderer.c -o build/renderer_wasm.o
build/inputs_wasm.o: src/raycast/platform/wasm/inputs.c
	$(WASM_CC) $(WASM_CFLAGS) -c src/raycast/platform/wasm/inputs.c -o build/inputs_wasm.o
build/wasm_runtime.o: src/raycast/platform/wasm/wasm_runtime.c
	$(WASM_CC) $(WASM_CFLAGS) -c src/raycast/platform/wasm/wasm_runtime.c -o build/wasm_runtime.o
build/physics_wasm.o: src/raycast/physics.c
	$(WASM_CC) $(WASM_CFLAGS) -c src/raycast/physics.c -o build/physics_wasm.o
build/raycast/libraycast_wasm.a: build/raycast_wasm.o build/colors_wasm.o build/inputs_wasm.o build/renderer_wasm.o build/wasm_runtime.o build/physics_wasm.o
	@mkdir -p build/raycast
	ar rcs build/raycast/libraycast_wasm.a build/raycast_wasm.o build/colors_wasm.o build/inputs_wasm.o build/renderer_wasm.o build/wasm_runtime.o build/physics_wasm.o

build/assets_packer: src/tools/assets_packer.c
	$(CC) -o build/assets_packer src/tools/assets_packer.c -lm

assets: build/assets_packer
	build/assets_packer assets example/game/main/assets.h

build/ray: assets build/raycast/libraycast_raylib.a example/game/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/ray example/game/main/main.c $(MAIN_LIBS) $(RAYLIB_LIBS)

ray: build/ray

run: ray
	build/ray

build/example_game.o: assets example/game/main/main.c
	$(WASM_CC) $(WASM_CFLAGS) -c example/game/main/main.c -o build/example_game.o

docs/game.wasm: build/example_game.o $(WASM_OBJS)
	wasm-ld $(WASM_LIBS) build/example_game.o $(WASM_OBJS) -o docs/game.wasm

wasm: docs/game.wasm

run-wasm: wasm
	npx serve docs

# ESP32
esp32-build:
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py build

esp32-flash: esp32-build
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py flash

esp32-monitor:
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-flash-monitor: esp32-flash
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-clean:
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py fullclean
