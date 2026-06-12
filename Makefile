RAYCAST_CFLAGS = -Wall -Wextra -O2
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib) -lm
RAYLIB_WEB_VERSION ?= 5.5
RAYLIB_WEB_PATH ?= external/raylib
RAYLIB_WEB_LIB = $(RAYLIB_WEB_PATH)/src/libraylib.a
RAYLIB_WEB_RAYCAST_LIB = build/raycast/libraycast_raylib_web.a
RAYLIB_WEB_STAMP = build/raylib-web.stamp
RAYLIB_WEB_OUTPUT = docs/index.html
RAYLIB_WEB_SHELL = src/raycast/platform/raylib/shell.html
RAYLIB_WEB_CFLAGS = -Wall -Wextra -O2 -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I./src/raycast -I$(RAYLIB_WEB_PATH)/src
RAYLIB_WEB_LDFLAGS = -s USE_GLFW=3 -s ASSERTIONS=1 -s INITIAL_MEMORY=33554432 --shell-file $(RAYLIB_WEB_SHELL)
RAYLIB_WEB_RAYCAST_OBJS = \
	build/raycast_web.o \
	build/colors_web.o \
	build/physics_web.o \
	build/renderer_web.o \
	build/inputs_web.o
RAYLIB_WEB_DEPS = \
	example/game/main/assets.h \
	example/game/main/level.h \
	src/raycast/colors.h \
	src/raycast/inputs.h \
	src/raycast/physics.h \
	src/raycast/raycast.h \
	src/raycast/raymath.h \
	src/raycast/renderer.h \
	$(RAYLIB_WEB_SHELL)

MAIN_CFLAGS = -Wall -Wextra -O2 -I./src/raycast
MAIN_LIBS = -Lbuild/raycast -lraycast_raylib

EMCC = emcc
EMAR = emar

ESP32_HOME = ~/esp/v5.5.*/esp-idf

.PHONY: all
all: ray wasm esp32-build

# Lib raycast

## Raylib
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

## Raylib web
build/raycast_web.o: src/raycast/raycast.c src/raycast/raycast.h src/raycast/renderer.h src/raycast/inputs.h src/raycast/colors.h src/raycast/physics.h
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/raycast/raycast.c -o build/raycast_web.o
build/colors_web.o: src/raycast/colors.c src/raycast/colors.h
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/raycast/colors.c -o build/colors_web.o
build/physics_web.o: src/raycast/physics.c src/raycast/physics.h src/raycast/raycast.h
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/raycast/physics.c -o build/physics_web.o
build/renderer_web.o: src/raycast/platform/raylib/renderer.c src/raycast/renderer.h | $(RAYLIB_WEB_PATH)
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/raycast/platform/raylib/renderer.c -o build/renderer_web.o
build/inputs_web.o: src/raycast/platform/raylib/inputs.c src/raycast/inputs.h | $(RAYLIB_WEB_PATH)
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/raycast/platform/raylib/inputs.c -o build/inputs_web.o
$(RAYLIB_WEB_RAYCAST_LIB): $(RAYLIB_WEB_RAYCAST_OBJS)
	@mkdir -p build/raycast
	$(EMAR) rcs $(RAYLIB_WEB_RAYCAST_LIB) $(RAYLIB_WEB_RAYCAST_OBJS)

# Tools
build/assets_packer: src/tools/assets_packer.c
	$(CC) -o build/assets_packer src/tools/assets_packer.c -lm

example/game/main/assets.h: build/assets_packer assets/*.png
	build/assets_packer assets example/game/main/assets.h

build/map_builder: src/tools/map_builder.c src/tools/raygui.h
	$(CC) -Wall -Wextra -O2 $(RAYLIB_CFLAGS) -I./src/tools -o build/map_builder src/tools/map_builder.c $(RAYLIB_LIBS)

.PHONY: assets map-builder run-map-builder 
assets: example/game/main/assets.h
map-builder: build/map_builder
run-map-builder: map-builder
	build/map_builder assets example/game/main/level.h

# Examples

## Raylib examples

## Raylib examples

### base example
build/ray-base: build/raycast/libraycast_raylib.a example/base/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/ray-base example/base/main/main.c $(MAIN_LIBS) $(RAYLIB_LIBS)

.PHONY: ray-base run-base
ray-base: build/ray-base
run-base: ray-base
	build/ray-base

### game example
build/ray: assets build/raycast/libraycast_raylib.a example/game/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/ray example/game/main/main.c $(MAIN_LIBS) $(RAYLIB_LIBS)

.PHONY: ray run
ray: build/ray
run: ray
	build/ray

## WASM raylib example

$(RAYLIB_WEB_PATH):
	@mkdir -p $(@D)
	@if [ ! -d "$@" ]; then \
		git clone --branch $(RAYLIB_WEB_VERSION) --depth 1 https://github.com/raysan5/raylib.git "$@"; \
	fi

$(RAYLIB_WEB_STAMP): | $(RAYLIB_WEB_PATH)
	$(MAKE) -C $(RAYLIB_WEB_PATH)/src PLATFORM=PLATFORM_WEB -B
	@touch $@

$(RAYLIB_WEB_OUTPUT): $(RAYLIB_WEB_DEPS) $(RAYLIB_WEB_STAMP) $(RAYLIB_WEB_RAYCAST_LIB) example/game/main/main.c
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -o $(RAYLIB_WEB_OUTPUT) \
		example/game/main/main.c \
		$(RAYLIB_WEB_RAYCAST_LIB) $(RAYLIB_WEB_LIB) $(RAYLIB_WEB_LDFLAGS) -lm

.PHONY: wasm run-wasm
wasm: $(RAYLIB_WEB_OUTPUT)
run-wasm: wasm
	npx serve docs


## ESP32 examples

### base example
.PHONY: esp32-base-build esp32-base-flash esp32-base-monitor esp32-base-flash-monitor esp32-base-clean
esp32-base-build:
	cd example/base && . $(ESP32_HOME)/export.sh && idf.py build && idf.py size

esp32-base-flash: esp32-base-build
	cd example/base && . $(ESP32_HOME)/export.sh && idf.py flash

esp32-base-monitor:
	cd example/base && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-base-flash-monitor: esp32-base-flash
	cd example/base && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-base-clean:
	cd example/base && . $(ESP32_HOME)/export.sh && idf.py fullclean

### game example
.PHONY: esp32-build esp32-flash esp32-monitor esp32-flash-monitor esp32-clean
esp32-build:
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py build && idf.py size

esp32-flash: esp32-build
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py flash

esp32-monitor:
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-flash-monitor: esp32-flash
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-clean:
	cd example/game && . $(ESP32_HOME)/export.sh && idf.py fullclean
