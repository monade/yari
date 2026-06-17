#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#define DS_IMPLEMENTATION
#define DS_NO_PREFIX
#include "ds.h"

#define DEFAULT_MAP_COLS 100
#define DEFAULT_MAP_ROWS 100
#define DEFAULT_ENTITY_COLLISION_THRESHOLD 0.4f
#define MIN_MAP_SIZE 1
#define MAX_MAP_SIZE 255
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800
#define SIDEBAR_WIDTH 420.0f
#define ASSET_ROW_HEIGHT 42.0f
#define ENTITY_NAME_SIZE 64
#define MASK_NAME_SIZE 32
#define MAX_CUSTOM_MASKS 30
#define NO_SELECTION -1
#define MAP_BUILDER_STATE_BEGIN "MAP_BUILDER_STATE_BEGIN"
#define MAP_BUILDER_STATE_END "MAP_BUILDER_STATE_END"
#define MAP_BUILDER_STATE_VERSION 1

typedef struct {
    char *name;
    char *path;
    char *texture_symbol;
    Texture2D texture;
    Color fallback;
} Asset;

typedef struct {
    char name[ENTITY_NAME_SIZE];
    Vector2 pos;
    int asset_index;
    float vdiv;
    float hdiv;
    float vmove;
    bool disabled;
    bool exported;
    char update_fn[ENTITY_NAME_SIZE];
    uint32_t collision_mask;
    float collision_threshold;
} PlacedEntity;

typedef struct {
    char name[MASK_NAME_SIZE];
    char symbol[MASK_NAME_SIZE + 5];
    uint32_t value;
    unsigned int shift;
} CollisionLayer;

typedef struct {
    char name[ENTITY_NAME_SIZE];
} UpdateFn;

typedef struct {
    int cols;
    int rows;
    int *data;
} WallMap;

typedef enum {
    BRUSH_WALL = 0,
    BRUSH_ENTITY = 1,
    BRUSH_PLAYER = 2,
    BRUSH_SURFACE = 3,
} BrushKind;

typedef enum {
    WALL_POINT = 0,
    WALL_RECT = 1,
    WALL_CIRCLE = 2,
} WallDrawMode;

typedef enum {
    SELECTION_NONE = 0,
    SELECTION_ENTITY = 1,
    SELECTION_WALL = 2,
} SelectionKind;

typedef struct {
    int x;
    int y;
} SelectedWall;

typedef struct {
    PlacedEntity entity;
    Vector2 offset;
} ClipboardEntity;

typedef struct {
    int offset_x;
    int offset_y;
    int asset_index;
} ClipboardWall;

da_declare(Assets, Asset);
da_declare(PlacedEntities, PlacedEntity);
da_declare(CollisionLayers, CollisionLayer);
da_declare(UpdateFns, UpdateFn);
da_declare(SelectedEntities, int);
da_declare(SelectedWalls, SelectedWall);
da_declare(ClipboardEntities, ClipboardEntity);
da_declare(ClipboardWalls, ClipboardWall);

typedef struct {
    Assets assets;
    PlacedEntities entities;
    WallMap map;
    Camera2D camera;

    int selected_asset;
    int floor_asset;
    int ceil_asset;
    int editing_entity;
    bool editing_wall;
    int editing_wall_x;
    int editing_wall_y;
    SelectionKind selection_kind;
    SelectedEntities selected_entities;
    SelectedWalls selected_walls;
    SelectionKind clipboard_kind;
    ClipboardEntities clipboard_entities;
    ClipboardWalls clipboard_walls;
    int clipboard_wall_width;
    int clipboard_wall_height;
    bool selection_dragging;
    bool selection_drag_moved;
    Vector2 selection_drag_start;
    Vector2 selection_drag_end;
    int selection_drag_start_x;
    int selection_drag_start_y;
    int selection_drag_end_x;
    int selection_drag_end_y;
    bool selection_move_dragging;
    bool selection_move_moved;
    Vector2 selection_move_start;
    Vector2 selection_move_last;
    int selection_move_start_x;
    int selection_move_start_y;
    int selection_move_last_x;
    int selection_move_last_y;
    BrushKind brush;
    WallDrawMode wall_mode;
    bool wall_dragging;
    bool wall_drag_moved;
    int wall_drag_start_x;
    int wall_drag_start_y;
    int wall_drag_end_x;
    int wall_drag_end_y;
    int pending_cols;
    int pending_rows;

    bool cols_edit;
    bool rows_edit;
    bool vdiv_edit;
    bool hdiv_edit;
    bool vmove_edit;
    bool threshold_edit;
    bool entity_name_edit;
    bool new_mask_edit;
    bool update_fn_edit;
    bool player_pos_x_edit;
    bool player_pos_y_edit;
    bool player_dir_x_edit;
    bool player_dir_y_edit;
    bool player_threshold_edit;

    float brush_vdiv;
    float brush_hdiv;
    float brush_vmove;
    float brush_collision_threshold;
    uint32_t brush_collision_mask;
    char brush_vdiv_text[32];
    char brush_hdiv_text[32];
    char brush_vmove_text[32];
    char brush_threshold_text[32];
    char brush_entity_name[ENTITY_NAME_SIZE];
    char new_collision_layer[MASK_NAME_SIZE];
    char new_update_fn[ENTITY_NAME_SIZE];
    bool brush_disabled;
    bool brush_exported;
    char brush_update_fn[ENTITY_NAME_SIZE];

    Vector2 player_pos;
    Vector2 player_dir;
    float player_collision_threshold;
    uint32_t player_collision_mask;
    char player_pos_x_text[32];
    char player_pos_y_text[32];
    char player_dir_x_text[32];
    char player_dir_y_text[32];
    char player_threshold_text[32];

    float asset_scroll;
    float mask_scroll;
    float player_mask_scroll;
    float update_fn_scroll;
    CollisionLayers collision_layers;
    UpdateFns update_fns;
    const char *asset_dir;
    const char *output_path;
    char status[256];
    double status_until;
    bool pinching;
    float pinch_distance;
    bool suppress_left_drag;
} App;

typedef struct {
    bool has_size;
    WallMap map;
    PlacedEntities entities;
    CollisionLayers collision_layers;
    UpdateFns update_fns;
    int floor_asset;
    int ceil_asset;
    Vector2 player_pos;
    Vector2 player_dir;
    float player_collision_threshold;
    uint32_t player_collision_mask;
} LoadedLevel;

static void set_status(App *app, const char *fmt, ...);

static char *copy_string(const char *text) {
    size_t len = strlen(text);
    char *copy = malloc(len + 1);
    if (!copy) {
        log_error("Out of memory\n");
        exit(1);
    }
    memcpy(copy, text, len + 1);
    return copy;
}

static char *join_path(const char *dir, const char *file) {
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file);
    bool needs_sep = dir_len > 0 && dir[dir_len - 1] != '/';
    char *path = malloc(dir_len + file_len + (needs_sep ? 2 : 1));
    if (!path) {
        log_error("Out of memory\n");
        exit(1);
    }
    snprintf(path, dir_len + file_len + (needs_sep ? 2 : 1), "%s%s%s", dir, needs_sep ? "/" : "", file);
    return path;
}

static bool has_image_extension(const char *file_name) {
    const char *dot = strrchr(file_name, '.');
    if (!dot) return false;

    char ext[8] = {0};
    size_t len = strlen(dot);
    if (len >= sizeof(ext)) return false;
    for (size_t i = 0; i < len; i++) ext[i] = (char)tolower((unsigned char)dot[i]);

    return strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0;
}

static char *basename_without_extension(const char *file_name) {
    char *name = copy_string(file_name);
    char *dot = strrchr(name, '.');
    if (dot) *dot = '\0';
    return name;
}

static char *texture_symbol_from_name(const char *name) {
    size_t len = strlen(name);
    char *symbol = malloc(len + 4);
    if (!symbol) {
        log_error("Out of memory\n");
        exit(1);
    }

    symbol[0] = 't';
    symbol[1] = 'x';
    symbol[2] = '_';
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        symbol[i + 3] = (char)(isalnum(c) ? c : '_');
    }
    symbol[len + 3] = '\0';
    return symbol;
}

static Color color_from_name(const char *name) {
    uint32_t hash = 2166136261u;
    for (const char *c = name; *c; c++) {
        hash ^= (uint8_t)*c;
        hash *= 16777619u;
    }

    return (Color){
        .r = (unsigned char)(80 + (hash & 0x7f)),
        .g = (unsigned char)(80 + ((hash >> 8) & 0x7f)),
        .b = (unsigned char)(80 + ((hash >> 16) & 0x7f)),
        .a = 255,
    };
}

static int compare_assets(const void *a, const void *b) {
    const Asset *left = (const Asset *)a;
    const Asset *right = (const Asset *)b;
    return strcmp(left->name, right->name);
}

static bool scan_assets(Assets *assets, const char *asset_dir) {
    DIR *dir = opendir(asset_dir);
    if (!dir) {
        log_error("Error reading asset directory %s\n", asset_dir);
        return false;
    }

    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (!has_image_extension(entry->d_name)) continue;

        char *name = basename_without_extension(entry->d_name);
        Asset asset = {
            .name = name,
            .path = join_path(asset_dir, entry->d_name),
            .texture_symbol = texture_symbol_from_name(name),
            .texture = {0},
            .fallback = color_from_name(name),
        };
        da_append(assets, asset);
    }

    closedir(dir);
    qsort(assets->data, assets->length, sizeof(assets->data[0]), compare_assets);
    return true;
}

static void load_asset_textures(Assets *assets) {
    da_foreach(assets, asset) {
        asset->texture = LoadTexture(asset->path);
        if (asset->texture.id != 0) SetTextureFilter(asset->texture, TEXTURE_FILTER_POINT);
    }
}

static void filter_assets_by_size(Assets *assets, int w, int h) {
    size_t write = 0;
    for (size_t i = 0; i < assets->length; i++) {
        Asset *asset = &assets->data[i];
        if (asset->texture.width == w && asset->texture.height == h) {
            if (write != i) assets->data[write] = assets->data[i];
            write++;
        } else {
            if (asset->texture.id != 0) UnloadTexture(asset->texture);
            free(asset->name);
            free(asset->path);
            free(asset->texture_symbol);
        }
    }
    assets->length = write;
}

static void free_assets(Assets *assets) {
    da_foreach(assets, asset) {
        if (asset->texture.id != 0) UnloadTexture(asset->texture);
        free(asset->name);
        free(asset->path);
        free(asset->texture_symbol);
    }
    da_free(assets);
}

static void wall_map_init(WallMap *map, int cols, int rows) {
    map->cols = cols;
    map->rows = rows;
    map->data = malloc((size_t)cols * (size_t)rows * sizeof(map->data[0]));
    if (!map->data) {
        log_error("Out of memory\n");
        exit(1);
    }
    for (int i = 0; i < cols * rows; i++) map->data[i] = -1;
}

static void wall_map_free(WallMap *map) {
    free(map->data);
    map->data = NULL;
    map->cols = 0;
    map->rows = 0;
}

static bool wall_map_inside(const WallMap *map, int x, int y) {
    return x >= 0 && y >= 0 && x < map->cols && y < map->rows;
}

static int *wall_map_cell(WallMap *map, int x, int y) {
    if (!wall_map_inside(map, x, y)) return NULL;
    return &map->data[y * map->cols + x];
}

static int clamp_int(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static void world_to_cell_clamped(const App *app, Vector2 world, int *x, int *y) {
    *x = clamp_int((int)floorf(world.x), 0, app->map.cols - 1);
    *y = clamp_int((int)floorf(world.y), 0, app->map.rows - 1);
}

static bool selected_wall_asset_is_valid(const App *app) {
    return app->selected_asset >= 0 && app->selected_asset < (int)app->assets.length;
}

static bool asset_index_is_valid(const App *app, int asset_index) {
    return asset_index >= 0 && asset_index < (int)app->assets.length;
}

static const char *asset_symbol_or_null(const App *app, int asset_index) {
    if (asset_index_is_valid(app, asset_index)) return app->assets.data[asset_index].texture_symbol;
    return "NULL_ASSET";
}

static int asset_index_from_symbol(const App *app, const char *symbol) {
    if (!symbol || symbol[0] == '\0' || strcmp(symbol, "0") == 0 || strcmp(symbol, "NULL_ASSET") == 0) return -1;
    for (size_t i = 0; i < app->assets.length; i++) {
        if (strcmp(app->assets.data[i].texture_symbol, symbol) == 0) return (int)i;
    }
    return -1;
}

static void stroke_wall_rect(App *app, int x0, int y0, int x1, int y1) {
    int min_x = x0 < x1 ? x0 : x1;
    int max_x = x0 > x1 ? x0 : x1;
    int min_y = y0 < y1 ? y0 : y1;
    int max_y = y0 > y1 ? y0 : y1;

    min_x = clamp_int(min_x, 0, app->map.cols - 1);
    max_x = clamp_int(max_x, 0, app->map.cols - 1);
    min_y = clamp_int(min_y, 0, app->map.rows - 1);
    max_y = clamp_int(max_y, 0, app->map.rows - 1);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            if (x != min_x && x != max_x && y != min_y && y != max_y) continue;
            int *cell = wall_map_cell(&app->map, x, y);
            if (cell) *cell = app->selected_asset;
        }
    }
}

static void stroke_wall_circle(App *app, int cx, int cy, int edge_x, int edge_y) {
    float center_x = (float)cx + 0.5f;
    float center_y = (float)cy + 0.5f;
    float dx = ((float)edge_x + 0.5f) - center_x;
    float dy = ((float)edge_y + 0.5f) - center_y;
    float radius = sqrtf(dx * dx + dy * dy);
    if (radius < 0.5f) {
        int *cell = wall_map_cell(&app->map, cx, cy);
        if (cell) *cell = app->selected_asset;
        return;
    }

    int min_x = clamp_int((int)floorf(center_x - radius), 0, app->map.cols - 1);
    int max_x = clamp_int((int)floorf(center_x + radius), 0, app->map.cols - 1);
    int min_y = clamp_int((int)floorf(center_y - radius), 0, app->map.rows - 1);
    int max_y = clamp_int((int)floorf(center_y + radius), 0, app->map.rows - 1);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float cell_x = (float)x + 0.5f;
            float cell_y = (float)y + 0.5f;
            float ddx = cell_x - center_x;
            float ddy = cell_y - center_y;
            float dist = sqrtf(ddx * ddx + ddy * ddy);
            if (fabsf(dist - radius) <= 0.5f) {
                int *cell = wall_map_cell(&app->map, x, y);
                if (cell) *cell = app->selected_asset;
            }
        }
    }
}

static void apply_wall_drag(App *app) {
    if (!selected_wall_asset_is_valid(app)) {
        set_status(app, "Select a texture first");
        return;
    }

    if (app->wall_mode == WALL_CIRCLE) {
        stroke_wall_circle(app, app->wall_drag_start_x, app->wall_drag_start_y, app->wall_drag_end_x, app->wall_drag_end_y);
    } else {
        stroke_wall_rect(app, app->wall_drag_start_x, app->wall_drag_start_y, app->wall_drag_end_x, app->wall_drag_end_y);
    }
}

static void wall_map_resize(App *app, int cols, int rows) {
    WallMap next = {0};
    wall_map_init(&next, cols, rows);

    int copy_cols = app->map.cols < cols ? app->map.cols : cols;
    int copy_rows = app->map.rows < rows ? app->map.rows : rows;
    for (int y = 0; y < copy_rows; y++) {
        memcpy(&next.data[y * next.cols], &app->map.data[y * app->map.cols], (size_t)copy_cols * sizeof(next.data[0]));
    }

    for (size_t i = 0; i < app->entities.length;) {
        PlacedEntity *entity = &app->entities.data[i];
        if (entity->pos.x < 0.0f || entity->pos.y < 0.0f || entity->pos.x >= (float)cols || entity->pos.y >= (float)rows) {
            da_remove_unordered(&app->entities, i);
        } else {
            i++;
        }
    }

    wall_map_free(&app->map);
    app->map = next;
    app->pending_cols = cols;
    app->pending_rows = rows;

    if (app->player_pos.x < 0.0f) app->player_pos.x = 0.0f;
    if (app->player_pos.y < 0.0f) app->player_pos.y = 0.0f;
    if (app->player_pos.x >= (float)cols) app->player_pos.x = (float)cols - 0.001f;
    if (app->player_pos.y >= (float)rows) app->player_pos.y = (float)rows - 0.001f;
}

