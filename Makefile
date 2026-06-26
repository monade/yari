YARI_CFLAGS = -Wall -Wextra -O2
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib) -lm
SDL2_CFLAGS = $(shell pkg-config --cflags sdl2)
SDL2_LIBS = $(shell pkg-config --libs sdl2) -lm
RAYLIB_WEB_VERSION ?= 5.5
RAYLIB_WEB_PATH ?= external/raylib
RAYLIB_WEB_LIB = $(RAYLIB_WEB_PATH)/src/libraylib.a
RAYLIB_WEB_YARI_LIB = build/yari/libyari_raylib_web.a
RAYLIB_WEB_STAMP = build/raylib-web.stamp
RAYLIB_WEB_OUTPUT = docs/index.html
RAYLIB_WEB_SHELL = src/yari/platform/raylib/shell.html
RAYLIB_WEB_CFLAGS = -Wall -Wextra -O2 -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I./src/yari -I$(RAYLIB_WEB_PATH)/src
RAYLIB_WEB_LDFLAGS = -s USE_GLFW=3 -s ASSERTIONS=1 -s INITIAL_MEMORY=33554432 --shell-file $(RAYLIB_WEB_SHELL)
RAYLIB_WEB_YARI_OBJS = \
	build/yari_web.o \
	build/colors_web.o \
	build/physics_web.o \
	build/renderer_web.o \
	build/renderer_common_web.o \
	build/inputs_web.o
RAYLIB_WEB_DEPS = \
	example/fps/main/assets.h \
	example/fps/main/level.h \
	src/yari/colors.h \
	src/yari/inputs.h \
	src/yari/physics.h \
	src/yari/yari.h \
	src/yari/raymath.h \
	src/yari/renderer.h \
	$(RAYLIB_WEB_SHELL)

MAIN_CFLAGS = -Wall -Wextra -O2 -I./src/yari
MAIN_LIBS = -Lbuild/yari -lyari_raylib
SDL_MAIN_LIBS = -Lbuild/yari -lyari_sdl

EMCC = emcc
EMAR = emar

ESP32_HOME = ~/esp/v5.5.*/esp-idf

$(shell mkdir -p build)

.PHONY: all

all: ray wasm esp32-build

# Lib yari

## Raylib
build/yari.o: src/yari/yari.c
	$(CC) $(YARI_CFLAGS) -c src/yari/yari.c -o build/yari.o
build/colors.o: src/yari/colors.c
	$(CC) $(YARI_CFLAGS) -c src/yari/colors.c -o build/colors.o
build/renderer_common.o: src/yari/renderer_common.c
	$(CC) $(YARI_CFLAGS) -c src/yari/renderer_common.c -o build/renderer_common.o
build/inputs.o: src/yari/platform/raylib/inputs.c
	$(CC) $(YARI_CFLAGS) $(RAYLIB_CFLAGS) -c src/yari/platform/raylib/inputs.c -o build/inputs.o
build/renderer.o: src/yari/platform/raylib/renderer.c
	$(CC) $(YARI_CFLAGS) $(RAYLIB_CFLAGS) -c src/yari/platform/raylib/renderer.c -o build/renderer.o
build/physics.o: src/yari/physics.c
	$(CC) $(YARI_CFLAGS) -c src/yari/physics.c -o build/physics.o
build/yari/libyari_raylib.a: build/yari.o build/colors.o build/renderer_common.o build/inputs.o build/renderer.o build/physics.o
	@mkdir -p build/yari
	ar rcs build/yari/libyari_raylib.a build/yari.o build/colors.o build/renderer_common.o build/inputs.o build/renderer.o build/physics.o

## SDL2
build/inputs_sdl.o: src/yari/platform/sdl/inputs.c src/yari/inputs.h
	$(CC) $(YARI_CFLAGS) $(SDL2_CFLAGS) -c src/yari/platform/sdl/inputs.c -o build/inputs_sdl.o
build/renderer_sdl.o: src/yari/platform/sdl/renderer.c src/yari/renderer.h
	$(CC) $(YARI_CFLAGS) $(SDL2_CFLAGS) -c src/yari/platform/sdl/renderer.c -o build/renderer_sdl.o
build/yari/libyari_sdl.a: build/yari.o build/colors.o build/renderer_common.o build/inputs_sdl.o build/renderer_sdl.o build/physics.o
	@mkdir -p build/yari
	ar rcs build/yari/libyari_sdl.a build/yari.o build/colors.o build/renderer_common.o build/inputs_sdl.o build/renderer_sdl.o build/physics.o

## Raylib web
build/yari_web.o: src/yari/yari.c src/yari/yari.h src/yari/renderer.h src/yari/inputs.h src/yari/colors.h src/yari/physics.h
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/yari/yari.c -o build/yari_web.o
build/colors_web.o: src/yari/colors.c src/yari/colors.h
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/yari/colors.c -o build/colors_web.o
build/renderer_common_web.o: src/yari/renderer_common.c src/yari/renderer.h
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/yari/renderer_common.c -o build/renderer_common_web.o
build/physics_web.o: src/yari/physics.c src/yari/physics.h src/yari/yari.h
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/yari/physics.c -o build/physics_web.o
build/renderer_web.o: src/yari/platform/raylib/renderer.c src/yari/renderer.h | $(RAYLIB_WEB_PATH)
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/yari/platform/raylib/renderer.c -o build/renderer_web.o
build/inputs_web.o: src/yari/platform/raylib/inputs.c src/yari/inputs.h | $(RAYLIB_WEB_PATH)
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -c src/yari/platform/raylib/inputs.c -o build/inputs_web.o
$(RAYLIB_WEB_YARI_LIB): $(RAYLIB_WEB_YARI_OBJS)
	@mkdir -p build/yari
	$(EMAR) rcs $(RAYLIB_WEB_YARI_LIB) $(RAYLIB_WEB_YARI_OBJS)

