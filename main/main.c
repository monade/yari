#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "raylib.h"
#define RAYMATH_STATIC_INLINE
#include "raymath.h"
#include "assets.h"

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define SetPixel(x, y, color) DrawRectangle(x, y, 1, 1, color)

#ifdef ESP32
#define TARGET_FPS 30
#define SCREEN_W LCD_W
#define SCREEN_H LCD_H
#define RAY_RES 2
#define EMPTY_PIXEL 0
#else
#define TARGET_FPS 60
#define SCREEN_W 800
#define SCREEN_H 600
#define RAY_RES 2
#define EMPTY_PIXEL 0xFF
#endif
#define COLS 10
#define ROWS 10
#define ASPECT_RATIO ((float)SCREEN_W / SCREEN_H)
#define MINIMAP_CELL_SCALE 20
#define FOV_ANGLE (PI / 3.5)
#define MAX_RENDER_DIST 20.0
#define TEXTURE_SIZE 64

#define PLAYER_ROTATION_SPEED 1.25
#define PLAYER_SPEED 2.5

#define POINT_R 2.5
#define LINE_THICKNESS 1.5

typedef struct {
    Vector2 pos;
    Vector2 dir;
} Player;

typedef struct Sprite {
    Vector2 pos;
    int texture_id;
    float dist;
    float vdiv;
    float hdiv;
    float vmove;
    void (*update)(struct Sprite *self);
} Sprite;

// 0 null, 1-127 texture_id, 128-255 color_id
static uint8_t map[ROWS][COLS] = {0};

// pixel_t assets_map from assets.h

Color color_map[] = {
    RED,    // 128
    GREEN,  // 129
    BLUE,   // 130
    YELLOW, // 131
    PURPLE, // 132
    ORANGE, // 133
    WHITE   // 134
};

float zbuffer[SCREEN_W/RAY_RES];

void test_sprite_script(Sprite *self) {
    float incX = sinf(GetTime());
    float incY = cosf(GetTime());
    self->pos.x += incX * 0.01;
    self->pos.y += incY * 0.01;
    if(fabs(incX) > 0.75 || fabs(incY) > 0.75) self->texture_id = tx_greenlight2;
    else self->texture_id = tx_greenlight;
}

Sprite sprites[] = {
    {.pos = {6.5, 4.5}, .texture_id = tx_pillar, .vmove=-0.7},
    {.pos = {5.5, 5.5}, .texture_id = tx_greenlight, .update = test_sprite_script},
    {.pos = {2.5, 3.5}, .texture_id = tx_barrel},
    {.pos = {3.5, 3.5}, .texture_id = tx_barrel, .vmove=0.25},
};
#define NUM_SPRITES ARRAY_LEN(sprites)

static int compare_sprite_dist(const void *a, const void *b) {
    const Sprite *sa = (const Sprite *)a;
    const Sprite *sb = (const Sprite *)b;
    if (sa->dist < sb->dist) return 1;
    if (sa->dist > sb->dist) return -1;
    return 0;
}

void init_game() {
    for (int i = 0; i < ROWS; i++) {
        map[i][0] = tx_bricks;
        map[i][COLS - 1] = tx_bricks;
    }
    for (int j = 0; j < COLS; j++) {
        map[0][j] = tx_bricks;
        map[ROWS - 1][j] = tx_bricks;
    }
    map[1][3] = tx_bricks;
    map[1][4] = 131;
    map[1][5] = 129;
    map[2][5] = 133;
    map[3][4] = 129;
    map[3][5] = tx_bricks;

    map[7][7] = 130;
    map[8][8] = 129;
    map[9][9] = 134;
}

void draw_minimap() {
    DrawRectangle(0, 0, COLS * MINIMAP_CELL_SCALE, ROWS * MINIMAP_CELL_SCALE, GetColor(0x00000046));
    DrawRectangleLines(0, 0, COLS * MINIMAP_CELL_SCALE, ROWS * MINIMAP_CELL_SCALE, RAYWHITE);
    for (int i = 1; i < COLS; i++) {
        int x = i * MINIMAP_CELL_SCALE;
        DrawLine(x, 0, x, ROWS * MINIMAP_CELL_SCALE, RAYWHITE);
    }
    for (int i = 1; i < ROWS; i++) {
        int y = i * MINIMAP_CELL_SCALE;
        DrawLine(0, y, COLS * MINIMAP_CELL_SCALE, y, RAYWHITE);
    }
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int ci = i*MINIMAP_CELL_SCALE;
            int cj = j*MINIMAP_CELL_SCALE;
            uint8_t c = map[i][j];
            if (c == 0) continue;
            if (c >= 128) {
                // color
                DrawRectangle(cj, ci, MINIMAP_CELL_SCALE, MINIMAP_CELL_SCALE, color_map[c - 128]);
            } else {
                // texture
                DrawRectangle(cj, ci, MINIMAP_CELL_SCALE, MINIMAP_CELL_SCALE, MAGENTA);
                DrawLine(cj, ci, cj + MINIMAP_CELL_SCALE, ci + MINIMAP_CELL_SCALE, BLACK);
                DrawLine(cj + MINIMAP_CELL_SCALE, ci, cj, ci + MINIMAP_CELL_SCALE, BLACK);
            }
        }
    }
}