static float clamp_float(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static Rectangle get_sidebar_bounds(void) {
    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();
    return (Rectangle){(float)screen_w - SIDEBAR_WIDTH, 0.0f, SIDEBAR_WIDTH, (float)screen_h};
}

static Rectangle get_map_bounds(void) {
    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();
    float map_width = (float)screen_w - SIDEBAR_WIDTH;
    if (map_width < 160.0f) map_width = 160.0f;
    return (Rectangle){0.0f, 0.0f, map_width, (float)screen_h};
}

static void update_camera_offset(App *app, Rectangle map_bounds) {
    app->camera.offset = (Vector2){map_bounds.x + map_bounds.width * 0.5f, map_bounds.y + map_bounds.height * 0.5f};
}

static void fit_camera(App *app, Rectangle map_bounds) {
    update_camera_offset(app, map_bounds);
    float zoom_x = map_bounds.width / (float)app->map.cols;
    float zoom_y = map_bounds.height / (float)app->map.rows;
    app->camera.zoom = clamp_float(fminf(zoom_x, zoom_y) * 0.92f, 0.05f, 96.0f);
    app->camera.target = (Vector2){(float)app->map.cols * 0.5f, (float)app->map.rows * 0.5f};
    app->camera.rotation = 0.0f;
}

static bool app_is_editing(const App *app) {
    return app->cols_edit ||
        app->rows_edit ||
        app->vdiv_edit ||
        app->hdiv_edit ||
        app->vmove_edit ||
        app->threshold_edit ||
        app->entity_name_edit ||
        app->new_mask_edit ||
        app->update_fn_edit ||
        app->player_pos_x_edit ||
        app->player_pos_y_edit ||
        app->player_dir_x_edit ||
        app->player_dir_y_edit ||
        app->player_threshold_edit;
}

static void set_status(App *app, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(app->status, sizeof(app->status), fmt, args);
    va_end(args);
    app->status_until = GetTime() + 4.0;
}

static void draw_texture_preview(const Asset *asset, Rectangle dst, Color tint) {
    if (asset && asset->texture.id != 0) {
        Rectangle src = {0.0f, 0.0f, (float)asset->texture.width, (float)asset->texture.height};
        DrawTexturePro(asset->texture, src, dst, (Vector2){0.0f, 0.0f}, 0.0f, tint);
    } else if (asset) {
        DrawRectangleRec(dst, asset->fallback);
    } else {
        DrawRectangleRec(dst, (Color){68, 72, 78, 255});
    }
}

static void sanitize_identifier(char *dst, size_t dst_size, const char *src, const char *fallback, bool upper) {
    if (dst_size == 0) return;
    size_t out = 0;

    for (const char *c = src; *c && out + 1 < dst_size; c++) {
        unsigned char ch = (unsigned char)*c;
        if (isalnum(ch) || ch == '_') {
            char next = (char)ch;
            if (isalpha(ch)) next = upper ? (char)toupper(ch) : (char)tolower(ch);
            dst[out++] = next;
        } else if (out > 0 && dst[out - 1] != '_' && out + 1 < dst_size) {
            dst[out++] = '_';
        }
    }

    while (out > 0 && dst[out - 1] == '_') out--;
    dst[out] = '\0';

    if (out == 0) {
        snprintf(dst, dst_size, "%s", fallback);
        out = strlen(dst);
    }

    if (isdigit((unsigned char)dst[0])) {
        char tmp[ENTITY_NAME_SIZE + MASK_NAME_SIZE] = {0};
        snprintf(tmp, sizeof(tmp), "%s_%s", fallback, dst);
        snprintf(dst, dst_size, "%s", tmp);
    }
}

static bool text_is_empty(const char *text) {
    while (*text) {
        if (!isspace((unsigned char)*text)) return false;
        text++;
    }
    return true;
}

static void sanitize_update_fn(char *dst, size_t dst_size, const char *src) {
    if (dst_size == 0) return;

    size_t out = 0;
    for (const char *c = src; *c && out + 1 < dst_size; c++) {
        unsigned char ch = (unsigned char)*c;
        if (isalnum(ch) || ch == '_') {
            dst[out++] = (char)ch;
        } else if (out > 0 && dst[out - 1] != '_' && out + 1 < dst_size) {
            dst[out++] = '_';
        }
    }

    while (out > 0 && dst[out - 1] == '_') out--;
    dst[out] = '\0';

    if (out == 0) {
        snprintf(dst, dst_size, "update_entity");
        out = strlen(dst);
    }

    if (isdigit((unsigned char)dst[0])) {
        char tmp[ENTITY_NAME_SIZE * 2] = {0};
        snprintf(tmp, sizeof(tmp), "update_%s", dst);
        snprintf(dst, dst_size, "%s", tmp);
    }
}

static bool update_fn_exists(const UpdateFns *update_fns, const char *name) {
    for (size_t i = 0; i < update_fns->length; i++) {
        if (strcmp(update_fns->data[i].name, name) == 0) return true;
    }
    return false;
}

static bool remember_update_fn(UpdateFns *update_fns, const char *raw_name, char *stored_name, size_t stored_name_size) {
    if (text_is_empty(raw_name)) return false;

    char name[ENTITY_NAME_SIZE] = {0};
    sanitize_update_fn(name, sizeof(name), raw_name);
    if (stored_name && stored_name_size > 0) snprintf(stored_name, stored_name_size, "%s", name);

    if (update_fn_exists(update_fns, name)) return true;

    UpdateFn update_fn = {0};
    snprintf(update_fn.name, sizeof(update_fn.name), "%s", name);
    da_append(update_fns, update_fn);
    return true;
}

static void add_update_fn(App *app, const char *raw_name) {
    char name[ENTITY_NAME_SIZE] = {0};
    if (!remember_update_fn(&app->update_fns, raw_name, name, sizeof(name))) return;

    snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "%s", name);
    snprintf(app->new_update_fn, sizeof(app->new_update_fn), "");
    app->update_fn_edit = false;
}

static void remove_update_fn_at(App *app, size_t index) {
    if (index >= app->update_fns.length) return;

    char name[ENTITY_NAME_SIZE] = {0};
    snprintf(name, sizeof(name), "%s", app->update_fns.data[index].name);

    if (strcmp(app->brush_update_fn, name) == 0) {
        snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "");
    }

    for (size_t i = 0; i < app->entities.length; i++) {
        PlacedEntity *entity = &app->entities.data[i];
        if (strcmp(entity->update_fn, name) == 0) {
            snprintf(entity->update_fn, sizeof(entity->update_fn), "");
        }
    }

    if (index + 1 < app->update_fns.length) {
        memmove(
            &app->update_fns.data[index],
            &app->update_fns.data[index + 1],
            (app->update_fns.length - index - 1) * sizeof(app->update_fns.data[0]));
    }
    app->update_fns.length--;
    set_status(app, "Removed update fn %s", name);
}

static void sync_update_fns_from_entities(App *app) {
    for (size_t i = 0; i < app->entities.length; i++) {
        const PlacedEntity *entity = &app->entities.data[i];
        if (entity->update_fn[0] == '\0') continue;
        remember_update_fn(&app->update_fns, entity->update_fn, NULL, 0);
    }
}

static bool entity_name_exists(const App *app, const char *name) {
    for (size_t i = 0; i < app->entities.length; i++) {
        if (strcmp(app->entities.data[i].name, name) == 0) return true;
    }
    return false;
}

static bool entity_name_exists_except(const App *app, const char *name, int skip_index) {
    for (size_t i = 0; i < app->entities.length; i++) {
        if ((int)i == skip_index) continue;
        if (strcmp(app->entities.data[i].name, name) == 0) return true;
    }
    return false;
}

static void strip_numeric_suffix(char *name) {
    size_t len = strlen(name);
    if (len < 3) return;

    size_t i = len;
    while (i > 0 && isdigit((unsigned char)name[i - 1])) i--;
    if (i > 0 && i < len && name[i - 1] == '_') name[i - 1] = '\0';
}

static void make_unique_entity_name(const App *app, const char *raw_name, char *dst, size_t dst_size) {
    char base[ENTITY_NAME_SIZE] = {0};
    sanitize_identifier(base, sizeof(base), raw_name, "entity", false);

    char root[ENTITY_NAME_SIZE] = {0};
    snprintf(root, sizeof(root), "%s", base);
    strip_numeric_suffix(root);
    if (root[0] == '\0') snprintf(root, sizeof(root), "entity");

    snprintf(dst, dst_size, "%s", base);
    if (!entity_name_exists(app, dst)) return;

    for (unsigned int suffix = 2; suffix < 100000; suffix++) {
        snprintf(dst, dst_size, "%s_%u", root, suffix);
        if (!entity_name_exists(app, dst)) return;
    }
}

static void make_unique_entity_name_except(const App *app, const char *raw_name, int skip_index, char *dst, size_t dst_size) {
    char base[ENTITY_NAME_SIZE] = {0};
    sanitize_identifier(base, sizeof(base), raw_name, "entity", false);

    char root[ENTITY_NAME_SIZE] = {0};
    snprintf(root, sizeof(root), "%s", base);
    strip_numeric_suffix(root);
    if (root[0] == '\0') snprintf(root, sizeof(root), "entity");

    snprintf(dst, dst_size, "%s", base);
    if (!entity_name_exists_except(app, dst, skip_index)) return;

    for (unsigned int suffix = 2; suffix < 100000; suffix++) {
        snprintf(dst, dst_size, "%s_%u", root, suffix);
        if (!entity_name_exists_except(app, dst, skip_index)) return;
    }
}

static bool collision_layer_exists(const App *app, const char *name) {
    for (size_t i = 0; i < app->collision_layers.length; i++) {
        if (strcmp(app->collision_layers.data[i].name, name) == 0) return true;
    }
    return false;
}

static bool collision_layer_reserved(const char *name) {
    return strcmp(name, "NONE") == 0 || strcmp(name, "WALL") == 0 || strcmp(name, "ALL") == 0;
}

static bool collision_layer_shift_used(const App *app, unsigned int shift) {
    for (size_t i = 0; i < app->collision_layers.length; i++) {
        if (app->collision_layers.data[i].shift == shift) return true;
    }
    return false;
}

static unsigned int next_collision_layer_shift(const App *app) {
    for (unsigned int shift = 1; shift <= MAX_CUSTOM_MASKS; shift++) {
        if (!collision_layer_shift_used(app, shift)) return shift;
    }
    return 0;
}

static void add_collision_layer(App *app, const char *raw_name) {
    char input[MASK_NAME_SIZE] = {0};
    snprintf(input, sizeof(input), "%s", raw_name);
    for (char *c = input; *c; c++) *c = (char)toupper((unsigned char)*c);

    const char *name_src = input;
    if (strncmp(name_src, "YR_CMSK_", 5) == 0) name_src += 5;

    char base[MASK_NAME_SIZE] = {0};
    sanitize_identifier(base, sizeof(base), name_src, "LAYER", true);

    char name[MASK_NAME_SIZE] = {0};
    snprintf(name, sizeof(name), "%s", base);
    if (collision_layer_reserved(name) || collision_layer_exists(app, name)) {
        for (unsigned int suffix = 2; suffix < 1000; suffix++) {
            snprintf(name, sizeof(name), "%s_%u", base, suffix);
            if (!collision_layer_reserved(name) && !collision_layer_exists(app, name)) break;
        }
    }

    unsigned int shift = next_collision_layer_shift(app);
    if (shift == 0) {
        set_status(app, "Collision layer limit reached");
        return;
    }

    CollisionLayer layer = {
        .value = 1u << shift,
        .shift = shift,
    };
    snprintf(layer.name, sizeof(layer.name), "%s", name);
    snprintf(layer.symbol, sizeof(layer.symbol), "YR_CMSK_%s", name);
    da_append(&app->collision_layers, layer);

    app->brush_collision_mask |= layer.value;
    app->player_collision_mask |= layer.value;
    snprintf(app->new_collision_layer, sizeof(app->new_collision_layer), "");
}

static void remove_collision_layer_at(App *app, size_t index) {
    if (index >= app->collision_layers.length) return;

    CollisionLayer layer = app->collision_layers.data[index];
    app->brush_collision_mask &= ~layer.value;
    app->player_collision_mask &= ~layer.value;

    for (size_t i = 0; i < app->entities.length; i++) {
        app->entities.data[i].collision_mask &= ~layer.value;
    }

    if (index + 1 < app->collision_layers.length) {
        memmove(
            &app->collision_layers.data[index],
            &app->collision_layers.data[index + 1],
            (app->collision_layers.length - index - 1) * sizeof(app->collision_layers.data[0]));
    }
    app->collision_layers.length--;
    set_status(app, "Removed collision mask %s", layer.symbol);
}

static bool editing_entity_is_valid(const App *app) {
    return app->editing_entity >= 0 && app->editing_entity < (int)app->entities.length;
}

static bool shift_is_down(void) {
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}

static bool selected_entity_is_valid(const App *app, int entity_index) {
    return entity_index >= 0 && entity_index < (int)app->entities.length;
}

static int selected_entity_slot(const App *app, int entity_index) {
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        if (app->selected_entities.data[i] == entity_index) return (int)i;
    }
    return -1;
}

static bool entity_is_selected(const App *app, int entity_index) {
    return selected_entity_slot(app, entity_index) >= 0;
}

static int selected_wall_slot(const App *app, int x, int y) {
    for (size_t i = 0; i < app->selected_walls.length; i++) {
        const SelectedWall *wall = &app->selected_walls.data[i];
        if (wall->x == x && wall->y == y) return (int)i;
    }
    return -1;
}

static bool wall_is_selected(const App *app, int x, int y) {
    return selected_wall_slot(app, x, y) >= 0;
}

static bool selection_has_items(const App *app) {
    return app->selected_entities.length > 0 || app->selected_walls.length > 0;
}

static void clear_edit_selection(App *app) {
    app->editing_entity = NO_SELECTION;
    app->editing_wall = false;
    app->editing_wall_x = NO_SELECTION;
    app->editing_wall_y = NO_SELECTION;
    app->selection_kind = SELECTION_NONE;
    app->selected_entities.length = 0;
    app->selected_walls.length = 0;
    app->selection_dragging = false;
    app->selection_drag_moved = false;
    app->selection_move_dragging = false;
    app->selection_move_moved = false;
    app->wall_dragging = false;
}

static void load_entity_fields_into_editor(App *app, int entity_index) {
    if (entity_index < 0 || entity_index >= (int)app->entities.length) return;

    PlacedEntity *entity = &app->entities.data[entity_index];
    app->brush = BRUSH_ENTITY;
    app->editing_entity = entity_index;
    app->editing_wall = false;
    app->editing_wall_x = NO_SELECTION;
    app->editing_wall_y = NO_SELECTION;
    app->selected_asset = entity->asset_index;
    app->brush_vdiv = entity->vdiv;
    app->brush_hdiv = entity->hdiv;
    app->brush_vmove = entity->vmove;
    app->brush_collision_threshold = entity->collision_threshold;
    app->brush_collision_mask = entity->collision_mask;
    app->brush_disabled = entity->disabled;
    app->brush_exported = entity->exported;
    snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "%s", entity->update_fn);
    if (app->brush_update_fn[0] != '\0') remember_update_fn(&app->update_fns, app->brush_update_fn, NULL, 0);
    snprintf(app->brush_vdiv_text, sizeof(app->brush_vdiv_text), "%.3f", app->brush_vdiv);
    snprintf(app->brush_hdiv_text, sizeof(app->brush_hdiv_text), "%.3f", app->brush_hdiv);
    snprintf(app->brush_vmove_text, sizeof(app->brush_vmove_text), "%.3f", app->brush_vmove);
    snprintf(app->brush_threshold_text, sizeof(app->brush_threshold_text), "%.3f", app->brush_collision_threshold);
    snprintf(app->brush_entity_name, sizeof(app->brush_entity_name), "%s", entity->name);
    set_status(app, "Editing entity %s", entity->name);
}

static void load_entity_into_editor(App *app, int entity_index) {
    if (entity_index < 0 || entity_index >= (int)app->entities.length) return;

    clear_edit_selection(app);
    app->selection_kind = SELECTION_ENTITY;
    da_append(&app->selected_entities, entity_index);
    load_entity_fields_into_editor(app, entity_index);
}

static void load_wall_fields_into_editor(App *app, int x, int y) {
    int *cell = wall_map_cell(&app->map, x, y);
    if (!cell || *cell < 0) return;

    app->brush = BRUSH_WALL;
    app->editing_entity = NO_SELECTION;
    app->editing_wall = true;
    app->editing_wall_x = x;
    app->editing_wall_y = y;
    app->selected_asset = *cell;
    set_status(app, "Editing wall %d,%d", x, y);
}

static void load_wall_into_editor(App *app, int x, int y) {
    int *cell = wall_map_cell(&app->map, x, y);
    if (!cell || *cell < 0) return;

    clear_edit_selection(app);
    app->selection_kind = SELECTION_WALL;
    da_append(&app->selected_walls, ((SelectedWall){x, y}));
    load_wall_fields_into_editor(app, x, y);
}

static bool entity_name_exists_outside_selected(const App *app, const char *name) {
    for (size_t i = 0; i < app->entities.length; i++) {
        if (entity_is_selected(app, (int)i)) continue;
        if (strcmp(app->entities.data[i].name, name) == 0) return true;
    }
    return false;
}

static bool assigned_entity_name_exists(char (*assigned)[ENTITY_NAME_SIZE], size_t count, const char *name) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(assigned[i], name) == 0) return true;
    }
    return false;
}

static void make_unique_selected_entity_names(App *app) {
    if (app->selected_entities.length == 0 || app->entity_name_edit) return;

    if (app->selected_entities.length == 1) {
        int entity_index = app->selected_entities.data[0];
        if (!selected_entity_is_valid(app, entity_index)) return;

        char clean_name[ENTITY_NAME_SIZE] = {0};
        make_unique_entity_name_except(app, app->brush_entity_name, entity_index, clean_name, sizeof(clean_name));
        snprintf(app->entities.data[entity_index].name, sizeof(app->entities.data[entity_index].name), "%s", clean_name);
        snprintf(app->brush_entity_name, sizeof(app->brush_entity_name), "%s", clean_name);
        return;
    }

    char base[ENTITY_NAME_SIZE] = {0};
    sanitize_identifier(base, sizeof(base), app->brush_entity_name, "entity", false);

    char root[ENTITY_NAME_SIZE] = {0};
    snprintf(root, sizeof(root), "%s", base);
    strip_numeric_suffix(root);
    if (root[0] == '\0') snprintf(root, sizeof(root), "entity");

    size_t count = app->selected_entities.length;
    char (*assigned)[ENTITY_NAME_SIZE] = calloc(count, sizeof(*assigned));
    if (!assigned) {
        log_error("Out of memory\n");
        exit(1);
    }

    unsigned int suffix = 1;
    for (size_t i = 0; i < count; i++) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) continue;

        char candidate[ENTITY_NAME_SIZE] = {0};
        for (;;) {
            if (suffix == 1) snprintf(candidate, sizeof(candidate), "%s", base);
            else snprintf(candidate, sizeof(candidate), "%s_%u", root, suffix);
            suffix++;

            if (entity_name_exists_outside_selected(app, candidate)) continue;
            if (assigned_entity_name_exists(assigned, i, candidate)) continue;
            break;
        }

        snprintf(assigned[i], ENTITY_NAME_SIZE, "%s", candidate);
        snprintf(app->entities.data[entity_index].name, sizeof(app->entities.data[entity_index].name), "%s", candidate);
    }

    snprintf(app->brush_entity_name, sizeof(app->brush_entity_name), "%s", assigned[0]);
    free(assigned);
}

