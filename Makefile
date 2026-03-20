RAYCAST_CFLAGS = -Wall -Wextra -O2
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib) -lm

MAIN_CFLAGS = -Wall -Wextra -O2 -DDEBUG -I./src/raycast
MAIN_LIBS = -Lbuild/raycast -lraycast_raylib $(RAYLIB_LIBS)

WASM_CFLAGS = --target=wasm32 -O2 -flto -DWASM -isystem ./src/raycast/platform/wasm/wasm_sysroot -I./src/raycast -nostdlib -ffreestanding
WASM_LIBS = --no-entry --export-dynamic --allow-undefined --initial-memory=16777216 -z stack-size=65536 -Lbuild/raycast -lraycast_wasm


all: ray wasm esp32-build

# Build object files into a static library raycast.a
# Mac/Linux
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
	$(CC) $(WASM_CFLAGS) -c src/raycast/raycast.c -o build/raycast_wasm.o
build/colors_wasm.o: src/raycast/colors.c
	$(CC) $(WASM_CFLAGS) -c src/raycast/colors.c -o build/colors_wasm.o
build/renderer_wasm.o: src/raycast/platform/wasm/renderer.c
	$(CC) $(WASM_CFLAGS) -c src/raycast/platform/wasm/renderer.c -o build/renderer_wasm.o
build/inputs_wasm.o: src/raycast/platform/wasm/inputs.c
	$(CC) $(WASM_CFLAGS) -c src/raycast/platform/wasm/inputs.c -o build/inputs_wasm.o
build/wasm_runtime.o: src/raycast/platform/wasm/wasm_runtime.c
	$(CC) $(WASM_CFLAGS) -c src/raycast/platform/wasm/wasm_runtime.c -o build/wasm_runtime.o
build/physics_wasm.o: src/raycast/physics.c
	$(CC) $(WASM_CFLAGS) -c src/raycast/physics.c -o build/physics_wasm.o
build/raycast/libraycast_wasm.a: build/raycast_wasm.o build/colors_wasm.o build/inputs_wasm.o build/renderer_wasm.o build/wasm_runtime.o build/physics_wasm.o
	@mkdir -p build/raycast
	ar rcs build/raycast/libraycast_wasm.a build/raycast_wasm.o build/colors_wasm.o build/inputs_wasm.o build/renderer_wasm.o build/wasm_runtime.o build/physics_wasm.o

build/assets_packer: src/tools/assets_packer.c
	$(CC) -o build/assets_packer src/tools/assets_packer.c

assets: build/assets_packer
	build/assets_packer assets example/game/main/assets.h

build/ray: assets build/raycast/libraycast_raylib.a example/game/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/ray example/game/main/main.c $(MAIN_LIBS)

ray: build/ray

run: ray
	build/ray

build/example_game.o: assets example/game/main/main.c
	$(CC) $(WASM_CFLAGS) -c example/game/main/main.c -o build/example_game.o

example/web/game.wasm: build/example_game.o build/raycast/libraycast_wasm.a
	wasm-ld $(WASM_LIBS) build/example_game.o -o example/web/game.wasm

wasm: example/web/game.wasm

run-wasm: wasm
	npx serve example/web

# ESP32
esp32-build:
	cd example/game && . ~/esp/v5.5.*/esp-idf/export.sh && idf.py build

esp32-flash: esp32-build
	cd example/game && . ~/esp/v5.5.*/esp-idf/export.sh && idf.py flash

esp32-monitor:
	cd example/game && . ~/esp/v5.5.*/esp-idf/export.sh && idf.py monitor

esp32-flash-monitor: esp32-flash
	cd example/game && . ~/esp/v5.5.*/esp-idf/export.sh && idf.py monitor

esp32-clean:
	cd example/game && . ~/esp/v5.5.*/esp-idf/export.sh && idf.py fullclean