void draw_minimap_player(Vector2 p) {
    DrawCircleV(Vector2Scale(p, MINIMAP_CELL_SCALE), POINT_R * 2.0, GREEN);
}

void raycast_walls(const Player *p, Vector2 dir, int slice_x) {
    if (dir.x == 0.0) dir.x = EPSILON;
    if (dir.y == 0.0) dir.y = EPSILON;
    Vector2 rs = Vector2Add(p->pos, Vector2Scale(dir, EPSILON));
    while (Vector2Length(Vector2Subtract(rs, p->pos)) <= MAX_RENDER_DIST) {
        Vector2 cell = {.x = floorf(rs.x), .y = floorf(rs.y)};
        if (rs.x > 0.0 && rs.x < COLS && rs.y > 0.0 && rs.y < ROWS) {
            uint8_t map_cell = map[(int)cell.y][(int)cell.x];
            if (map_cell) {
                // draw slice
                float dist = Vector2DotProduct(Vector2Subtract(rs, p->pos), p->dir);
                zbuffer[slice_x / RAY_RES] = dist;
                int h = SCREEN_W / dist;
                float bright_factor = 1.0 / dist - 0.9;
                if (bright_factor >= 0.0) bright_factor = 0.0;

                if (map_cell >= 128) {
                    // color
                    Color c = ColorBrightness(color_map[map_cell - 128], bright_factor);
                    DrawRectangle(slice_x, (SCREEN_H - h) / 2.0, RAY_RES, h, c);
                } else {
                    const pixel_t *tex = assets_map[map_cell];
                    int texture_x;
                    float diff_x = rs.x - cell.x;
                    float diff_y = rs.y - cell.y;


                    if (diff_x > EPSILON && diff_x < 1.0 - EPSILON) {
                        texture_x = diff_x * TEXTURE_SIZE;
                    } else {
                        texture_x = TEXTURE_SIZE - (diff_y * TEXTURE_SIZE);
                    }
                    int hmax = h;
                    if(hmax > SCREEN_H) hmax = SCREEN_H;
                    for (int y = 0; y < hmax; y+=RAY_RES) {
                        int overflow_screen = (h-hmax)/2;
                        int texture_y = ((overflow_screen+y) * TEXTURE_SIZE) / h;
                        pixel_t texel = tex[texture_y * TEXTURE_SIZE + texture_x];
                        Color texel_color = GetColor(texel);
                        Color c = ColorBrightness(texel_color, bright_factor);
                        DrawRectangle(slice_x, (SCREEN_H - hmax) / 2 + y, RAY_RES, RAY_RES, c);
                    }
                }
                return;
            }
        }
        float distX = cell.x + (dir.x >= 0 ? 1.0 : -EPSILON) - rs.x;
        float distY = cell.y + (dir.y >= 0 ? 1.0 : -EPSILON) - rs.y;
        Vector2 inc;
        if (fabs(distX / dir.x) < fabs(distY / dir.y)) {
            inc = (Vector2){.x = distX, .y = distX * dir.y / dir.x};
        } else {
            inc = (Vector2){.x = distY * dir.x / dir.y, .y = distY};
        }
        Vector2 new_rs = Vector2Add(rs, inc);
        #ifdef DEBUG
        // draw raycast on minimap
        if (new_rs.x > -1.0 && new_rs.x <= COLS && new_rs.y > -1.0 && new_rs.y <= ROWS && rs.x > -1.0 && rs.x <= COLS && rs.y > -1.0 && rs.y <= ROWS) {
            DrawLineEx(Vector2Scale(rs, MINIMAP_CELL_SCALE), Vector2Scale(new_rs, MINIMAP_CELL_SCALE), LINE_THICKNESS, BLUE);
            // DrawCircleV(Vector2Scale(rs, MINIMAP_CELL_SCALE), POINT_R, RED);
        }
        #endif
        rs = new_rs;
    }
    zbuffer[slice_x / RAY_RES] = MAX_RENDER_DIST;
}