static void refresh_entity_editor_primary(App *app) {
    if (app->selected_entities.length == 0) return;
    int entity_index = app->selected_entities.data[0];
    if (selected_entity_is_valid(app, entity_index)) {
        load_entity_fields_into_editor(app, entity_index);
    }
}

static void refresh_wall_editor_primary(App *app) {
    if (app->selected_walls.length == 0) return;
    SelectedWall wall = app->selected_walls.data[0];
    if (wall_map_inside(&app->map, wall.x, wall.y)) {
        load_wall_fields_into_editor(app, wall.x, wall.y);
    }
}

static void add_entity_to_selection(App *app, int entity_index) {
    if (!selected_entity_is_valid(app, entity_index)) return;
    if (app->selection_kind == SELECTION_WALL) return;

    if (app->selection_kind == SELECTION_NONE) {
        app->selection_kind = SELECTION_ENTITY;
        app->selected_walls.length = 0;
    }

    if (selected_entity_slot(app, entity_index) < 0) {
        da_append(&app->selected_entities, entity_index);
    }

    if (app->selected_entities.length == 1 || !editing_entity_is_valid(app)) {
        load_entity_fields_into_editor(app, entity_index);
    }
}

static void add_wall_to_selection(App *app, int x, int y) {
    int *cell = wall_map_cell(&app->map, x, y);
    if (!cell || *cell < 0) return;
    if (app->selection_kind == SELECTION_ENTITY) return;

    if (app->selection_kind == SELECTION_NONE) {
        app->selection_kind = SELECTION_WALL;
        app->selected_entities.length = 0;
    }

    if (selected_wall_slot(app, x, y) < 0) {
        da_append(&app->selected_walls, ((SelectedWall){x, y}));
    }

    if (app->selected_walls.length == 1 || !app->editing_wall) {
        load_wall_fields_into_editor(app, x, y);
    }
}

static void remove_entity_from_selection(App *app, int entity_index) {
    int slot = selected_entity_slot(app, entity_index);
    if (slot < 0) return;

    if ((size_t)slot + 1 < app->selected_entities.length) {
        memmove(
            &app->selected_entities.data[slot],
            &app->selected_entities.data[slot + 1],
            (app->selected_entities.length - (size_t)slot - 1) * sizeof(app->selected_entities.data[0]));
    }
    app->selected_entities.length--;

    if (app->selected_entities.length == 0) {
        clear_edit_selection(app);
    } else {
        refresh_entity_editor_primary(app);
    }
}

static void remove_wall_from_selection(App *app, int x, int y) {
    int slot = selected_wall_slot(app, x, y);
    if (slot < 0) return;

    if ((size_t)slot + 1 < app->selected_walls.length) {
        memmove(
            &app->selected_walls.data[slot],
            &app->selected_walls.data[slot + 1],
            (app->selected_walls.length - (size_t)slot - 1) * sizeof(app->selected_walls.data[0]));
    }
    app->selected_walls.length--;

    if (app->selected_walls.length == 0) {
        clear_edit_selection(app);
    } else {
        refresh_wall_editor_primary(app);
    }
}

static void toggle_entity_selection(App *app, int entity_index) {
    if (entity_is_selected(app, entity_index)) remove_entity_from_selection(app, entity_index);
    else add_entity_to_selection(app, entity_index);
}

static void toggle_wall_selection(App *app, int x, int y) {
    if (wall_is_selected(app, x, y)) remove_wall_from_selection(app, x, y);
    else add_wall_to_selection(app, x, y);
}

static void prune_invalid_selection(App *app) {
    for (size_t i = 0; i < app->selected_entities.length;) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) {
            if (i + 1 < app->selected_entities.length) {
                memmove(
                    &app->selected_entities.data[i],
                    &app->selected_entities.data[i + 1],
                    (app->selected_entities.length - i - 1) * sizeof(app->selected_entities.data[0]));
            }
            app->selected_entities.length--;
        } else {
            i++;
        }
    }

    for (size_t i = 0; i < app->selected_walls.length;) {
        SelectedWall wall = app->selected_walls.data[i];
        int *cell = wall_map_cell(&app->map, wall.x, wall.y);
        if (!cell || *cell < 0) {
            if (i + 1 < app->selected_walls.length) {
                memmove(
                    &app->selected_walls.data[i],
                    &app->selected_walls.data[i + 1],
                    (app->selected_walls.length - i - 1) * sizeof(app->selected_walls.data[0]));
            }
            app->selected_walls.length--;
        } else {
            i++;
        }
    }

    if (!selection_has_items(app)) {
        app->selection_kind = SELECTION_NONE;
        app->editing_entity = NO_SELECTION;
        app->editing_wall = false;
        app->editing_wall_x = NO_SELECTION;
        app->editing_wall_y = NO_SELECTION;
    }
}

static void clear_clipboard(App *app) {
    app->clipboard_kind = SELECTION_NONE;
    app->clipboard_entities.length = 0;
    app->clipboard_walls.length = 0;
    app->clipboard_wall_width = 0;
    app->clipboard_wall_height = 0;
}

static bool selected_entities_center(App *app, Vector2 *center) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY || app->selected_entities.length == 0) return false;

    bool found = false;
    float min_x = 0.0f;
    float max_x = 0.0f;
    float min_y = 0.0f;
    float max_y = 0.0f;

    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) continue;

        Vector2 pos = app->entities.data[entity_index].pos;
        if (!found) {
            min_x = max_x = pos.x;
            min_y = max_y = pos.y;
            found = true;
        } else {
            if (pos.x < min_x) min_x = pos.x;
            if (pos.x > max_x) max_x = pos.x;
            if (pos.y < min_y) min_y = pos.y;
            if (pos.y > max_y) max_y = pos.y;
        }
    }

    if (!found) return false;
    *center = (Vector2){(min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f};
    return true;
}

static bool selected_walls_bounds(App *app, int *min_x, int *min_y, int *max_x, int *max_y) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_WALL || app->selected_walls.length == 0) return false;

    bool found = false;
    for (size_t i = 0; i < app->selected_walls.length; i++) {
        SelectedWall wall = app->selected_walls.data[i];
        int *cell = wall_map_cell(&app->map, wall.x, wall.y);
        if (!cell || *cell < 0) continue;

        if (!found) {
            *min_x = *max_x = wall.x;
            *min_y = *max_y = wall.y;
            found = true;
        } else {
            if (wall.x < *min_x) *min_x = wall.x;
            if (wall.x > *max_x) *max_x = wall.x;
            if (wall.y < *min_y) *min_y = wall.y;
            if (wall.y > *max_y) *max_y = wall.y;
        }
    }

    return found;
}

static void copy_active_selection(App *app) {
    prune_invalid_selection(app);
    if (!selection_has_items(app)) {
        set_status(app, "Nothing selected to copy");
        return;
    }

    clear_clipboard(app);

    if (app->selection_kind == SELECTION_ENTITY) {
        Vector2 center = {0.0f, 0.0f};
        if (!selected_entities_center(app, &center)) {
            set_status(app, "Nothing selected to copy");
            return;
        }

        app->clipboard_kind = SELECTION_ENTITY;
        for (size_t i = 0; i < app->selected_entities.length; i++) {
            int entity_index = app->selected_entities.data[i];
            if (!selected_entity_is_valid(app, entity_index)) continue;

            PlacedEntity *entity = &app->entities.data[entity_index];
            ClipboardEntity item = {
                .entity = *entity,
                .offset = {entity->pos.x - center.x, entity->pos.y - center.y},
            };
            da_append(&app->clipboard_entities, item);
        }

        set_status(app, "Copied %u entities", (unsigned int)app->clipboard_entities.length);
        return;
    }

    if (app->selection_kind == SELECTION_WALL) {
        int min_x = 0;
        int min_y = 0;
        int max_x = 0;
        int max_y = 0;
        if (!selected_walls_bounds(app, &min_x, &min_y, &max_x, &max_y)) {
            set_status(app, "Nothing selected to copy");
            return;
        }

        app->clipboard_kind = SELECTION_WALL;
        app->clipboard_wall_width = max_x - min_x + 1;
        app->clipboard_wall_height = max_y - min_y + 1;

        for (size_t i = 0; i < app->selected_walls.length; i++) {
            SelectedWall wall = app->selected_walls.data[i];
            int *cell = wall_map_cell(&app->map, wall.x, wall.y);
            if (!cell || *cell < 0) continue;

            ClipboardWall item = {
                .offset_x = wall.x - min_x,
                .offset_y = wall.y - min_y,
                .asset_index = *cell,
            };
            da_append(&app->clipboard_walls, item);
        }

        set_status(app, "Copied %u walls", (unsigned int)app->clipboard_walls.length);
    }
}

static bool clipboard_entity_offset_bounds(const App *app, float *min_x, float *min_y, float *max_x, float *max_y) {
    if (app->clipboard_kind != SELECTION_ENTITY || app->clipboard_entities.length == 0) return false;

    *min_x = *max_x = app->clipboard_entities.data[0].offset.x;
    *min_y = *max_y = app->clipboard_entities.data[0].offset.y;

    for (size_t i = 1; i < app->clipboard_entities.length; i++) {
        Vector2 offset = app->clipboard_entities.data[i].offset;
        if (offset.x < *min_x) *min_x = offset.x;
        if (offset.x > *max_x) *max_x = offset.x;
        if (offset.y < *min_y) *min_y = offset.y;
        if (offset.y > *max_y) *max_y = offset.y;
    }

    return true;
}

static void paste_clipboard_entities_at(App *app, Vector2 center) {
    float min_offset_x = 0.0f;
    float min_offset_y = 0.0f;
    float max_offset_x = 0.0f;
    float max_offset_y = 0.0f;
    if (!clipboard_entity_offset_bounds(app, &min_offset_x, &min_offset_y, &max_offset_x, &max_offset_y)) {
        set_status(app, "Nothing copied to paste");
        return;
    }

    float max_map_x = (float)app->map.cols - 0.001f;
    float max_map_y = (float)app->map.rows - 0.001f;
    float min_center_x = -min_offset_x;
    float max_center_x = max_map_x - max_offset_x;
    float min_center_y = -min_offset_y;
    float max_center_y = max_map_y - max_offset_y;
    if (min_center_x > max_center_x || min_center_y > max_center_y) {
        set_status(app, "Copied entities do not fit in map");
        return;
    }

    center.x = clamp_float(center.x, min_center_x, max_center_x);
    center.y = clamp_float(center.y, min_center_y, max_center_y);

    clear_edit_selection(app);
    app->selection_kind = SELECTION_ENTITY;

    for (size_t i = 0; i < app->clipboard_entities.length; i++) {
        ClipboardEntity *item = &app->clipboard_entities.data[i];
        PlacedEntity entity = item->entity;
        entity.pos = (Vector2){center.x + item->offset.x, center.y + item->offset.y};
        make_unique_entity_name(app, item->entity.name, entity.name, sizeof(entity.name));
        if (entity.update_fn[0] != '\0') remember_update_fn(&app->update_fns, entity.update_fn, NULL, 0);

        da_append(&app->entities, entity);
        da_append(&app->selected_entities, (int)app->entities.length - 1);
    }

    refresh_entity_editor_primary(app);
    set_status(app, "Pasted %u entities", (unsigned int)app->clipboard_entities.length);
}

static void paste_clipboard_walls_at(App *app, int center_x, int center_y) {
    if (app->clipboard_kind != SELECTION_WALL || app->clipboard_walls.length == 0) {
        set_status(app, "Nothing copied to paste");
        return;
    }

    if (app->clipboard_wall_width <= 0 ||
        app->clipboard_wall_height <= 0 ||
        app->clipboard_wall_width > app->map.cols ||
        app->clipboard_wall_height > app->map.rows) {
        set_status(app, "Copied walls do not fit in map");
        return;
    }

    int min_x = center_x - app->clipboard_wall_width / 2;
    int min_y = center_y - app->clipboard_wall_height / 2;
    min_x = clamp_int(min_x, 0, app->map.cols - app->clipboard_wall_width);
    min_y = clamp_int(min_y, 0, app->map.rows - app->clipboard_wall_height);

    clear_edit_selection(app);
    app->selection_kind = SELECTION_WALL;

    for (size_t i = 0; i < app->clipboard_walls.length; i++) {
        ClipboardWall item = app->clipboard_walls.data[i];
        int x = min_x + item.offset_x;
        int y = min_y + item.offset_y;
        int *cell = wall_map_cell(&app->map, x, y);
        if (!cell) continue;

        *cell = item.asset_index;
        da_append(&app->selected_walls, ((SelectedWall){x, y}));
    }

    refresh_wall_editor_primary(app);
    set_status(app, "Pasted %u walls", (unsigned int)app->selected_walls.length);
}

static void paste_clipboard_at_mouse(App *app, Rectangle map_bounds) {
    if (app->clipboard_kind == SELECTION_NONE) {
        set_status(app, "Nothing copied to paste");
        return;
    }

    Vector2 mouse = GetMousePosition();
    if (!CheckCollisionPointRec(mouse, map_bounds)) {
        set_status(app, "Move mouse over the map to paste");
        return;
    }

    Vector2 world = GetScreenToWorld2D(mouse, app->camera);
    int cell_x = 0;
    int cell_y = 0;
    world_to_cell_clamped(app, world, &cell_x, &cell_y);

    if (app->clipboard_kind == SELECTION_ENTITY) {
        paste_clipboard_entities_at(app, world);
    } else if (app->clipboard_kind == SELECTION_WALL) {
        paste_clipboard_walls_at(app, cell_x, cell_y);
    }
}

static void delete_active_selection(App *app) {
    prune_invalid_selection(app);
    if (!selection_has_items(app)) {
        set_status(app, "Nothing selected to delete");
        return;
    }

    if (app->selection_kind == SELECTION_ENTITY) {
        bool *remove = calloc(app->entities.length, sizeof(*remove));
        if (!remove) {
            log_error("Out of memory\n");
            exit(1);
        }

        size_t delete_count = 0;
        for (size_t i = 0; i < app->selected_entities.length; i++) {
            int entity_index = app->selected_entities.data[i];
            if (!selected_entity_is_valid(app, entity_index) || remove[entity_index]) continue;
            remove[entity_index] = true;
            delete_count++;
        }

        size_t write = 0;
        for (size_t read = 0; read < app->entities.length; read++) {
            if (remove[read]) continue;
            if (write != read) app->entities.data[write] = app->entities.data[read];
            write++;
        }
        app->entities.length = write;
        free(remove);

        clear_edit_selection(app);
        set_status(app, "Deleted %u entities", (unsigned int)delete_count);
        return;
    }

    if (app->selection_kind == SELECTION_WALL) {
        size_t delete_count = 0;
        for (size_t i = 0; i < app->selected_walls.length; i++) {
            SelectedWall wall = app->selected_walls.data[i];
            int *cell = wall_map_cell(&app->map, wall.x, wall.y);
            if (!cell || *cell < 0) continue;
            *cell = -1;
            delete_count++;
        }

        clear_edit_selection(app);
        set_status(app, "Deleted %u walls", (unsigned int)delete_count);
    }
}

static void sync_selected_entity_from_editor(App *app) {
    if (app->selection_kind != SELECTION_ENTITY || app->selected_entities.length == 0) {
        if (!editing_entity_is_valid(app)) return;
        app->selection_kind = SELECTION_ENTITY;
        if (selected_entity_slot(app, app->editing_entity) < 0) {
            da_append(&app->selected_entities, app->editing_entity);
        }
    }

    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY || app->selected_entities.length == 0) return;
    if (app->selected_entities.length > 1) return;

    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) continue;

        PlacedEntity *entity = &app->entities.data[entity_index];
        if (app->selected_asset >= 0 && app->selected_asset < (int)app->assets.length) entity->asset_index = app->selected_asset;
        entity->vdiv = app->brush_vdiv;
        entity->hdiv = app->brush_hdiv;
        entity->vmove = app->brush_vmove;
        entity->collision_threshold = app->brush_collision_threshold;
        entity->collision_mask = app->brush_collision_mask;
        entity->disabled = app->brush_disabled;
        entity->exported = app->brush_exported;
        snprintf(entity->update_fn, sizeof(entity->update_fn), "%s", app->brush_update_fn);
        if (entity->update_fn[0] != '\0') remember_update_fn(&app->update_fns, entity->update_fn, NULL, 0);
    }

    make_unique_selected_entity_names(app);
}

static void apply_selected_entities_vdiv(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].vdiv = app->brush_vdiv;
    }
}

static void apply_selected_entities_hdiv(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].hdiv = app->brush_hdiv;
    }
}

static void apply_selected_entities_vmove(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].vmove = app->brush_vmove;
    }
}

static void apply_selected_entities_threshold(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].collision_threshold = app->brush_collision_threshold;
    }
}

static void apply_selected_entities_collision_mask(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].collision_mask = app->brush_collision_mask;
    }
}