# Tools
.PHONY: assets map-builder run-map-builder

build/assets_packer: src/tools/assets_packer.c
	$(CC) -o build/assets_packer src/tools/assets_packer.c -lm

build/font_baker: src/tools/font_baker.c
	$(CC) -o build/font_baker src/tools/font_baker.c -lm

assets: build/assets_packer build/font_baker
	build/assets_packer example/fps/assets example/fps/main/assets.h
	build/font_baker example/fps/assets/font example/fps/main/fonts.h
	build/assets_packer example/cart/assets example/cart/main/assets.h
	build/font_baker example/cart/assets/font example/cart/main/fonts.h

build/map_builder: src/tools/map_builder.c src/tools/raygui.h
	$(CC) -Wall -Wextra -O2 $(RAYLIB_CFLAGS) -I./src/tools -o build/map_builder src/tools/map_builder.c $(RAYLIB_LIBS)
map-builder: build/map_builder
edit-fps: map-builder
	build/map_builder example/fps/assets example/fps/main/level.h
edit-cart: map-builder
	build/map_builder example/cart/assets example/cart/main/level.h

# Examples

## Raylib examples

## Raylib examples

### base example
build/ray-base: build/yari/libyari_raylib.a example/base/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/ray-base example/base/main/main.c $(MAIN_LIBS) $(RAYLIB_LIBS)

.PHONY: ray-base run-base
ray-base: build/ray-base
run-base: ray-base
	build/ray-base

### fps example
build/ray: assets build/yari/libyari_raylib.a example/fps/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/ray example/fps/main/main.c $(MAIN_LIBS) $(RAYLIB_LIBS)

.PHONY: ray run
ray: build/ray
run: ray
	build/ray

### SDL2 fps example
build/sdl: assets build/yari/libyari_sdl.a example/fps/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/sdl example/fps/main/main.c $(SDL_MAIN_LIBS) $(SDL2_LIBS)

.PHONY: sdl run-sdl
sdl: build/sdl
run-sdl: sdl
	build/sdl

## WASM raylib example

$(RAYLIB_WEB_PATH):
	@mkdir -p $(@D)
	@if [ ! -d "$@" ]; then \
		git clone --branch $(RAYLIB_WEB_VERSION) --depth 1 https://github.com/raysan5/raylib.git "$@"; \
	fi

$(RAYLIB_WEB_STAMP): | $(RAYLIB_WEB_PATH)
	$(MAKE) -C $(RAYLIB_WEB_PATH)/src PLATFORM=PLATFORM_WEB -B
	@touch $@

$(RAYLIB_WEB_OUTPUT): $(RAYLIB_WEB_DEPS) $(RAYLIB_WEB_STAMP) $(RAYLIB_WEB_YARI_LIB) example/fps/main/main.c
	$(EMCC) $(RAYLIB_WEB_CFLAGS) -o $(RAYLIB_WEB_OUTPUT) \
		example/fps/main/main.c \
		$(RAYLIB_WEB_YARI_LIB) $(RAYLIB_WEB_LIB) $(RAYLIB_WEB_LDFLAGS) -lm

.PHONY: wasm run-wasm
wasm: $(RAYLIB_WEB_OUTPUT)
run-wasm: wasm
	npx serve docs


### cart example
build/ray-cart: assets build/yari/libyari_raylib.a example/cart/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/ray-cart example/cart/main/main.c $(MAIN_LIBS) $(RAYLIB_LIBS)

.PHONY: ray-cart run-cart
ray-cart: build/ray-cart
run-cart: ray-cart
	build/ray-cart

### SDL2 cart example
build/sdl-cart: assets build/yari/libyari_sdl.a example/cart/main/main.c
	$(CC) $(MAIN_CFLAGS) -o build/sdl-cart example/cart/main/main.c $(SDL_MAIN_LIBS) $(SDL2_LIBS)

.PHONY: sdl-cart run-sdl-cart
sdl-cart: build/sdl-cart
run-sdl-cart: sdl-cart
	build/sdl-cart

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
esp32-build: assets
	cd example/fps && . $(ESP32_HOME)/export.sh && idf.py build && idf.py size

esp32-flash: esp32-build
	cd example/fps && . $(ESP32_HOME)/export.sh && idf.py flash

esp32-monitor:
	cd example/fps && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-flash-monitor: esp32-flash
	cd example/fps && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-clean:
	cd example/fps && . $(ESP32_HOME)/export.sh && idf.py fullclean

### cart example
.PHONY: esp32-cart-build esp32-cart-flash esp32-cart-monitor esp32-cart-flash-monitor esp32-cart-clean
esp32-cart-build: assets
	cd example/cart && . $(ESP32_HOME)/export.sh && idf.py build && idf.py size

esp32-cart-flash: esp32-cart-build
	cd example/cart && . $(ESP32_HOME)/export.sh && idf.py flash

esp32-cart-monitor:
	cd example/cart && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-cart-flash-monitor: esp32-cart-flash
	cd example/cart && . $(ESP32_HOME)/export.sh && idf.py monitor

esp32-cart-clean:
	cd example/cart && . $(ESP32_HOME)/export.sh && idf.py fullclean