bool check_for_obstacle(Vector2 pos, Vector2 dir, float threshold)
{
  Vector2 rs = Vector2Add(pos, Vector2Scale(dir, EPSILON));
  Vector2 cell = {.x = floorf(rs.x), .y = floorf(rs.y)};

  float distX = cell.x + (dir.x >= 0 ? 1.0 : -EPSILON) - rs.x;
  float distY = cell.y + (dir.y >= 0 ? 1.0 : -EPSILON) - rs.y;

  Vector2 inc;
  if (fabs(distX / dir.x) < fabs(distY / dir.y)) {
      inc = (Vector2){.x = distX, .y = distX * dir.y / dir.x};
  } else {
      inc = (Vector2){.x = distY * dir.x / dir.y, .y = distY};
  }

  Vector2 next_cell = Vector2Add(inc, rs);
  uint8_t map_cell = map[(int)next_cell.y][(int)next_cell.x];

  return map_cell && Vector2LengthSqr(inc) <= threshold * threshold;
}

void move_player(Player *p) {
    float dt = GetFrameTime();
    if (IsKeyDown(KEY_A)) {
        p->dir = Vector2Rotate(p->dir, -dt * PLAYER_ROTATION_SPEED);
    }
    if (IsKeyDown(KEY_D)) {
        p->dir = Vector2Rotate(p->dir, dt * PLAYER_ROTATION_SPEED);
    }
    if (IsKeyDown(KEY_W)) {
        if (check_for_obstacle(p->pos, p->dir, 0.3)) {
          return;
        } else {
          p->pos = Vector2Add(p->pos, Vector2Scale(p->dir, dt * PLAYER_SPEED));
        }
    }
    if (IsKeyDown(KEY_S)) {
      if (check_for_obstacle(p->pos, Vector2Scale(p->dir, -1), 0.3)) {
        return;
      } else {
        p->pos = Vector2Add(p->pos, Vector2Scale(p->dir, -dt * PLAYER_SPEED));
      }
    }
    if (IsKeyDown(KEY_E)) {
      if (check_for_obstacle(p->pos, Vector2Rotate(p->dir, PI / 2.0), 0.3)) {
        return;
      } else {
        p->pos = Vector2Add(p->pos, Vector2Scale(Vector2Rotate(p->dir, PI / 2.0), dt * PLAYER_SPEED));
      }
    }
    if (IsKeyDown(KEY_Q)) {
      if (check_for_obstacle(p->pos, Vector2Rotate(p->dir, -PI / 2.0), 0.3)) {
        return;
      } else {
        p->pos = Vector2Add(p->pos, Vector2Scale(Vector2Rotate(p->dir, -PI / 2.0), dt * PLAYER_SPEED));
      }
    }
}

void draw_walls(const Player *p) {
    float alpha = -FOV_ANGLE / 2.0;
    float alpha_step = FOV_ANGLE * RAY_RES / SCREEN_W;
    for (int slice_x = 0; slice_x < SCREEN_W; slice_x += RAY_RES) {
        Vector2 ray = Vector2Rotate(p->dir, alpha);
        raycast_walls(p, ray, slice_x);
        alpha += alpha_step;
    }
}

void draw_background(const Player *p) {
    Vector2 r0 = Vector2Rotate(p->dir, -FOV_ANGLE / 2.0);
    Vector2 r1 = Vector2Rotate(p->dir, FOV_ANGLE / 2.0);
    int h = SCREEN_H/2;
    const pixel_t *floor_texture = assets_map[tx_wood];
    const pixel_t *ceil_texture = assets_map[tx_greystone];
    for(int y = 0; y < h; y += RAY_RES) {
        float h_cam = (float)SCREEN_W/2.0;
        float row_dist = (h_cam / (h - y));
        Vector2 floor_step = Vector2Scale(Vector2Subtract(r1, r0), (row_dist * RAY_RES) / SCREEN_W);
        Vector2 floor = Vector2Add(p->pos, Vector2Scale(r0, row_dist));
        for(int x = 0; x < SCREEN_W; x += RAY_RES) {
            Vector2 cell = { .x = floorf(floor.x), .y = floorf(floor.y) };
            int tx = TEXTURE_SIZE * (floor.x - cell.x);
            int ty = TEXTURE_SIZE * (floor.y - cell.y);
            
            Color c = GetColor(ceil_texture[ty * TEXTURE_SIZE + tx]);
            c = ColorBrightness(c, -(row_dist / MAX_RENDER_DIST));
            DrawRectangle(x, y, RAY_RES, RAY_RES, c);

            Color c2 = GetColor(floor_texture[ty * TEXTURE_SIZE + tx]);
            c2 = ColorBrightness(c2, -(row_dist / MAX_RENDER_DIST));
            DrawRectangle(x, SCREEN_H - y - RAY_RES, RAY_RES, RAY_RES, c2);
            floor = Vector2Add(floor, floor_step);
        }
    }
}