static void apply_selected_entities_disabled(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].disabled = app->brush_disabled;
    }
}

static void apply_selected_entities_exported(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].exported = app->brush_exported;
    }
}

static void apply_selected_entities_update_fn(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) continue;

        PlacedEntity *entity = &app->entities.data[entity_index];
        snprintf(entity->update_fn, sizeof(entity->update_fn), "%s", app->brush_update_fn);
        if (entity->update_fn[0] != '\0') remember_update_fn(&app->update_fns, entity->update_fn, NULL, 0);
    }
}

static void apply_selected_entities_name(App *app) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY) return;
    make_unique_selected_entity_names(app);
}

static void apply_selected_asset_to_edit(App *app) {
    if (app->selected_asset < 0 || app->selected_asset >= (int)app->assets.length) return;

    prune_invalid_selection(app);

    if (app->selection_kind == SELECTION_ENTITY && app->selected_entities.length > 0) {
        for (size_t i = 0; i < app->selected_entities.length; i++) {
            int entity_index = app->selected_entities.data[i];
            if (selected_entity_is_valid(app, entity_index)) app->entities.data[entity_index].asset_index = app->selected_asset;
        }
    } else if (app->selection_kind == SELECTION_WALL && app->selected_walls.length > 0) {
        for (size_t i = 0; i < app->selected_walls.length; i++) {
            SelectedWall wall = app->selected_walls.data[i];
            int *cell = wall_map_cell(&app->map, wall.x, wall.y);
            if (cell) *cell = app->selected_asset;
        }
    } else if (editing_entity_is_valid(app)) {
        app->entities.data[app->editing_entity].asset_index = app->selected_asset;
    } else if (app->editing_wall) {
        int *cell = wall_map_cell(&app->map, app->editing_wall_x, app->editing_wall_y);
        if (cell) *cell = app->selected_asset;
    }
}

static float distance_between(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

static void zoom_at_screen_point(App *app, Vector2 screen_point, float scale) {
    if (scale <= 0.0f || !isfinite(scale)) return;

    Vector2 before = GetScreenToWorld2D(screen_point, app->camera);
    app->camera.zoom = clamp_float(app->camera.zoom * scale, 0.05f, 96.0f);
    Vector2 after = GetScreenToWorld2D(screen_point, app->camera);
    app->camera.target.x += before.x - after.x;
    app->camera.target.y += before.y - after.y;
}

static Vector2 normalized_player_dir(const App *app) {
    Vector2 dir = app->player_dir;
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len <= 0.0001f || !isfinite(len)) return (Vector2){0.0f, 1.0f};
    return (Vector2){dir.x / len, dir.y / len};
}

static void refresh_player_text(App *app) {
    snprintf(app->player_pos_x_text, sizeof(app->player_pos_x_text), "%.3f", app->player_pos.x);
    snprintf(app->player_pos_y_text, sizeof(app->player_pos_y_text), "%.3f", app->player_pos.y);
    snprintf(app->player_dir_x_text, sizeof(app->player_dir_x_text), "%.3f", app->player_dir.x);
    snprintf(app->player_dir_y_text, sizeof(app->player_dir_y_text), "%.3f", app->player_dir.y);
    snprintf(app->player_threshold_text, sizeof(app->player_threshold_text), "%.3f", app->player_collision_threshold);
}

static void set_player_pos(App *app, Vector2 pos) {
    app->player_pos.x = clamp_float(pos.x, 0.0f, (float)app->map.cols - 0.001f);
    app->player_pos.y = clamp_float(pos.y, 0.0f, (float)app->map.rows - 0.001f);
    refresh_player_text(app);
}

static bool player_marker_hit_at(const App *app, Vector2 pos) {
    float radius = fmaxf(0.18f, 3.0f / app->camera.zoom);
    float dx = pos.x - app->player_pos.x;
    float dy = pos.y - app->player_pos.y;
    return (dx * dx + dy * dy) <= radius * radius;
}

static int nearest_entity_at(const App *app, Vector2 pos, float radius) {
    int best_index = -1;
    float best_dist = radius * radius;
    for (size_t i = 0; i < app->entities.length; i++) {
        const PlacedEntity *entity = &app->entities.data[i];
        float dx = entity->pos.x - pos.x;
        float dy = entity->pos.y - pos.y;
        float dist = dx * dx + dy * dy;
        if (dist <= best_dist) {
            best_dist = dist;
            best_index = (int)i;
        }
    }
    return best_index;
}

static void load_player_into_editor(App *app) {
    clear_edit_selection(app);
    app->brush = BRUSH_PLAYER;
    refresh_player_text(app);
    set_status(app, "Editing player");
}

static bool select_existing_at(App *app, Vector2 world, int cell_x, int cell_y) {
    int entity_index = nearest_entity_at(app, world, fmaxf(0.35f, 8.0f / app->camera.zoom));
    if (entity_index >= 0) {
        load_entity_into_editor(app, entity_index);
        return true;
    }

    if (player_marker_hit_at(app, world)) {
        load_player_into_editor(app);
        return true;
    }

    int *cell = wall_map_cell(&app->map, cell_x, cell_y);
    if (cell && *cell >= 0) {
        load_wall_into_editor(app, cell_x, cell_y);
        return true;
    }

    return false;
}

static Rectangle normalized_world_rect(Vector2 a, Vector2 b) {
    float x0 = a.x < b.x ? a.x : b.x;
    float x1 = a.x > b.x ? a.x : b.x;
    float y0 = a.y < b.y ? a.y : b.y;
    float y1 = a.y > b.y ? a.y : b.y;
    return (Rectangle){x0, y0, x1 - x0, y1 - y0};
}

static bool entity_inside_rect(const PlacedEntity *entity, Rectangle rect) {
    return entity->pos.x >= rect.x &&
        entity->pos.y >= rect.y &&
        entity->pos.x <= rect.x + rect.width &&
        entity->pos.y <= rect.y + rect.height;
}

static int count_entities_inside_rect(const App *app, Rectangle rect) {
    int count = 0;
    for (size_t i = 0; i < app->entities.length; i++) {
        if (entity_inside_rect(&app->entities.data[i], rect)) count++;
    }
    return count;
}

static void select_entities_inside_rect(App *app, Rectangle rect) {
    for (size_t i = 0; i < app->entities.length; i++) {
        if (entity_inside_rect(&app->entities.data[i], rect)) add_entity_to_selection(app, (int)i);
    }
}

static void selection_rect_cells(const App *app, int *min_x, int *min_y, int *max_x, int *max_y) {
    *min_x = app->selection_drag_start_x < app->selection_drag_end_x ? app->selection_drag_start_x : app->selection_drag_end_x;
    *max_x = app->selection_drag_start_x > app->selection_drag_end_x ? app->selection_drag_start_x : app->selection_drag_end_x;
    *min_y = app->selection_drag_start_y < app->selection_drag_end_y ? app->selection_drag_start_y : app->selection_drag_end_y;
    *max_y = app->selection_drag_start_y > app->selection_drag_end_y ? app->selection_drag_start_y : app->selection_drag_end_y;
    *min_x = clamp_int(*min_x, 0, app->map.cols - 1);
    *max_x = clamp_int(*max_x, 0, app->map.cols - 1);
    *min_y = clamp_int(*min_y, 0, app->map.rows - 1);
    *max_y = clamp_int(*max_y, 0, app->map.rows - 1);
}

static int count_walls_inside_selection_rect(const App *app) {
    int min_x = 0;
    int min_y = 0;
    int max_x = 0;
    int max_y = 0;
    selection_rect_cells(app, &min_x, &min_y, &max_x, &max_y);

    int count = 0;
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            if (wall_map_inside(&app->map, x, y) && app->map.data[y * app->map.cols + x] >= 0) count++;
        }
    }
    return count;
}

static void select_walls_inside_selection_rect(App *app) {
    int min_x = 0;
    int min_y = 0;
    int max_x = 0;
    int max_y = 0;
    selection_rect_cells(app, &min_x, &min_y, &max_x, &max_y);

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            add_wall_to_selection(app, x, y);
        }
    }
}

static void apply_shift_click_selection(App *app, Vector2 world, int cell_x, int cell_y) {
    if (app->selection_kind == SELECTION_ENTITY || app->selection_kind == SELECTION_NONE) {
        int entity_index = nearest_entity_at(app, world, fmaxf(0.35f, 8.0f / app->camera.zoom));
        if (entity_index >= 0) {
            toggle_entity_selection(app, entity_index);
            return;
        }
    }

    if (app->selection_kind == SELECTION_WALL || app->selection_kind == SELECTION_NONE) {
        int *cell = wall_map_cell(&app->map, cell_x, cell_y);
        if (cell && *cell >= 0) toggle_wall_selection(app, cell_x, cell_y);
    }
}

static void apply_shift_rect_selection(App *app) {
    Rectangle rect = normalized_world_rect(app->selection_drag_start, app->selection_drag_end);

    if (app->selection_kind == SELECTION_NONE) {
        if (count_entities_inside_rect(app, rect) > 0) {
            app->selection_kind = SELECTION_ENTITY;
        } else if (count_walls_inside_selection_rect(app) > 0) {
            app->selection_kind = SELECTION_WALL;
        }
    }

    if (app->selection_kind == SELECTION_ENTITY) {
        select_entities_inside_rect(app, rect);
    } else if (app->selection_kind == SELECTION_WALL) {
        select_walls_inside_selection_rect(app);
    }
}

static void begin_shift_selection_drag(App *app, Vector2 world, int cell_x, int cell_y) {
    app->selection_dragging = true;
    app->selection_drag_moved = false;
    app->selection_drag_start = world;
    app->selection_drag_end = world;
    app->selection_drag_start_x = cell_x;
    app->selection_drag_start_y = cell_y;
    app->selection_drag_end_x = cell_x;
    app->selection_drag_end_y = cell_y;

    if (app->selection_kind == SELECTION_NONE) {
        int entity_index = nearest_entity_at(app, world, fmaxf(0.35f, 8.0f / app->camera.zoom));
        if (entity_index >= 0) {
            app->selection_kind = SELECTION_ENTITY;
        } else {
            int *cell = wall_map_cell(&app->map, cell_x, cell_y);
            if (cell && *cell >= 0) app->selection_kind = SELECTION_WALL;
        }
    }
}

static int selected_entity_at(const App *app, Vector2 pos, float radius) {
    if (app->selection_kind != SELECTION_ENTITY) return NO_SELECTION;

    int best_index = NO_SELECTION;
    float best_dist = radius * radius;
    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) continue;

        const PlacedEntity *entity = &app->entities.data[entity_index];
        float dx = entity->pos.x - pos.x;
        float dy = entity->pos.y - pos.y;
        float dist = dx * dx + dy * dy;
        if (dist <= best_dist) {
            best_dist = dist;
            best_index = entity_index;
        }
    }
    return best_index;
}

static bool selection_move_hit_at(const App *app, Vector2 world, int cell_x, int cell_y) {
    if (app->selection_kind == SELECTION_ENTITY) {
        return selected_entity_at(app, world, fmaxf(0.35f, 8.0f / app->camera.zoom)) != NO_SELECTION;
    }

    if (app->selection_kind == SELECTION_WALL) {
        return wall_is_selected(app, cell_x, cell_y);
    }

    return false;
}

static void begin_selection_move_drag(App *app, Vector2 world, int cell_x, int cell_y) {
    app->selection_move_dragging = true;
    app->selection_move_moved = false;
    app->selection_move_start = world;
    app->selection_move_last = world;
    app->selection_move_start_x = cell_x;
    app->selection_move_start_y = cell_y;
    app->selection_move_last_x = cell_x;
    app->selection_move_last_y = cell_y;
    app->selection_dragging = false;
    app->wall_dragging = false;
}

static bool selection_move_threshold_reached(const App *app, Vector2 world, int cell_x, int cell_y) {
    if (app->selection_kind == SELECTION_WALL) {
        return cell_x != app->selection_move_start_x || cell_y != app->selection_move_start_y;
    }

    return fabsf(world.x - app->selection_move_start.x) > 0.05f ||
        fabsf(world.y - app->selection_move_start.y) > 0.05f;
}

static bool clamp_selected_entities_delta(App *app, Vector2 *delta) {
    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_ENTITY || app->selected_entities.length == 0) return false;

    bool found = false;
    float min_x = 0.0f;
    float max_x = 0.0f;
    float min_y = 0.0f;
    float max_y = 0.0f;

    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) continue;

        Vector2 pos = app->entities.data[entity_index].pos;
        if (!found) {
            min_x = max_x = pos.x;
            min_y = max_y = pos.y;
            found = true;
        } else {
            if (pos.x < min_x) min_x = pos.x;
            if (pos.x > max_x) max_x = pos.x;
            if (pos.y < min_y) min_y = pos.y;
            if (pos.y > max_y) max_y = pos.y;
        }
    }

    if (!found) return false;

    float max_map_x = (float)app->map.cols - 0.001f;
    float max_map_y = (float)app->map.rows - 0.001f;
    delta->x = clamp_float(delta->x, -min_x, max_map_x - max_x);
    delta->y = clamp_float(delta->y, -min_y, max_map_y - max_y);
    return fabsf(delta->x) > 0.000001f || fabsf(delta->y) > 0.000001f;
}

static bool move_selected_entities_by(App *app, Vector2 *delta) {
    if (!clamp_selected_entities_delta(app, delta)) return false;

    for (size_t i = 0; i < app->selected_entities.length; i++) {
        int entity_index = app->selected_entities.data[i];
        if (!selected_entity_is_valid(app, entity_index)) continue;

        app->entities.data[entity_index].pos.x += delta->x;
        app->entities.data[entity_index].pos.y += delta->y;
    }

    return true;
}

static bool move_selected_walls_by(App *app, int dx, int dy) {
    if (dx == 0 && dy == 0) return false;

    prune_invalid_selection(app);
    if (app->selection_kind != SELECTION_WALL || app->selected_walls.length == 0) return false;

    for (size_t i = 0; i < app->selected_walls.length; i++) {
        SelectedWall wall = app->selected_walls.data[i];
        int dst_x = wall.x + dx;
        int dst_y = wall.y + dy;
        if (!wall_map_inside(&app->map, dst_x, dst_y)) return false;

        int *dst = wall_map_cell(&app->map, dst_x, dst_y);
        if (dst && *dst >= 0 && selected_wall_slot(app, dst_x, dst_y) < 0) return false;
    }

    size_t count = app->selected_walls.length;
    int *assets = malloc(count * sizeof(*assets));
    if (!assets) {
        log_error("Out of memory\n");
        exit(1);
    }

    for (size_t i = 0; i < count; i++) {
        SelectedWall wall = app->selected_walls.data[i];
        int *src = wall_map_cell(&app->map, wall.x, wall.y);
        if (!src || *src < 0) {
            free(assets);
            prune_invalid_selection(app);
            return false;
        }
        assets[i] = *src;
    }

    bool editing_wall_selected = app->editing_wall &&
        selected_wall_slot(app, app->editing_wall_x, app->editing_wall_y) >= 0;

    for (size_t i = 0; i < count; i++) {
        SelectedWall wall = app->selected_walls.data[i];
        int *src = wall_map_cell(&app->map, wall.x, wall.y);
        if (src) *src = -1;
    }

    for (size_t i = 0; i < count; i++) {
        SelectedWall *wall = &app->selected_walls.data[i];
        int *dst = wall_map_cell(&app->map, wall->x + dx, wall->y + dy);
        if (dst) *dst = assets[i];
        wall->x += dx;
        wall->y += dy;
    }

    free(assets);

    if (editing_wall_selected) {
        app->editing_wall_x += dx;
        app->editing_wall_y += dy;
    }

    return true;
}

static void update_selection_move_drag(App *app, Vector2 world, int cell_x, int cell_y) {
    if (!app->selection_move_moved && !selection_move_threshold_reached(app, world, cell_x, cell_y)) return;

    if (app->selection_kind == SELECTION_ENTITY) {
        Vector2 delta = {
            world.x - app->selection_move_last.x,
            world.y - app->selection_move_last.y,
        };
        if (move_selected_entities_by(app, &delta)) {
            app->selection_move_last.x += delta.x;
            app->selection_move_last.y += delta.y;
            app->selection_move_moved = true;
        }
        return;
    }

    if (app->selection_kind == SELECTION_WALL) {
        int dx = cell_x - app->selection_move_last_x;
        int dy = cell_y - app->selection_move_last_y;
        if (move_selected_walls_by(app, dx, dy)) {
            app->selection_move_last_x += dx;
            app->selection_move_last_y += dy;
            app->selection_move_moved = true;
        }
    }
}

static void finish_selection_move_drag(App *app) {
    bool moved = app->selection_move_moved;
    app->selection_move_dragging = false;
    app->selection_move_moved = false;
    prune_invalid_selection(app);

    if (!moved) return;

    if (app->selection_kind == SELECTION_ENTITY && app->selected_entities.length > 0) {
        set_status(app, "Moved %u entities", (unsigned int)app->selected_entities.length);
    } else if (app->selection_kind == SELECTION_WALL && app->selected_walls.length > 0) {
        set_status(app, "Moved %u walls", (unsigned int)app->selected_walls.length);
    }
}

static void append_float_literal(String *out, float value) {
    if (!isfinite(value) || fabsf(value) < 0.0000005f) value = 0.0f;

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.6f", value);

    char *dot = strchr(buffer, '.');
    if (dot) {
        char *end = buffer + strlen(buffer) - 1;
        while (end > dot + 1 && *end == '0') {
            *end = '\0';
            end--;
        }
    }

    str_appendf(out, "%sf", buffer);
}

static void append_collision_mask_expression(String *out, const App *app, uint32_t mask) {
    if (mask == 0) {
        str_append(out, "YR_CMSK_NONE");
        return;
    }

    bool first = true;
    uint32_t known_bits = 0;
    for (size_t i = 0; i < app->collision_layers.length; i++) {
        const CollisionLayer *layer = &app->collision_layers.data[i];
        known_bits |= layer->value;
        if ((mask & layer->value) == 0) continue;

        if (!first) str_append(out, " | ");
        str_append(out, layer->symbol);
        first = false;
    }

    uint32_t unknown_bits = mask & ~known_bits;
    if (unknown_bits != 0) {
        if (!first) str_append(out, " | ");
        str_appendf(out, "0x%08Xu", unknown_bits);
    }
}

static void append_player_collision_mask_expression(String *out, const App *app) {
    str_append(out, "YR_CMSK_ALL");
    for (size_t i = 0; i < app->collision_layers.length; i++) {
        const CollisionLayer *layer = &app->collision_layers.data[i];
        if ((app->player_collision_mask & layer->value) != 0) continue;
        str_appendf(out, " & ~%s", layer->symbol);
    }
}

static uint32_t all_custom_collision_layer_bits(const CollisionLayers *layers) {
    uint32_t mask = 0;
    for (size_t i = 0; i < layers->length; i++) mask |= layers->data[i].value;
    return mask;
}

static void append_map_builder_state(String *out, const App *app) {
    str_append(out, "// " MAP_BUILDER_STATE_BEGIN "\n");
    str_appendf(out, "// version %d\n", MAP_BUILDER_STATE_VERSION);
    str_appendf(out, "// size %d %d\n", app->map.cols, app->map.rows);
    str_appendf(out, "// surface floor %s\n", asset_symbol_or_null(app, app->floor_asset));
    str_appendf(out, "// surface ceil %s\n", asset_symbol_or_null(app, app->ceil_asset));
    str_appendf(
        out,
        "// player %.9g %.9g %.9g %.9g %.9g 0x%08X\n",
        app->player_pos.x,
        app->player_pos.y,
        app->player_dir.x,
        app->player_dir.y,
        app->player_collision_threshold,
        app->player_collision_mask);

    for (size_t i = 0; i < app->collision_layers.length; i++) {
        const CollisionLayer *layer = &app->collision_layers.data[i];
        str_appendf(out, "// layer %s %u\n", layer->name, layer->shift);
    }

    for (size_t i = 0; i < app->update_fns.length; i++) {
        const UpdateFn *update_fn = &app->update_fns.data[i];
        str_appendf(out, "// update_fn %s\n", update_fn->name);
    }

    for (int y = 0; y < app->map.rows; y++) {
        for (int x = 0; x < app->map.cols; x++) {
            int asset_index = app->map.data[y * app->map.cols + x];
            if (!asset_index_is_valid(app, asset_index)) continue;
            str_appendf(out, "// wall %d %d %s\n", x, y, app->assets.data[asset_index].texture_symbol);
        }
    }

    for (size_t i = 0; i < app->entities.length; i++) {
        const PlacedEntity *entity = &app->entities.data[i];
        str_appendf(
            out,
            "// entity %s %s %.9g %.9g %.9g %.9g %.9g %d %.9g 0x%08X %d %s\n",
            entity->name,
            asset_symbol_or_null(app, entity->asset_index),
            entity->pos.x,
            entity->pos.y,
            entity->vdiv,
            entity->hdiv,
            entity->vmove,
            entity->disabled ? 1 : 0,
            entity->collision_threshold,
            entity->collision_mask,
            entity->exported ? 1 : 0,
            entity->update_fn[0] != '\0' ? entity->update_fn : "-");
    }

    str_append(out, "// " MAP_BUILDER_STATE_END "\n\n");
}

static void loaded_level_init(LoadedLevel *loaded) {
    memset(loaded, 0, sizeof(*loaded));
    loaded->floor_asset = -1;
    loaded->ceil_asset = -1;
    loaded->player_pos = (Vector2){DEFAULT_MAP_COLS * 0.5f, DEFAULT_MAP_ROWS * 0.5f};
    loaded->player_dir = (Vector2){0.0f, 1.0f};
    loaded->player_collision_threshold = 0.15f;
    loaded->player_collision_mask = UINT32_MAX;
}

static void loaded_level_free(LoadedLevel *loaded) {
    if (loaded->has_size) wall_map_free(&loaded->map);
    da_free(&loaded->entities);
    da_free(&loaded->collision_layers);
    da_free(&loaded->update_fns);
    loaded_level_init(loaded);
}

static bool read_file_null_terminated(const char *path, String *out) {
    FILE *file = fopen(path, "rb");
    if (!file) return false;

    bool ok = false;
    if (fseek(file, 0, SEEK_END) != 0) goto cleanup;
    long size = ftell(file);
    if (size < 0) goto cleanup;
    if (fseek(file, 0, SEEK_SET) != 0) goto cleanup;

    size_t old_length = out->length;
    da_reserve(out, old_length + (size_t)size + 1);
    size_t bytes_read = fread(out->data + old_length, 1, (size_t)size, file);
    if (bytes_read != (size_t)size || ferror(file)) goto cleanup;

    out->length = old_length + bytes_read;
    out->data[out->length] = '\0';
    ok = true;

cleanup:
    fclose(file);
    return ok;
}

static char *trim_left(char *text) {
    while (*text && isspace((unsigned char)*text)) text++;
    return text;
}

static void trim_right_in_place(char *text) {
    size_t len = strlen(text);
    while (len > 0 && isspace((unsigned char)text[len - 1])) {
        text[--len] = '\0';
    }
}

static char *state_line_payload(char *line) {
    char *payload = trim_left(line);
    if (payload[0] == '/' && payload[1] == '/') payload += 2;
    payload = trim_left(payload);
    trim_right_in_place(payload);
    return payload;
}

static bool loaded_collision_layer_exists(const LoadedLevel *loaded, const char *name) {
    for (size_t i = 0; i < loaded->collision_layers.length; i++) {
        if (strcmp(loaded->collision_layers.data[i].name, name) == 0) return true;
    }
    return false;
}

static bool loaded_level_add_collision_layer(LoadedLevel *loaded, const char *raw_name, unsigned int shift) {
    if (loaded->collision_layers.length >= MAX_CUSTOM_MASKS || shift == 0 || shift > MAX_CUSTOM_MASKS) return false;

    char input[MASK_NAME_SIZE] = {0};
    snprintf(input, sizeof(input), "%s", raw_name);
    for (char *c = input; *c; c++) *c = (char)toupper((unsigned char)*c);

    const char *name_src = input;
    if (strncmp(name_src, "YR_CMSK_", 5) == 0) name_src += 5;

    char name[MASK_NAME_SIZE] = {0};
    sanitize_identifier(name, sizeof(name), name_src, "LAYER", true);
    if (collision_layer_reserved(name) || loaded_collision_layer_exists(loaded, name)) return false;

    CollisionLayer layer = {
        .value = 1u << shift,
        .shift = shift,
    };
    snprintf(layer.name, sizeof(layer.name), "%s", name);
    snprintf(layer.symbol, sizeof(layer.symbol), "YR_CMSK_%s", name);
    da_append(&loaded->collision_layers, layer);
    return true;
}

static bool parse_state_line(App *app, LoadedLevel *loaded, const char *payload, int *missing_assets) {
    if (payload[0] == '\0' || strcmp(payload, MAP_BUILDER_STATE_BEGIN) == 0 || strcmp(payload, MAP_BUILDER_STATE_END) == 0) return true;

    int version = 0;
    if (sscanf(payload, "version %d", &version) == 1) {
        return version == MAP_BUILDER_STATE_VERSION;
    }

    int cols = 0;
    int rows = 0;
    if (sscanf(payload, "size %d %d", &cols, &rows) == 2) {
        if (cols < MIN_MAP_SIZE || rows < MIN_MAP_SIZE || cols > MAX_MAP_SIZE || rows > MAX_MAP_SIZE) return false;
        if (loaded->has_size) wall_map_free(&loaded->map);
        wall_map_init(&loaded->map, cols, rows);
        loaded->has_size = true;
        loaded->player_pos = (Vector2){(float)cols * 0.5f, (float)rows * 0.5f};
        return true;
    }

    char surface_name[16] = {0};
    char texture_symbol[128] = {0};
    if (sscanf(payload, "surface %15s %127s", surface_name, texture_symbol) == 2) {
        int asset_index = asset_index_from_symbol(app, texture_symbol);
        if (asset_index < 0 && strcmp(texture_symbol, "NULL_ASSET") != 0 && strcmp(texture_symbol, "0") != 0) (*missing_assets)++;
        if (strcmp(surface_name, "floor") == 0) loaded->floor_asset = asset_index;
        else if (strcmp(surface_name, "ceil") == 0) loaded->ceil_asset = asset_index;
        return true;
    }

    float player_x = 0.0f;
    float player_y = 0.0f;
    float dir_x = 0.0f;
    float dir_y = 0.0f;
    float threshold = 0.0f;
    unsigned int player_mask = UINT32_MAX;
    if (sscanf(payload, "player %f %f %f %f %f %x", &player_x, &player_y, &dir_x, &dir_y, &threshold, &player_mask) == 6) {
        loaded->player_pos = (Vector2){player_x, player_y};
        loaded->player_dir = (Vector2){dir_x, dir_y};
        loaded->player_collision_threshold = threshold;
        loaded->player_collision_mask = player_mask;
        return true;
    }

    char layer_name[MASK_NAME_SIZE] = {0};
    unsigned int shift = 0;
    if (sscanf(payload, "layer %31s %u", layer_name, &shift) == 2) {
        return loaded_level_add_collision_layer(loaded, layer_name, shift);
    }

    char update_fn_name[ENTITY_NAME_SIZE] = {0};
    if (sscanf(payload, "update_fn %63s", update_fn_name) == 1) {
        return remember_update_fn(&loaded->update_fns, update_fn_name, NULL, 0);
    }

    int x = 0;
    int y = 0;
    if (sscanf(payload, "wall %d %d %127s", &x, &y, texture_symbol) == 3) {
        if (!loaded->has_size || !wall_map_inside(&loaded->map, x, y)) return false;
        int asset_index = asset_index_from_symbol(app, texture_symbol);
        if (asset_index < 0) {
            if (strcmp(texture_symbol, "NULL_ASSET") != 0 && strcmp(texture_symbol, "0") != 0) (*missing_assets)++;
            return true;
        }
        int *cell = wall_map_cell(&loaded->map, x, y);
        if (cell) *cell = asset_index;
        return true;
    }

    char entity_name[ENTITY_NAME_SIZE] = {0};
    char ignored_fn[64] = {0};
    char update_fn_buf[ENTITY_NAME_SIZE] = {0};
    PlacedEntity entity = {0};
    int disabled = 0;
    unsigned int entity_mask = 0;
    int exported_val = 0;
    int entity_fields = sscanf(
            payload,
            "entity %63s %127s %f %f %f %f %f %d %f %x %63s %63s",
            entity_name,
            texture_symbol,
            &entity.pos.x,
            &entity.pos.y,
            &entity.vdiv,
            &entity.hdiv,
            &entity.vmove,
            &disabled,
            &entity.collision_threshold,
            &entity_mask,
            ignored_fn,
            update_fn_buf);
    /* fields 11+: ignored_fn = exported (int as string), update_fn_buf = optional */
    if (entity_fields == 12 || entity_fields == 11) {
        exported_val = (ignored_fn[0] != '-') ? atoi(ignored_fn) : 0;
        sanitize_identifier(entity.name, sizeof(entity.name), entity_name, "entity", false);
        entity.asset_index = asset_index_from_symbol(app, texture_symbol);
        if (entity.asset_index < 0 && strcmp(texture_symbol, "NULL_ASSET") != 0 && strcmp(texture_symbol, "0") != 0) (*missing_assets)++;
        entity.disabled = disabled != 0;
        entity.exported = exported_val != 0;
        entity.collision_mask = entity_mask;
        if (entity_fields == 12 && update_fn_buf[0] != '\0' && strcmp(update_fn_buf, "-") != 0) {
            snprintf(entity.update_fn, sizeof(entity.update_fn), "%s", update_fn_buf);
            remember_update_fn(&loaded->update_fns, entity.update_fn, NULL, 0);
        }
        da_append(&loaded->entities, entity);
        return true;
    }

    return true;
}

static void apply_loaded_level(App *app, LoadedLevel *loaded, Rectangle map_bounds) {
    wall_map_free(&app->map);
    da_free(&app->entities);
    da_free(&app->collision_layers);
    da_free(&app->update_fns);

    app->map = loaded->map;
    app->entities = loaded->entities;
    app->collision_layers = loaded->collision_layers;
    app->update_fns = loaded->update_fns;
    loaded->map = (WallMap){0};
    loaded->entities = (PlacedEntities){0};
    loaded->collision_layers = (CollisionLayers){0};
    loaded->update_fns = (UpdateFns){0};
    loaded->has_size = false;

    app->pending_cols = app->map.cols;
    app->pending_rows = app->map.rows;
    app->floor_asset = loaded->floor_asset;
    app->ceil_asset = loaded->ceil_asset;
    app->player_dir = loaded->player_dir;
    app->player_collision_threshold = loaded->player_collision_threshold < 0.0f ? 0.0f : loaded->player_collision_threshold;
    app->player_collision_mask = loaded->player_collision_mask;
    set_player_pos(app, loaded->player_pos);

    app->brush = BRUSH_WALL;
    app->wall_mode = WALL_POINT;
    app->brush_collision_threshold = DEFAULT_ENTITY_COLLISION_THRESHOLD;
    app->brush_collision_mask = all_custom_collision_layer_bits(&app->collision_layers);
    snprintf(app->brush_threshold_text, sizeof(app->brush_threshold_text), "%.3f", app->brush_collision_threshold);
    app->mask_scroll = 0.0f;
    app->player_mask_scroll = 0.0f;
    app->update_fn_scroll = 0.0f;
    app->asset_scroll = 0.0f;
    app->cols_edit = false;
    app->rows_edit = false;
    app->vdiv_edit = false;
    app->hdiv_edit = false;
    app->vmove_edit = false;
    app->threshold_edit = false;
    app->entity_name_edit = false;
    app->new_mask_edit = false;
    app->update_fn_edit = false;
    app->player_pos_x_edit = false;
    app->player_pos_y_edit = false;
    app->player_dir_x_edit = false;
    app->player_dir_y_edit = false;
    app->player_threshold_edit = false;
    snprintf(app->new_update_fn, sizeof(app->new_update_fn), "update_entity");
    snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "");
    sync_update_fns_from_entities(app);
    clear_edit_selection(app);
    if (!asset_index_is_valid(app, app->selected_asset)) app->selected_asset = app->assets.length > 0 ? 0 : -1;
    fit_camera(app, map_bounds);
}

static bool load_level_header(App *app, Rectangle map_bounds, bool report_status) {
    String file = {0};
    if (!read_file_null_terminated(app->output_path, &file)) {
        if (report_status) set_status(app, "Cannot read %s", app->output_path);
        return false;
    }

    bool ok = false;
    int missing_assets = 0;
    LoadedLevel loaded = {0};
    loaded_level_init(&loaded);

    char *begin = strstr(file.data, MAP_BUILDER_STATE_BEGIN);
    if (!begin) {
        if (report_status) set_status(app, "No saved builder state in %s", app->output_path);
        goto cleanup;
    }
    begin = strchr(begin, '\n');
    if (!begin) {
        if (report_status) set_status(app, "Invalid builder state in %s", app->output_path);
        goto cleanup;
    }
    begin++;

    char *end = strstr(begin, MAP_BUILDER_STATE_END);
    if (!end) {
        if (report_status) set_status(app, "Invalid builder state in %s", app->output_path);
        goto cleanup;
    }
    *end = '\0';

    char *line = begin;
    while (line && *line) {
        char *next = strchr(line, '\n');
        if (next) *next++ = '\0';

        char *payload = state_line_payload(line);
        if (!parse_state_line(app, &loaded, payload, &missing_assets)) {
            if (report_status) set_status(app, "Invalid builder state in %s", app->output_path);
            goto cleanup;
        }

        line = next;
    }

    if (!loaded.has_size) {
        if (report_status) set_status(app, "Missing map size in %s", app->output_path);
        goto cleanup;
    }

    apply_loaded_level(app, &loaded, map_bounds);
    if (report_status) {
        if (missing_assets > 0) set_status(app, "Loaded %s (%d missing assets)", app->output_path, missing_assets);
        else set_status(app, "Loaded %s", app->output_path);
    }
    ok = true;

cleanup:
    loaded_level_free(&loaded);
    da_free(&file);
    return ok;
}