void draw_sprites(const Player *p) {
    for (size_t i = 0; i < NUM_SPRITES; i++) {
        if (sprites[i].update) {
            sprites[i].update(&sprites[i]);
        }
        sprites[i].dist = Vector2LengthSqr(Vector2Subtract(sprites[i].pos, p->pos));
    }

    qsort(sprites, NUM_SPRITES, sizeof(Sprite), compare_sprite_dist);

    Vector2 plane = {
        -p->dir.y * tanf(FOV_ANGLE / 2.0f),
         p->dir.x * tanf(FOV_ANGLE / 2.0f)
    };

    for (size_t i = 0; i < NUM_SPRITES; i++) {
        Sprite sprite = sprites[i];

        Vector2 rel = Vector2Subtract(sprite.pos, p->pos);
        float invDet = 1.0f / (plane.x * p->dir.y - p->dir.x * plane.y);
        Vector2 transform = {
            .x = p->dir.y * rel.x - p->dir.x * rel.y,
            .y = -plane.y * rel.x + plane.x * rel.y
        };
        transform = Vector2Scale(transform, invDet);

        if (transform.y <= 0.0f || transform.y >= MAX_RENDER_DIST) continue;

        int spriteScreenX = (int)((SCREEN_W / 2.0f) * (1 + transform.x / transform.y));
        int vmove = (int)((sprite.vmove*SCREEN_W) / transform.y);

        int spriteHeight = abs((int)((SCREEN_W * (1.0 - sprite.vdiv)) / transform.y));
        int drawStartY = (-spriteHeight + SCREEN_H)/2;
        if (vmove >= 0) drawStartY += vmove;
        if (drawStartY < 0) drawStartY = 0;
        int drawEndY = (spriteHeight + SCREEN_H)/2;
        if(vmove < 0) drawEndY += vmove;
        if (drawEndY >= SCREEN_H) drawEndY = SCREEN_H - 1;

        int spriteWidth = abs((int)((SCREEN_W * (1.0 - sprite.hdiv)) / transform.y));
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if (drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if (drawEndX >= SCREEN_W) drawEndX = SCREEN_W - 1;

        const pixel_t *tex = assets_map[sprite.texture_id];

        for (int x = drawStartX; x < drawEndX; x += RAY_RES) {
            int texX = (x - (-spriteWidth / 2 + spriteScreenX)) * TEXTURE_SIZE / spriteWidth;

            if (transform.y < zbuffer[x / RAY_RES]) {
                for (int y = drawStartY; y < drawEndY; y += RAY_RES) {
                    int d = ((y - vmove)*256 - SCREEN_H * 128 + spriteHeight * 128);
                    int texY = (d * TEXTURE_SIZE) / (spriteHeight * 256);

                    pixel_t texel = tex[texY * TEXTURE_SIZE + texX];
                    if (texel != EMPTY_PIXEL) {
                        Color c = GetColor(texel);
                        c = ColorBrightness(c, -(transform.y / MAX_RENDER_DIST));
                        DrawRectangle(x, y, RAY_RES, RAY_RES, c);
                    }
                }
            }
        }
    }
}

#ifdef ESP32
int app_main()
#else
int main()
#endif
{
    init_game();
    InitWindow(SCREEN_W, SCREEN_H, "ray");
    SetTargetFPS(TARGET_FPS);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    Player p = {.pos = {.x = 1.2, .y = 1.3}, .dir = {.x = 1, .y = 0}};

    while (!WindowShouldClose()) {
        move_player(&p);
        BeginDrawing();
        draw_background(&p);
        draw_walls(&p);
        draw_sprites(&p);
        #ifdef DEBUG
        draw_minimap();
        draw_minimap_player(p.pos);
        float fps = GetFPS();
        char fps_text[16];
        snprintf(fps_text, sizeof(fps_text), "FPS: %.2f", fps);
        DrawText(fps_text, SCREEN_W - 150, 30, 20, RAYWHITE);
        #endif
        EndDrawing();
    }
    return 0;
}