static bool write_level_header(App *app) {
    bool entity_name_was_editing = app->entity_name_edit;
    app->entity_name_edit = false;
    app->update_fn_edit = false;
    if (app->selection_kind == SELECTION_ENTITY && app->selected_entities.length > 1) {
        if (entity_name_was_editing) apply_selected_entities_name(app);
    } else {
        sync_selected_entity_from_editor(app);
    }
    sync_update_fns_from_entities(app);

    String out = {0};

    str_append(&out, "// File generated automatically by map_builder.c. DO NOT EDIT.\n");
    append_map_builder_state(&out, app);
    str_append(&out, "#ifndef YR_LEVEL_H\n");
    str_append(&out, "#define YR_LEVEL_H\n\n");
    str_append(&out, "#include <stdint.h>\n");
    str_append(&out, "#include <stddef.h>\n");
    str_append(&out, "#include <stdbool.h>\n");
    str_append(&out, "#include <yari.h>\n");
    str_append(&out, "#include \"assets.h\"\n\n");
    str_appendf(&out, "#define YR_MAP_COLS %d\n", app->map.cols);
    str_appendf(&out, "#define YR_MAP_ROWS %d\n\n", app->map.rows);

    for (size_t i = 0; i < app->collision_layers.length; i++) {
        CollisionLayer *layer = &app->collision_layers.data[i];
        str_appendf(&out, "#define %s (1u << %u)\n", layer->symbol, layer->shift);
    }
    str_append(&out, "#define YR_CMSK_PLAYER (");
    append_player_collision_mask_expression(&out, app);
    str_append(&out, ")\n\n");

    str_appendf(&out, "#define YR_LEVEL_FLOOR %s\n", asset_symbol_or_null(app, app->floor_asset));
    str_appendf(&out, "#define YR_LEVEL_CEIL  %s\n", asset_symbol_or_null(app, app->ceil_asset));
    str_appendf(&out, "#define YR_PLAYER_COLLISION_THRESHOLD %f\n\n", app->player_collision_threshold);

    {
        Vector2 player_dir_val = normalized_player_dir(app);
        str_append(&out, "static inline YrCamera init_camera_pos(Vector2 pos) {\n");
        str_append(&out, "    return (YrCamera){\n");
        str_append(&out, "        .pos = pos,\n");
        str_append(&out, "        .dir = (Vector2){");
        append_float_literal(&out, player_dir_val.x);
        str_append(&out, ", ");
        append_float_literal(&out, player_dir_val.y);
        str_append(&out, "},\n");
        str_append(&out, "        .horizon = 0.0f,\n");
        str_append(&out, "        .angle = 0.0f\n");
        str_append(&out, "    };\n");
        str_append(&out, "}\n\n");
    }

    str_append(&out, "static inline YrCamera init_camera(void) {\n");
    str_append(&out, "    return init_camera_pos((Vector2){");
    append_float_literal(&out, app->player_pos.x);
    str_append(&out, ", ");
    append_float_literal(&out, app->player_pos.y);
    str_append(&out, "});\n");
    str_append(&out, "}\n\n");

    /* forward declarations for entity update functions (unique names only) */
    for (size_t i = 0; i < app->entities.length; i++) {
        const PlacedEntity *ei = &app->entities.data[i];
        if (ei->update_fn[0] == '\0') continue;
        /* check if already declared */
        bool already = false;
        for (size_t j = 0; j < i; j++) {
            if (strcmp(app->entities.data[j].update_fn, ei->update_fn) == 0) { already = true; break; }
        }
        if (!already) {
            str_appendf(&out, "void %s(YrGameState *state, YrEntity *self, size_t index);\n", ei->update_fn);
        }
    }
    str_append(&out, "\n");

    for (size_t i = 0; i < app->entities.length; i++) {
        PlacedEntity *entity = &app->entities.data[i];
        const char *texture_symbol = "NULL_ASSET";
        if (entity->asset_index >= 0 && entity->asset_index < (int)app->assets.length) {
            texture_symbol = app->assets.data[entity->asset_index].texture_symbol;
        }

        if (entity->update_fn[0] != '\0') {
            str_appendf(&out, "static inline YrEntity create_%s_pos(Vector2 pos, void *data) {\n", entity->name);
        } else {
            str_appendf(&out, "static inline YrEntity create_%s_pos(Vector2 pos, void *data, void (*update)(YrGameState *state, YrEntity *self, size_t index)) {\n", entity->name);

        }
        str_append(&out, "    return (YrEntity){\n");
        str_append(&out, "        .pos = pos,\n");
        str_appendf(&out, "        .texture_id = %s,\n", texture_symbol);
        str_append(&out, "        .vdiv = ");
        append_float_literal(&out, entity->vdiv);
        str_append(&out, ",\n");
        str_append(&out, "        .hdiv = ");
        append_float_literal(&out, entity->hdiv);
        str_append(&out, ",\n");
        str_append(&out, "        .vmove = ");
        append_float_literal(&out, entity->vmove);
        str_append(&out, ",\n");
        str_appendf(&out, "        .disabled = %s,\n", entity->disabled ? "true" : "false");
        str_appendf(&out, "        .entity_data = data,\n");
        str_append(&out, "        .collision_mask = (uint32_t)(");
        append_collision_mask_expression(&out, app, entity->collision_mask);
        str_append(&out, "),\n");
        str_append(&out, "        .collision_threshold = ");
        append_float_literal(&out, entity->collision_threshold);
        str_append(&out, ",\n");
        str_appendf(&out, "        .update = %s,\n", entity->update_fn[0] != '\0' ? entity->update_fn : "update");
        str_append(&out, "    };\n");
        str_append(&out, "}\n\n");

        if (entity->update_fn[0] != '\0') {
            str_appendf(&out, "static inline YrEntity create_%s(void *data) {\n", entity->name);
        } else {
            str_appendf(&out, "static inline YrEntity create_%s(void *data, void (*update)(YrGameState *state, YrEntity *self, size_t index)) {\n", entity->name);
        }
        str_appendf(&out, "    return create_%s_pos((Vector2){", entity->name);
        append_float_literal(&out, entity->pos.x);
        str_append(&out, ", ");
        append_float_literal(&out, entity->pos.y);
        if (entity->update_fn[0] != '\0') {
            str_append(&out, "}, data);\n");
        } else {
            str_append(&out, "}, data, update);\n");
        }
        str_append(&out, "}\n\n");
    }

    str_append(&out, "static inline void level_append_exported_entities(YrEntities *da) {\n");
    for (size_t i = 0; i < app->entities.length; i++) {
        PlacedEntity *entity = &app->entities.data[i];
        if (!entity->exported) continue;
        if(entity->update_fn[0] == '\0') {
            str_appendf(&out, "    yr_da_append(da, create_%s(NULL, NULL));\n", entity->name);
        } else {
            str_appendf(&out, "    yr_da_append(da, create_%s(NULL));\n", entity->name);
        }
    }
    str_append(&out, "}\n\n");

    str_append(&out, "static inline uint8_t *level_get_map(void) {\n");
    str_append(&out, "    static uint8_t data[YR_MAP_ROWS * YR_MAP_COLS] = {\n");
    for (int y = 0; y < app->map.rows; y++) {
        str_append(&out, "        ");
        for (int x = 0; x < app->map.cols; x++) {
            int asset_index = app->map.data[y * app->map.cols + x];
            if (asset_index >= 0 && asset_index < (int)app->assets.length) {
                str_appendf(&out, "%s", app->assets.data[asset_index].texture_symbol);
            } else {
                str_append(&out, "0");
            }
            str_append(&out, (x == app->map.cols - 1) ? "," : ", ");
        }
        str_append(&out, "\n");
    }
    str_append(&out, "    };\n");
    str_append(&out, "    return data;\n");
    str_append(&out, "}\n\n");

    str_append(&out, "static inline void load_level(YrGameState *state) {\n");
    str_append(&out, "    state->assets_map = assets_map;\n");
    str_append(&out, "    state->map = level_get_map();\n");
    str_append(&out, "    state->map_cols = YR_MAP_COLS;\n");
    str_append(&out, "    state->map_rows = YR_MAP_ROWS;\n");
    str_append(&out, "    state->floor_texture = YR_LEVEL_FLOOR;\n");
    str_append(&out, "    state->ceil_texture = YR_LEVEL_CEIL;\n");
    str_append(&out, "    state->camera = init_camera();\n");
    str_append(&out, "    level_append_exported_entities(&state->entities);\n");
    str_append(&out, "}\n\n");

    str_append(&out, "#endif // YR_LEVEL_H\n");

    bool ok = write_entire_file(app->output_path, &out);
    da_free(&out);

    if (ok) {
        set_status(app, "Saved %s", app->output_path);
    } else {
        set_status(app, "Error writing %s", app->output_path);
    }
    return ok;
}

static void place_entity(App *app, Vector2 pos) {
    if (app->selected_asset < 0 || app->selected_asset >= (int)app->assets.length) {
        set_status(app, "Select a texture first");
        return;
    }

    char entity_name[ENTITY_NAME_SIZE] = {0};
    make_unique_entity_name(app, app->brush_entity_name, entity_name, sizeof(entity_name));

    PlacedEntity entity = {
        .pos = pos,
        .asset_index = app->selected_asset,
        .vdiv = app->brush_vdiv,
        .hdiv = app->brush_hdiv,
        .vmove = app->brush_vmove,
        .disabled = app->brush_disabled,
        .exported = app->brush_exported,
        .collision_mask = app->brush_collision_mask,
        .collision_threshold = app->brush_collision_threshold,
    };
    snprintf(entity.name, sizeof(entity.name), "%s", entity_name);
    snprintf(entity.update_fn, sizeof(entity.update_fn), "%s", app->brush_update_fn);
    if (entity.update_fn[0] != '\0') remember_update_fn(&app->update_fns, entity.update_fn, NULL, 0);
    da_append(&app->entities, entity);
    clear_edit_selection(app);
    app->selection_kind = SELECTION_ENTITY;
    app->editing_entity = (int)app->entities.length - 1;
    da_append(&app->selected_entities, app->editing_entity);
}

static void handle_map_input(App *app, Rectangle map_bounds) {
    Vector2 mouse = GetMousePosition();

    int touch_count = GetTouchPointCount();
    if (touch_count >= 2) {
        Vector2 a = GetTouchPosition(0);
        Vector2 b = GetTouchPosition(1);
        Vector2 center = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
        float distance = distance_between(a, b);

        if (CheckCollisionPointRec(center, map_bounds) && distance > 1.0f) {
            if (app->pinching && app->pinch_distance > 1.0f) {
                zoom_at_screen_point(app, center, distance / app->pinch_distance);
            }
            app->pinching = true;
            app->pinch_distance = distance;
            return;
        }
    } else {
        app->pinching = false;
        app->pinch_distance = 0.0f;
    }

    bool mouse_in_map = CheckCollisionPointRec(mouse, map_bounds);
    if ((!mouse_in_map && !app->wall_dragging && !app->selection_dragging && !app->selection_move_dragging) || app_is_editing(app)) return;

    if (app->suppress_left_drag) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return;
        app->suppress_left_drag = false;
    }

    Vector2 wheel = GetMouseWheelMoveV();
    if (wheel.x != 0.0f || wheel.y != 0.0f) {
        bool zoom_modifier = IsKeyDown(KEY_LEFT_CONTROL) ||
            IsKeyDown(KEY_RIGHT_CONTROL) ||
            IsKeyDown(KEY_LEFT_SUPER) ||
            IsKeyDown(KEY_RIGHT_SUPER);

        if (zoom_modifier) {
            float zoom_wheel = wheel.y != 0.0f ? wheel.y : wheel.x;
            zoom_at_screen_point(app, mouse, powf(1.12f, zoom_wheel));
        } else {
            const float scroll_pixels = 48.0f;
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                app->camera.target.x -= wheel.y * scroll_pixels / app->camera.zoom;
            } else {
                app->camera.target.x -= wheel.x * scroll_pixels / app->camera.zoom;
                app->camera.target.y -= wheel.y * scroll_pixels / app->camera.zoom;
            }
        }
        return;
    }

    bool panning = !app->wall_dragging &&
        !app->selection_move_dragging &&
        (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || (IsKeyDown(KEY_SPACE) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)));
    if (panning) {
        Vector2 delta = GetMouseDelta();
        app->camera.target.x -= delta.x / app->camera.zoom;
        app->camera.target.y -= delta.y / app->camera.zoom;
        return;
    }

    Vector2 world = GetScreenToWorld2D(mouse, app->camera);
    if (!mouse_in_map && !app->wall_dragging && !app->selection_dragging && !app->selection_move_dragging) return;
    bool world_in_map = world.x >= 0.0f && world.y >= 0.0f && world.x < (float)app->map.cols && world.y < (float)app->map.rows;
    if (!world_in_map && !app->wall_dragging && !app->selection_dragging && !app->selection_move_dragging) return;

    int cell_x = 0;
    int cell_y = 0;
    world_to_cell_clamped(app, world, &cell_x, &cell_y);
    bool shift = shift_is_down();

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        app->wall_dragging = false;
        app->selection_dragging = false;
        app->selection_move_dragging = false;
        int entity_index = nearest_entity_at(app, world, fmaxf(0.25f, 8.0f / app->camera.zoom));
        if (entity_index >= 0) {
            da_remove_unordered(&app->entities, (size_t)entity_index);
            clear_edit_selection(app);
            return;
        }

        int *cell = wall_map_cell(&app->map, cell_x, cell_y);
        if (cell) *cell = -1;
        clear_edit_selection(app);
        return;
    }

    if (app->selection_move_dragging) {
        update_selection_move_drag(app, world, cell_x, cell_y);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) finish_selection_move_drag(app);
        return;
    }

    if (app->selection_dragging) {
        app->selection_drag_end = world;
        app->selection_drag_end_x = cell_x;
        app->selection_drag_end_y = cell_y;
        if (fabsf(app->selection_drag_end.x - app->selection_drag_start.x) > 0.05f ||
            fabsf(app->selection_drag_end.y - app->selection_drag_start.y) > 0.05f) {
            app->selection_drag_moved = true;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (app->selection_drag_moved) {
                apply_shift_rect_selection(app);
            } else {
                apply_shift_click_selection(app, app->selection_drag_start, app->selection_drag_start_x, app->selection_drag_start_y);
            }
            app->selection_dragging = false;
            app->selection_drag_moved = false;
            prune_invalid_selection(app);
            if (app->selection_kind == SELECTION_ENTITY && app->selected_entities.length > 0) {
                set_status(app, "Selected %u entities", (unsigned int)app->selected_entities.length);
            } else if (app->selection_kind == SELECTION_WALL && app->selected_walls.length > 0) {
                set_status(app, "Selected %u walls", (unsigned int)app->selected_walls.length);
            }
        }
        return;
    }

    if (!shift && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_in_map && selection_move_hit_at(app, world, cell_x, cell_y)) {
        begin_selection_move_drag(app, world, cell_x, cell_y);
        return;
    }

    if (shift && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_in_map) {
        begin_shift_selection_drag(app, world, cell_x, cell_y);
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && select_existing_at(app, world, cell_x, cell_y)) {
        if (app->selection_kind == SELECTION_ENTITY || app->selection_kind == SELECTION_WALL) {
            begin_selection_move_drag(app, world, cell_x, cell_y);
        } else {
            app->suppress_left_drag = true;
        }
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_in_map && selection_has_items(app)) {
        clear_edit_selection(app);
        app->suppress_left_drag = true;
        return;
    }

    if (app->brush == BRUSH_WALL && app->wall_mode != WALL_POINT) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_in_map) {
            clear_edit_selection(app);
            app->wall_dragging = true;
            app->wall_drag_moved = false;
            app->wall_drag_start_x = cell_x;
            app->wall_drag_start_y = cell_y;
            app->wall_drag_end_x = cell_x;
            app->wall_drag_end_y = cell_y;
            return;
        }

        if (app->wall_dragging) {
            app->wall_drag_end_x = cell_x;
            app->wall_drag_end_y = cell_y;
            if (app->wall_drag_end_x != app->wall_drag_start_x || app->wall_drag_end_y != app->wall_drag_start_y) {
                app->wall_drag_moved = true;
            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (!app->wall_drag_moved) {
                    Vector2 click_pos = {(float)app->wall_drag_start_x + 0.5f, (float)app->wall_drag_start_y + 0.5f};
                    int entity_index = nearest_entity_at(app, click_pos, 0.5f);
                    int *cell = wall_map_cell(&app->map, app->wall_drag_start_x, app->wall_drag_start_y);
                    if (entity_index >= 0) {
                        load_entity_into_editor(app, entity_index);
                    } else if (cell && *cell >= 0) {
                        load_wall_into_editor(app, app->wall_drag_start_x, app->wall_drag_start_y);
                    } else {
                        apply_wall_drag(app);
                    }
                } else {
                    apply_wall_drag(app);
                }
                app->wall_dragging = false;
            }
            return;
        }
    }

    if (app->brush == BRUSH_PLAYER && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        clear_edit_selection(app);
        set_player_pos(app, world);
        set_status(app, "Player position %.2f, %.2f", app->player_pos.x, app->player_pos.y);
        app->suppress_left_drag = true;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) clear_edit_selection(app);

    if (app->brush == BRUSH_WALL && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (app->selected_asset < 0 || app->selected_asset >= (int)app->assets.length) {
            set_status(app, "Select a texture first");
            return;
        }

        int *cell = wall_map_cell(&app->map, cell_x, cell_y);
        if (cell) *cell = app->selected_asset;
    } else if (app->brush == BRUSH_ENTITY && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        place_entity(app, world);
    }
}

static void draw_map(App *app, Rectangle map_bounds) {
    DrawRectangleRec(map_bounds, (Color){25, 27, 31, 255});

    BeginScissorMode((int)map_bounds.x, (int)map_bounds.y, (int)map_bounds.width, (int)map_bounds.height);
    BeginMode2D(app->camera);

    DrawRectangleRec((Rectangle){0.0f, 0.0f, (float)app->map.cols, (float)app->map.rows}, (Color){37, 39, 44, 255});

    for (int y = 0; y < app->map.rows; y++) {
        for (int x = 0; x < app->map.cols; x++) {
            int asset_index = app->map.data[y * app->map.cols + x];
            if (asset_index < 0 || asset_index >= (int)app->assets.length) continue;

            draw_texture_preview(&app->assets.data[asset_index], (Rectangle){(float)x, (float)y, 1.0f, 1.0f}, WHITE);
        }
    }

    float line_width = fmaxf(1.0f / app->camera.zoom, 0.01f);
    if (app->camera.zoom >= 4.0f) {
        Color grid = (Color){255, 255, 255, 32};
        for (int x = 0; x <= app->map.cols; x++) {
            DrawLineEx((Vector2){(float)x, 0.0f}, (Vector2){(float)x, (float)app->map.rows}, line_width, grid);
        }
        for (int y = 0; y <= app->map.rows; y++) {
            DrawLineEx((Vector2){0.0f, (float)y}, (Vector2){(float)app->map.cols, (float)y}, line_width, grid);
        }
    }

    DrawRectangleLinesEx((Rectangle){0.0f, 0.0f, (float)app->map.cols, (float)app->map.rows}, line_width * 2.0f, (Color){255, 255, 255, 130});

    if (app->wall_dragging) {
        if (app->wall_mode == WALL_CIRCLE) {
            Vector2 center = {(float)app->wall_drag_start_x + 0.5f, (float)app->wall_drag_start_y + 0.5f};
            Vector2 edge = {(float)app->wall_drag_end_x + 0.5f, (float)app->wall_drag_end_y + 0.5f};
            float radius = distance_between(center, edge);
            DrawCircleLinesV(center, radius, (Color){80, 180, 255, 230});
        } else {
            int min_x = app->wall_drag_start_x < app->wall_drag_end_x ? app->wall_drag_start_x : app->wall_drag_end_x;
            int max_x = app->wall_drag_start_x > app->wall_drag_end_x ? app->wall_drag_start_x : app->wall_drag_end_x;
            int min_y = app->wall_drag_start_y < app->wall_drag_end_y ? app->wall_drag_start_y : app->wall_drag_end_y;
            int max_y = app->wall_drag_start_y > app->wall_drag_end_y ? app->wall_drag_start_y : app->wall_drag_end_y;
            Rectangle preview = {(float)min_x, (float)min_y, (float)(max_x - min_x + 1), (float)(max_y - min_y + 1)};
            DrawRectangleLinesEx(preview, line_width * 4.0f, (Color){80, 180, 255, 230});
        }
    }

    if (app->selection_dragging) {
        Rectangle selection = normalized_world_rect(app->selection_drag_start, app->selection_drag_end);
        if (selection.width < 0.02f) selection.width = 0.02f;
        if (selection.height < 0.02f) selection.height = 0.02f;
        DrawRectangleRec(selection, (Color){80, 180, 255, 35});
        DrawRectangleLinesEx(selection, line_width * 3.0f, (Color){80, 180, 255, 230});
    }

    for (size_t i = 0; i < app->selected_walls.length; i++) {
        SelectedWall wall = app->selected_walls.data[i];
        if (!wall_map_inside(&app->map, wall.x, wall.y)) continue;
        DrawRectangleLinesEx(
            (Rectangle){(float)wall.x, (float)wall.y, 1.0f, 1.0f},
            line_width * 4.0f,
            (Color){80, 180, 255, 255});
    }

    if (app->editing_wall && wall_map_inside(&app->map, app->editing_wall_x, app->editing_wall_y)) {
        DrawRectangleLinesEx(
            (Rectangle){(float)app->editing_wall_x, (float)app->editing_wall_y, 1.0f, 1.0f},
            line_width * 4.0f,
            (Color){80, 180, 255, 255});
    }

    for (size_t i = 0; i < app->entities.length; i++) {
        PlacedEntity *entity = &app->entities.data[i];
        Asset *asset = NULL;
        if (entity->asset_index >= 0 && entity->asset_index < (int)app->assets.length) asset = &app->assets.data[entity->asset_index];

        Rectangle dst = {entity->pos.x - 0.35f, entity->pos.y - 0.35f, 0.7f, 0.7f};
        draw_texture_preview(asset, dst, entity->disabled ? (Color){255, 255, 255, 96} : WHITE);
        Color border = entity->disabled ? (Color){255, 120, 120, 200} : (Color){255, 226, 96, 230};
        float border_width = line_width * 2.0f;
        if (entity_is_selected(app, (int)i) || (int)i == app->editing_entity) {
            border = (Color){80, 180, 255, 255};
            border_width = line_width * 4.0f;
        }
        DrawRectangleLinesEx(dst, border_width, border);
    }

    Vector2 player_dir = normalized_player_dir(app);
    float player_radius = app->player_collision_threshold > 0.02f ? app->player_collision_threshold : 0.02f;
    DrawCircleV(app->player_pos, player_radius, (Color){80, 210, 145, 70});
    DrawCircleLinesV(app->player_pos, player_radius, (Color){80, 210, 145, 255});
    DrawCircleV(app->player_pos, fmaxf(0.05f, 4.0f / app->camera.zoom), (Color){80, 210, 145, 255});
    DrawLineEx(
        app->player_pos,
        (Vector2){app->player_pos.x + player_dir.x * 0.8f, app->player_pos.y + player_dir.y * 0.8f},
        line_width * 4.0f,
        (Color){80, 210, 145, 255});

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, map_bounds) && !app_is_editing(app)) {
        Vector2 world = GetScreenToWorld2D(mouse, app->camera);
        if (world.x >= 0.0f && world.y >= 0.0f && world.x < (float)app->map.cols && world.y < (float)app->map.rows) {
            if (app->brush == BRUSH_WALL) {
                DrawRectangleLinesEx((Rectangle){floorf(world.x), floorf(world.y), 1.0f, 1.0f}, line_width * 3.0f, (Color){80, 180, 255, 230});
            } else if (app->brush == BRUSH_ENTITY) {
                DrawCircleV(world, 0.14f, (Color){80, 180, 255, 230});
                DrawCircleLinesV(world, 0.35f, (Color){80, 180, 255, 230});
            } else if (app->brush == BRUSH_PLAYER) {
                DrawCircleLinesV(world, player_radius, (Color){80, 210, 145, 230});
            }
        }
    }

    EndMode2D();
    EndScissorMode();

    if (CheckCollisionPointRec(mouse, map_bounds)) {
        Vector2 world = GetScreenToWorld2D(mouse, app->camera);
        char coords[96];
        if (app->brush == BRUSH_WALL) {
            snprintf(coords, sizeof(coords), "cell %d,%d", (int)floorf(world.x), (int)floorf(world.y));
        } else if (app->brush == BRUSH_PLAYER) {
            snprintf(coords, sizeof(coords), "player %.2f,%.2f", world.x, world.y);
        } else {
            snprintf(coords, sizeof(coords), "pos %.2f,%.2f", world.x, world.y);
        }
        DrawText(coords, 12, 12, 16, (Color){225, 230, 238, 230});
    }
}

static void draw_asset_list(App *app, Rectangle list_bounds) {
    DrawRectangleRec(list_bounds, (Color){31, 33, 38, 255});
    DrawRectangleLinesEx(list_bounds, 1.0f, (Color){88, 94, 104, 255});

    float content_height = (float)app->assets.length * ASSET_ROW_HEIGHT;
    float min_scroll = list_bounds.height - content_height;
    if (min_scroll > 0.0f) min_scroll = 0.0f;

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, list_bounds)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) app->asset_scroll += wheel * ASSET_ROW_HEIGHT;
    }
    app->asset_scroll = clamp_float(app->asset_scroll, min_scroll, 0.0f);

    BeginScissorMode((int)list_bounds.x, (int)list_bounds.y, (int)list_bounds.width, (int)list_bounds.height);

    for (size_t i = 0; i < app->assets.length; i++) {
        Rectangle row = {
            list_bounds.x + 6.0f,
            list_bounds.y + app->asset_scroll + (float)i * ASSET_ROW_HEIGHT + 4.0f,
            list_bounds.width - 12.0f,
            ASSET_ROW_HEIGHT - 6.0f,
        };
        if (row.y + row.height < list_bounds.y || row.y > list_bounds.y + list_bounds.height) continue;

        bool selected = app->selected_asset == (int)i;
        bool hovered = CheckCollisionPointRec(mouse, row);
        DrawRectangleRec(row, selected ? (Color){49, 83, 115, 255} : hovered ? (Color){45, 48, 55, 255} : (Color){37, 39, 44, 255});
        DrawRectangleLinesEx(row, selected ? 2.0f : 1.0f, selected ? (Color){113, 190, 255, 255} : (Color){69, 74, 84, 255});

        Rectangle thumb = {row.x + 5.0f, row.y + 4.0f, 28.0f, 28.0f};
        DrawRectangleRec(thumb, (Color){20, 22, 26, 255});
        draw_texture_preview(&app->assets.data[i], thumb, WHITE);

        DrawText(app->assets.data[i].name, (int)(row.x + 40.0f), (int)(row.y + 9.0f), 14, (Color){230, 233, 238, 255});

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app->selected_asset = (int)i;
            apply_selected_asset_to_edit(app);
        }
    }

    EndScissorMode();
}

static void set_selected_asset_for_surface(App *app, int *surface_asset, const char *surface_name) {
    if (!asset_index_is_valid(app, app->selected_asset)) {
        set_status(app, "Select a texture first");
        return;
    }

    *surface_asset = app->selected_asset;
    set_status(app, "%s texture = %s", surface_name, asset_symbol_or_null(app, *surface_asset));
}

static void draw_surface_asset_row(App *app, float x, float y, float w, const char *label, int *surface_asset) {
    const Asset *asset = asset_index_is_valid(app, *surface_asset) ? &app->assets.data[*surface_asset] : NULL;
    float label_w = 42.0f;
    float thumb_size = 24.0f;
    float button_w = 58.0f;
    float symbol_x = x + label_w + thumb_size + 10.0f;
    float symbol_w = w - label_w - thumb_size - button_w - 18.0f;

    GuiLabel((Rectangle){x, y, label_w, 26.0f}, label);
    Rectangle thumb = {x + label_w, y + 1.0f, thumb_size, thumb_size};
    DrawRectangleRec(thumb, (Color){20, 22, 26, 255});
    draw_texture_preview(asset, thumb, WHITE);
    DrawRectangleLinesEx(thumb, 1.0f, (Color){88, 94, 104, 255});
    GuiLabel((Rectangle){symbol_x, y, symbol_w, 26.0f}, asset_symbol_or_null(app, *surface_asset));

    if (GuiButton((Rectangle){x + w - button_w, y, button_w, 26.0f}, "Set")) {
        set_selected_asset_for_surface(app, surface_asset, label);
    }
}

static void draw_floor_ceil_section(App *app, float x, float *y, float w) {
    GuiLabel((Rectangle){x, *y, w, 20.0f}, "Floor / ceil");
    *y += 24.0f;

    draw_surface_asset_row(app, x, *y, w, "Floor", &app->floor_asset);
    *y += 30.0f;

    draw_surface_asset_row(app, x, *y, w, "Ceil", &app->ceil_asset);
    *y += 38.0f;
}

static void draw_float_field(const char *label, Rectangle bounds, char *text, float *value, bool *edit) {
    GuiLabel((Rectangle){bounds.x, bounds.y, 58.0f, bounds.height}, label);
    if (GuiValueBoxFloat((Rectangle){bounds.x + 62.0f, bounds.y, bounds.width - 62.0f, bounds.height}, NULL, text, value, *edit)) {
        *edit = !*edit;
    }
}

static void draw_update_fn_picker(App *app, float x, float *y, float w, float max_height) {
    float label_w = 58.0f;
    GuiLabel((Rectangle){x, *y, label_w, 20.0f}, "Update");
    float add_button_w = 64.0f;
    if (GuiTextBox((Rectangle){x + label_w + 4.0f, *y, w - label_w - add_button_w - 28.0f , 26.0f}, app->new_update_fn, (int)sizeof(app->new_update_fn), app->update_fn_edit)) {
        app->update_fn_edit = !app->update_fn_edit;
    }
    if (GuiButton((Rectangle){x + w - add_button_w, *y, add_button_w, 26.0f}, "Add")) {
        if (!text_is_empty(app->new_update_fn)) add_update_fn(app, app->new_update_fn);
    }
    *y += 34.0f;

    Rectangle list = {x, *y, w, max_height};
    DrawRectangleRec(list, (Color){31, 33, 38, 255});
    DrawRectangleLinesEx(list, 1.0f, (Color){88, 94, 104, 255});

    float row_h = 24.0f;
    float content_h = ((float)app->update_fns.length + 1.0f) * row_h;
    float min_scroll = list.height - content_h;
    if (min_scroll > 0.0f) min_scroll = 0.0f;

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, list)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) app->update_fn_scroll += wheel * row_h;
    }
    app->update_fn_scroll = clamp_float(app->update_fn_scroll, min_scroll, 0.0f);

    size_t delete_index = (size_t)-1;
    float delete_button_w = 42.0f;

    BeginScissorMode((int)list.x, (int)list.y, (int)list.width, (int)list.height);

    Rectangle default_row = {list.x + 8.0f, list.y + app->update_fn_scroll + 3.0f, list.width - 16.0f, 18.0f};
    if (default_row.y + default_row.height >= list.y && default_row.y <= list.y + list.height) {
        bool checked = app->brush_update_fn[0] == '\0';
        bool before = checked;
        GuiCheckBox((Rectangle){default_row.x, default_row.y, 18.0f, 18.0f}, NULL, &checked);
        GuiLabel((Rectangle){default_row.x + 26.0f, default_row.y, default_row.width - 26.0f, default_row.height}, "none");
        if (checked != before) snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "");
    }

    for (size_t i = 0; i < app->update_fns.length; i++) {
        const UpdateFn *update_fn = &app->update_fns.data[i];
        Rectangle row = {list.x + 8.0f, list.y + app->update_fn_scroll + ((float)i + 1.0f) * row_h + 3.0f, list.width - 16.0f, 18.0f};
        if (row.y + row.height < list.y || row.y > list.y + list.height) continue;

        bool checked = strcmp(app->brush_update_fn, update_fn->name) == 0;
        bool before = checked;
        GuiCheckBox((Rectangle){row.x, row.y, 18.0f, 18.0f}, NULL, &checked);
        GuiLabel((Rectangle){row.x + 26.0f, row.y, row.width - delete_button_w - 34.0f, row.height}, update_fn->name);
        if (GuiButton((Rectangle){row.x + row.width - delete_button_w, row.y, delete_button_w, row.height}, "Del")) {
            delete_index = i;
        }
        if (checked != before) {
            if (checked) snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "%s", update_fn->name);
            else snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "");
        }
    }

    EndScissorMode();

    if (delete_index != (size_t)-1) {
        remove_update_fn_at(app, delete_index);
    }

    *y += max_height + 10.0f;
}

static void draw_collision_picker(App *app, float x, float *y, float w, float max_height, const char *title, uint32_t *mask, float *scroll, bool allow_add) {
    if (allow_add) {
        float label_w = 58.0f;
        float add_button_w = 64.0f;
        GuiLabel((Rectangle){x, *y, label_w, 20.0f}, title);
        if (GuiTextBox((Rectangle){x + label_w + 4.0f, *y, w - label_w - add_button_w - 28.0f, 26.0f}, app->new_collision_layer, (int)sizeof(app->new_collision_layer), app->new_mask_edit)) {
            app->new_mask_edit = !app->new_mask_edit;
        }
        if (GuiButton((Rectangle){x + w - add_button_w, *y, add_button_w, 26.0f}, "Add")) {
            if (!text_is_empty(app->new_collision_layer)) add_collision_layer(app, app->new_collision_layer);
        }
        *y += 34.0f;
    } else {
        GuiLabel((Rectangle){x, *y, w, 20.0f}, title);
        *y += 24.0f;
    }

    Rectangle list = {x, *y, w, max_height};
    DrawRectangleRec(list, (Color){31, 33, 38, 255});
    DrawRectangleLinesEx(list, 1.0f, (Color){88, 94, 104, 255});

    float row_h = 24.0f;
    float content_h = (float)app->collision_layers.length * row_h;
    float min_scroll = list.height - content_h;
    if (min_scroll > 0.0f) min_scroll = 0.0f;

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, list)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) *scroll += wheel * row_h;
    }
    *scroll = clamp_float(*scroll, min_scroll, 0.0f);

    size_t delete_index = (size_t)-1;
    float delete_button_w = 42.0f;

    BeginScissorMode((int)list.x, (int)list.y, (int)list.width, (int)list.height);
    for (size_t i = 0; i < app->collision_layers.length; i++) {
        CollisionLayer *layer = &app->collision_layers.data[i];
        Rectangle row = {list.x + 8.0f, list.y + *scroll + (float)i * row_h + 3.0f, list.width - 16.0f, 18.0f};
        if (row.y + row.height < list.y || row.y > list.y + list.height) continue;

        bool checked = (*mask & layer->value) != 0;
        bool before = checked;
        char label[96];
        snprintf(label, sizeof(label), "%s  1<<%u", layer->symbol, layer->shift);
        GuiCheckBox((Rectangle){row.x, row.y, 18.0f, 18.0f}, NULL, &checked);
        GuiLabel((Rectangle){row.x + 26.0f, row.y, row.width - delete_button_w - 34.0f, row.height}, label);
        if (GuiButton((Rectangle){row.x + row.width - delete_button_w, row.y, delete_button_w, row.height}, "Del")) {
            delete_index = i;
        }
        if (checked != before) {
            if (checked) *mask |= layer->value;
            else *mask &= ~layer->value;
        }
    }

    if (app->collision_layers.length == 0) {
        GuiLabel((Rectangle){list.x + 8.0f, list.y + 8.0f, list.width - 16.0f, 20.0f}, "No custom layers");
    }
    EndScissorMode();

    if (delete_index != (size_t)-1) {
        remove_collision_layer_at(app, delete_index);
    }

    *y += max_height + 10.0f;
}

static void apply_pending_map_size(App *app, Rectangle map_bounds) {
    app->pending_cols = app->pending_cols < MIN_MAP_SIZE ? MIN_MAP_SIZE : app->pending_cols;
    app->pending_rows = app->pending_rows < MIN_MAP_SIZE ? MIN_MAP_SIZE : app->pending_rows;
    app->pending_cols = app->pending_cols > MAX_MAP_SIZE ? MAX_MAP_SIZE : app->pending_cols;
    app->pending_rows = app->pending_rows > MAX_MAP_SIZE ? MAX_MAP_SIZE : app->pending_rows;
    if (app->pending_cols == app->map.cols && app->pending_rows == app->map.rows) return;

    wall_map_resize(app, app->pending_cols, app->pending_rows);
    clear_edit_selection(app);
    refresh_player_text(app);
    fit_camera(app, map_bounds);
    set_status(app, "Map resized to %dx%d", app->map.cols, app->map.rows);
}

static void draw_sidebar(App *app, Rectangle sidebar_bounds, Rectangle map_bounds) {
    GuiPanel(sidebar_bounds, "Map Builder");

    float x = sidebar_bounds.x + 16.0f;
    float y = 34.0f;
    float w = sidebar_bounds.width - 32.0f;
    float gap = 8.0f;
    float row_h = 26.0f;

    GuiLabel((Rectangle){x, y, w, 20.0f}, "Size");
    y += 24.0f;
    float half_w = (w - gap) * 0.5f;
    GuiLabel((Rectangle){x, y, 38.0f, row_h}, "Cols");
    bool cols_was_editing = app->cols_edit;
    if (GuiValueBox((Rectangle){x + 42.0f, y, half_w - 42.0f, row_h}, NULL, &app->pending_cols, MIN_MAP_SIZE, MAX_MAP_SIZE, app->cols_edit)) {
        app->cols_edit = !app->cols_edit;
        if (cols_was_editing && !app->cols_edit) apply_pending_map_size(app, map_bounds);
    }
    GuiLabel((Rectangle){x + half_w + gap, y, 38.0f, row_h}, "Rows");
    bool rows_was_editing = app->rows_edit;
    if (GuiValueBox((Rectangle){x + half_w + gap + 42.0f, y, half_w - 42.0f, row_h}, NULL, &app->pending_rows, MIN_MAP_SIZE, MAX_MAP_SIZE, app->rows_edit)) {
        app->rows_edit = !app->rows_edit;
        if (rows_was_editing && !app->rows_edit) apply_pending_map_size(app, map_bounds);
    }
    y += 34.0f;

    float button_w = (w - gap * 2.0f) / 3.0f;
    if (GuiButton((Rectangle){x, y, button_w, 28.0f}, "Load")) load_level_header(app, map_bounds, true);
    if (GuiButton((Rectangle){x + button_w + gap, y, button_w, 28.0f}, "Fit")) fit_camera(app, map_bounds);
    if (GuiButton((Rectangle){x + (button_w + gap) * 2.0f, y, button_w, 28.0f}, "Save")) write_level_header(app);
    y += 42.0f;

    float mode_w = (w - gap) * 0.5f;
    Rectangle wall_toggle = {x, y, mode_w, 26.0f};
    Rectangle entity_toggle = {x + mode_w + gap, y, mode_w, 26.0f};
    Rectangle player_toggle = {x, y + 30.0f, mode_w, 26.0f};
    Rectangle surface_toggle = {x + mode_w + gap, y + 30.0f, mode_w, 26.0f};
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse, wall_toggle) && app->brush != BRUSH_WALL) {
            clear_edit_selection(app);
            app->brush = BRUSH_WALL;
        } else if (CheckCollisionPointRec(mouse, entity_toggle) && app->brush != BRUSH_ENTITY) {
            clear_edit_selection(app);
            app->brush = BRUSH_ENTITY;
        } else if (CheckCollisionPointRec(mouse, player_toggle) && app->brush != BRUSH_PLAYER) {
            clear_edit_selection(app);
            app->brush = BRUSH_PLAYER;
        } else if (CheckCollisionPointRec(mouse, surface_toggle) && app->brush != BRUSH_SURFACE) {
            clear_edit_selection(app);
            app->brush = BRUSH_SURFACE;
        }
    }

    bool wall_active = app->brush == BRUSH_WALL;
    bool entity_active = app->brush == BRUSH_ENTITY;
    bool player_active = app->brush == BRUSH_PLAYER;
    bool surface_active = app->brush == BRUSH_SURFACE;
    GuiLock();
    GuiToggle(wall_toggle, "Wall", &wall_active);
    GuiToggle(entity_toggle, "Entity", &entity_active);
    GuiToggle(player_toggle, "Player", &player_active);
    GuiToggle(surface_toggle, "Floor/Ceil", &surface_active);
    GuiUnlock();
    y += 66.0f;

    bool uses_textures = app->brush == BRUSH_WALL || app->brush == BRUSH_ENTITY || app->brush == BRUSH_SURFACE;

    if (app->brush == BRUSH_WALL) {
        GuiLabel((Rectangle){x, y, w, 20.0f}, "Wall insert");
        y += 24.0f;

        float insert_w = (w - gap * 2.0f) / 3.0f;
        Rectangle point_toggle = {x, y, insert_w, 24.0f};
        Rectangle rect_toggle = {x + insert_w + gap, y, insert_w, 24.0f};
        Rectangle circle_toggle = {x + (insert_w + gap) * 2.0f, y, insert_w, 24.0f};
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, point_toggle)) app->wall_mode = WALL_POINT;
            else if (CheckCollisionPointRec(mouse, rect_toggle)) app->wall_mode = WALL_RECT;
            else if (CheckCollisionPointRec(mouse, circle_toggle)) app->wall_mode = WALL_CIRCLE;
        }

        bool point_active = app->wall_mode == WALL_POINT;
        bool rect_active = app->wall_mode == WALL_RECT;
        bool circle_active = app->wall_mode == WALL_CIRCLE;
        GuiLock();
        GuiToggle(point_toggle, "Point", &point_active);
        GuiToggle(rect_toggle, "Rect", &rect_active);
        GuiToggle(circle_toggle, "Circle", &circle_active);
        GuiUnlock();
        y += 34.0f;
    } else if (app->brush == BRUSH_ENTITY) {
        bool multi_entity_edit = app->selection_kind == SELECTION_ENTITY && app->selected_entities.length > 1;
        bool name_was_editing = app->entity_name_edit;
        float vdiv_before = app->brush_vdiv;
        float hdiv_before = app->brush_hdiv;
        float vmove_before = app->brush_vmove;
        float threshold_before = app->brush_collision_threshold;
        uint32_t mask_before = app->brush_collision_mask;
        bool disabled_before = app->brush_disabled;
        bool exported_before = app->brush_exported;
        char update_fn_before[ENTITY_NAME_SIZE] = {0};
        snprintf(update_fn_before, sizeof(update_fn_before), "%s", app->brush_update_fn);

        GuiLabel((Rectangle){x, y, 52.0f, row_h}, "Name");
        if (GuiTextBox((Rectangle){x + 62.0f, y, w - 62.0f, row_h}, app->brush_entity_name, (int)sizeof(app->brush_entity_name), app->entity_name_edit)) app->entity_name_edit = !app->entity_name_edit;
        if (multi_entity_edit && name_was_editing && !app->entity_name_edit) apply_selected_entities_name(app);
        y += 32.0f;

        draw_update_fn_picker(app, x, &y, w, 80.0f);
        if (multi_entity_edit && strcmp(update_fn_before, app->brush_update_fn) != 0) apply_selected_entities_update_fn(app);

        draw_float_field("vdiv", (Rectangle){x, y, half_w, row_h}, app->brush_vdiv_text, &app->brush_vdiv, &app->vdiv_edit);
        draw_float_field("hdiv", (Rectangle){x + half_w + gap, y, half_w, row_h}, app->brush_hdiv_text, &app->brush_hdiv, &app->hdiv_edit);
        if (multi_entity_edit && app->brush_vdiv != vdiv_before) apply_selected_entities_vdiv(app);
        if (multi_entity_edit && app->brush_hdiv != hdiv_before) apply_selected_entities_hdiv(app);
        y += 32.0f;

        draw_float_field("vmove", (Rectangle){x, y, half_w, row_h}, app->brush_vmove_text, &app->brush_vmove, &app->vmove_edit);
        draw_float_field("radius", (Rectangle){x + half_w + gap, y, half_w, row_h}, app->brush_threshold_text, &app->brush_collision_threshold, &app->threshold_edit);
        if (multi_entity_edit && app->brush_vmove != vmove_before) apply_selected_entities_vmove(app);
        if (multi_entity_edit && app->brush_collision_threshold != threshold_before) apply_selected_entities_threshold(app);
        y += 34.0f;

        draw_collision_picker(app, x, &y, w, 80.0f, "YR_CMSK", &app->brush_collision_mask, &app->mask_scroll, true);
        if (multi_entity_edit && app->brush_collision_mask != mask_before) apply_selected_entities_collision_mask(app);

        GuiCheckBox((Rectangle){x, y + 2.0f, 18.0f, 18.0f}, "disabled", &app->brush_disabled);
        GuiCheckBox((Rectangle){x + half_w + gap, y + 2.0f, 18.0f, 18.0f}, "exported", &app->brush_exported);
        if (multi_entity_edit && app->brush_disabled != disabled_before) apply_selected_entities_disabled(app);
        if (multi_entity_edit && app->brush_exported != exported_before) apply_selected_entities_exported(app);
        y += 30.0f;

        if (!multi_entity_edit) sync_selected_entity_from_editor(app);
    } else if (app->brush == BRUSH_PLAYER) {
        GuiLabel((Rectangle){x, y, w, 20.0f}, "Player");
        y += 24.0f;

        draw_float_field("pos x", (Rectangle){x, y, half_w, row_h}, app->player_pos_x_text, &app->player_pos.x, &app->player_pos_x_edit);
        draw_float_field("pos y", (Rectangle){x + half_w + gap, y, half_w, row_h}, app->player_pos_y_text, &app->player_pos.y, &app->player_pos_y_edit);
        y += 34.0f;

        draw_float_field("dir x", (Rectangle){x, y, half_w, row_h}, app->player_dir_x_text, &app->player_dir.x, &app->player_dir_x_edit);
        draw_float_field("dir y", (Rectangle){x + half_w + gap, y, half_w, row_h}, app->player_dir_y_text, &app->player_dir.y, &app->player_dir_y_edit);
        y += 34.0f;

        draw_float_field("radius", (Rectangle){x, y, w, row_h}, app->player_threshold_text, &app->player_collision_threshold, &app->player_threshold_edit);
        if (!app->player_pos_x_edit) {
            app->player_pos.x = clamp_float(app->player_pos.x, 0.0f, (float)app->map.cols - 0.001f);
            snprintf(app->player_pos_x_text, sizeof(app->player_pos_x_text), "%.3f", app->player_pos.x);
        }
        if (!app->player_pos_y_edit) {
            app->player_pos.y = clamp_float(app->player_pos.y, 0.0f, (float)app->map.rows - 0.001f);
            snprintf(app->player_pos_y_text, sizeof(app->player_pos_y_text), "%.3f", app->player_pos.y);
        }
        if (!app->player_threshold_edit && app->player_collision_threshold < 0.0f) {
            app->player_collision_threshold = 0.0f;
            snprintf(app->player_threshold_text, sizeof(app->player_threshold_text), "%.3f", app->player_collision_threshold);
        }
        y += 34.0f;

        draw_collision_picker(app, x, &y, w, 96.0f, "Player collision", &app->player_collision_mask, &app->player_mask_scroll, false);
    } else if (app->brush == BRUSH_SURFACE) {
        draw_floor_ceil_section(app, x, &y, w);
    }

    float status_height = 34.0f;
    if (uses_textures) {
        GuiLabel((Rectangle){x, y, w, 20.0f}, "Textures");
        y += 24.0f;

        Rectangle list_bounds = {x, y, w, sidebar_bounds.height - y - status_height - 12.0f};
        if (list_bounds.height < 80.0f) list_bounds.height = 80.0f;
        draw_asset_list(app, list_bounds);
    }

    if (GetTime() <= app->status_until && app->status[0] != '\0') {
        GuiStatusBar((Rectangle){x, sidebar_bounds.height - status_height, w, 24.0f}, app->status);
    } else {
        char output[256];
        snprintf(output, sizeof(output), "%s", app->output_path);
        GuiStatusBar((Rectangle){x, sidebar_bounds.height - status_height, w, 24.0f}, output);
    }
}

static void init_app(App *app, const char *asset_dir, const char *output_path) {
    app->asset_dir = asset_dir;
    app->output_path = output_path;
    app->selected_asset = -1;
    app->floor_asset = -1;
    app->ceil_asset = -1;
    app->editing_entity = NO_SELECTION;
    app->editing_wall = false;
    app->editing_wall_x = NO_SELECTION;
    app->editing_wall_y = NO_SELECTION;
    app->selection_kind = SELECTION_NONE;
    app->clipboard_kind = SELECTION_NONE;
    app->clipboard_wall_width = 0;
    app->clipboard_wall_height = 0;
    app->selection_dragging = false;
    app->selection_drag_moved = false;
    app->selection_drag_start_x = NO_SELECTION;
    app->selection_drag_start_y = NO_SELECTION;
    app->selection_drag_end_x = NO_SELECTION;
    app->selection_drag_end_y = NO_SELECTION;
    app->selection_move_dragging = false;
    app->selection_move_moved = false;
    app->selection_move_start_x = NO_SELECTION;
    app->selection_move_start_y = NO_SELECTION;
    app->selection_move_last_x = NO_SELECTION;
    app->selection_move_last_y = NO_SELECTION;
    app->brush = BRUSH_WALL;
    app->wall_mode = WALL_POINT;
    app->wall_dragging = false;
    app->wall_drag_start_x = NO_SELECTION;
    app->wall_drag_start_y = NO_SELECTION;
    app->wall_drag_end_x = NO_SELECTION;
    app->wall_drag_end_y = NO_SELECTION;
    app->pending_cols = DEFAULT_MAP_COLS;
    app->pending_rows = DEFAULT_MAP_ROWS;
    app->brush_vdiv = 0.0f;
    app->brush_hdiv = 0.0f;
    app->brush_vmove = 0.0f;
    app->brush_collision_threshold = DEFAULT_ENTITY_COLLISION_THRESHOLD;
    app->brush_collision_mask = 0;
    snprintf(app->brush_vdiv_text, sizeof(app->brush_vdiv_text), "0.0");
    snprintf(app->brush_hdiv_text, sizeof(app->brush_hdiv_text), "0.0");
    snprintf(app->brush_vmove_text, sizeof(app->brush_vmove_text), "0.0");
    snprintf(app->brush_threshold_text, sizeof(app->brush_threshold_text), "%.3f", app->brush_collision_threshold);
    snprintf(app->brush_entity_name, sizeof(app->brush_entity_name), "entity");
    snprintf(app->new_collision_layer, sizeof(app->new_collision_layer), "ENTITY");
    snprintf(app->new_update_fn, sizeof(app->new_update_fn), "update_entity");
    snprintf(app->brush_update_fn, sizeof(app->brush_update_fn), "");
    app->brush_exported = true;
    app->player_pos = (Vector2){DEFAULT_MAP_COLS * 0.5f, DEFAULT_MAP_ROWS * 0.5f};
    app->player_dir = (Vector2){0.0f, 1.0f};
    app->player_collision_threshold = 0.15f;
    app->player_collision_mask = UINT32_MAX;
    refresh_player_text(app);

    wall_map_init(&app->map, DEFAULT_MAP_COLS, DEFAULT_MAP_ROWS);
    scan_assets(&app->assets, asset_dir);
}

int main(int argc, char **argv) {
    if (argc > 3) {
        fprintf(stderr, "Usage: %s [assets_dir] [output_file]\n", argv[0]);
        return 1;
    }

    const char *asset_dir = argc > 1 ? argv[1] : "assets";
    const char *output_path = argc > 2 ? argv[2] : "level.h";

    App app = {0};
    init_app(&app, asset_dir, output_path);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "map_builder");
    SetTargetFPS(60);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);

    load_asset_textures(&app.assets);
    filter_assets_by_size(&app.assets, 64, 64);
    if (app.assets.length > 0) app.selected_asset = 0;
    else set_status(&app, "No image assets found in %s", asset_dir);

    Rectangle map_bounds = get_map_bounds();
    fit_camera(&app, map_bounds);
    load_level_header(&app, map_bounds, false);

    while (!WindowShouldClose()) {
        map_bounds = get_map_bounds();
        Rectangle sidebar_bounds = get_sidebar_bounds();
        update_camera_offset(&app, map_bounds);

        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
        if (ctrl && IsKeyPressed(KEY_S)) write_level_header(&app);
        if (!app_is_editing(&app)) {
            if (ctrl && IsKeyPressed(KEY_C)) copy_active_selection(&app);
            if (ctrl && IsKeyPressed(KEY_V)) paste_clipboard_at_mouse(&app, map_bounds);
            if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) delete_active_selection(&app);
        }

        handle_map_input(&app, map_bounds);

        BeginDrawing();
        ClearBackground((Color){25, 27, 31, 255});
        draw_map(&app, map_bounds);
        draw_sidebar(&app, sidebar_bounds, map_bounds);
        EndDrawing();
    }

    free_assets(&app.assets);
    wall_map_free(&app.map);
    da_free(&app.entities);
    da_free(&app.selected_entities);
    da_free(&app.selected_walls);
    da_free(&app.clipboard_entities);
    da_free(&app.clipboard_walls);
    da_free(&app.collision_layers);
    da_free(&app.update_fns);
    CloseWindow();
    tmp_free();
    return 0;
}
