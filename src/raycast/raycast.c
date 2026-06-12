#include "raycast.h"

#include <stdlib.h>

#define THRESHOLD 0.0001

static int compare_sprite_dist(const void *a, const void *b) {
    const Entity *sa = (const Entity *)a;
    const Entity *sb = (const Entity *)b;
    if (sa->dist < sb->dist) return 1;
    if (sa->dist > sb->dist) return -1;
    return 0;
}

void raycast_walls(GameState *state, Vector2 dir, int slice_x) {
    Player *p = &state->player;
    if (dir.x == 0.0) dir.x = THRESHOLD;
    if (dir.y == 0.0) dir.y = THRESHOLD;
    Vector2 rs = Vector2Add(p->pos, Vector2Scale(dir, THRESHOLD));
    bool hit_vertical = false;
    while (Vector2Length(Vector2Subtract(rs, p->pos)) <= MAX_RENDER_DIST) {
        Vector2 cell = {.x = floorf(rs.x), .y = floorf(rs.y)};
        if (rs.x > 0.0 && rs.x < state->map_cols && rs.y > 0.0 && rs.y < state->map_rows) {
            uint8_t map_cell = state->map[(int)cell.y * state->map_cols + (int)cell.x];
            if (map_cell) {
                // draw slice
                float dist = Vector2DotProduct(Vector2Subtract(rs, p->pos), p->dir);
                state->zbuffer[slice_x / state->ray_res] = dist;
                int h = state->screen_width / dist;
                float bright_factor = 1.0 / dist - 0.9;
                if (bright_factor >= 0.0) bright_factor = 0.0;

                if (map_cell >= 128) {
                    // color
                    pixel_t c = color_brightness(color_map[map_cell - 128], bright_factor);
                    draw_rectangle(slice_x, (state->screen_height - h) / 2.0, state->ray_res, h, c);
                } else {
                    const pixel_t *tex = state->assets_map[map_cell];
                    int texture_x;
                    float diff_x = rs.x - cell.x;
                    float diff_y = rs.y - cell.y;


                    if (hit_vertical) {
                        texture_x = TEXTURE_SIZE - (diff_y * TEXTURE_SIZE);
                    } else {
                        texture_x = diff_x * TEXTURE_SIZE;
                    }
                    int hmax = h;
                    if(hmax > state->screen_height) hmax = state->screen_height;
                    for (int y = 0; y < hmax; y+=state->ray_res) {
                        int overflow_screen = (h-hmax)/2;
                        int texture_y = ((overflow_screen+y) * TEXTURE_SIZE) / h;
                        pixel_t texel = tex[texture_y * TEXTURE_SIZE + texture_x];
                        texel = color_brightness(texel, bright_factor);
                        draw_rectangle(slice_x, (state->screen_height - hmax) / 2 + y, state->ray_res, state->ray_res, texel);
                    }
                }
                return;
            }
        }
        float distX = cell.x + (dir.x >= 0 ? 1.0 : -THRESHOLD) - rs.x;
        float distY = cell.y + (dir.y >= 0 ? 1.0 : -THRESHOLD) - rs.y;
        Vector2 inc;
        if (fabs(distX / dir.x) < fabs(distY / dir.y)) {
            inc = (Vector2){.x = distX, .y = distX * dir.y / dir.x};
            hit_vertical = true;
        } else {
            inc = (Vector2){.x = distY * dir.x / dir.y, .y = distY};
            hit_vertical = false;
        }
        Vector2 new_rs = Vector2Add(rs, inc);
        rs = new_rs;
    }
    state->zbuffer[slice_x / state->ray_res] = MAX_RENDER_DIST;
}


void draw_walls(GameState *state) {
    Player *p = &state->player;
    float alpha = -FOV_ANGLE / 2.0;
    float alpha_step = FOV_ANGLE * state->ray_res / state->screen_width;
    for (int slice_x = 0; slice_x < state->screen_width; slice_x += state->ray_res) {
        Vector2 ray = Vector2Rotate(p->dir, alpha);
        raycast_walls(state, ray, slice_x);
        alpha += alpha_step;
    }
}


void draw_background(GameState *state) {
    Player *p = &state->player;
    Vector2 r0 = Vector2Rotate(p->dir, -FOV_ANGLE / 2.0);
    Vector2 r1 = Vector2Rotate(p->dir, FOV_ANGLE / 2.0);
    int h = state->screen_height/2;

    const pixel_t *floor_texture = NULL;
    const pixel_t *ceil_texture = NULL;

    if (state->floor_texture != 0) {
        floor_texture = state->assets_map[state->floor_texture];
    } else {
        draw_rectangle(0, state->screen_height/2, state->screen_width, state->screen_height/2, C_BLACK);
    }

    if (state->ceil_texture != 0) {
        ceil_texture = state->assets_map[state->ceil_texture];
    } else {
        draw_rectangle(0, 0, state->screen_width, state->screen_height/2, C_BLACK);
    }

    for(int y = 0; y < h; y += state->ray_res) {
        float h_cam = (float)state->screen_width/2.0;
        float row_dist = (h_cam / (h - y));
        Vector2 floor_step = Vector2Scale(Vector2Subtract(r1, r0), (row_dist * state->ray_res) / state->screen_width);
        Vector2 floor = Vector2Add(p->pos, Vector2Scale(r0, row_dist));
        for(int x = 0; x < state->screen_width; x += state->ray_res) {
            Vector2 cell = { .x = floorf(floor.x), .y = floorf(floor.y) };
            int tx = TEXTURE_SIZE * (floor.x - cell.x);
            int ty = TEXTURE_SIZE * (floor.y - cell.y);

            if (ceil_texture) {
                pixel_t c = ceil_texture[ty * TEXTURE_SIZE + tx];
                c = color_brightness(c, -(row_dist / MAX_RENDER_DIST));
                draw_rectangle(x, y, state->ray_res, state->ray_res, c);
            }

            if (floor_texture) {
                pixel_t c2 = floor_texture[ty * TEXTURE_SIZE + tx];
                c2 = color_brightness(c2, -(row_dist / MAX_RENDER_DIST));
                draw_rectangle(x, state->screen_height - y - state->ray_res, state->ray_res, state->ray_res, c2);
                floor = Vector2Add(floor, floor_step);
            }
        }
    }
}

void draw_entities(GameState *state) {
    Player *p = &state->player;
    for (size_t i = 0; i < state->entities.length; i++) {
        if (state->entities.data[i].disabled) continue;
        state->entities.data[i].dist = Vector2Length(Vector2Subtract(state->entities.data[i].pos, p->pos));
    }
    for (size_t i = 0; i < state->entities.length; i++) {
        if (state->entities.data[i].disabled || state->entities.data[i].update == NULL) continue;
        state->entities.data[i].update(state, &state->entities.data[i], i);
    }

    qsort(state->entities.data, state->entities.length, sizeof(Entity), compare_sprite_dist);

    Vector2 plane = {
        -p->dir.y * tanf(FOV_ANGLE / 2.0f),
         p->dir.x * tanf(FOV_ANGLE / 2.0f)
    };

    for (size_t i = 0; i < state->entities.length; i++) {
        if (state->entities.data[i].disabled) continue;
        Entity sprite = state->entities.data[i];

        Vector2 rel = Vector2Subtract(sprite.pos, p->pos);
        float invDet = 1.0f / (plane.x * p->dir.y - p->dir.x * plane.y);
        Vector2 transform = {
            .x = p->dir.y * rel.x - p->dir.x * rel.y,
            .y = -plane.y * rel.x + plane.x * rel.y
        };
        transform = Vector2Scale(transform, invDet);

        if (transform.y <= 0.0f || transform.y >= MAX_RENDER_DIST) continue;

        int spriteScreenX = (int)((state->screen_width / 2.0f) * (1 + transform.x / transform.y));
        int vmove = (int)((sprite.vmove*state->screen_width) / transform.y);

        int spriteHeight = abs((int)((state->screen_width * (1.0 - sprite.vdiv)) / transform.y));
        int drawStartY = (-spriteHeight + state->screen_height)/2;
        if (vmove >= 0) drawStartY += vmove;
        if (drawStartY < 0) drawStartY = 0;
        int drawEndY = (spriteHeight + state->screen_height)/2;
        if(vmove < 0) drawEndY += vmove;
        if (drawEndY >= state->screen_height) drawEndY = state->screen_height - 1;

        int spriteWidth = abs((int)((state->screen_width * (1.0 - sprite.hdiv)) / transform.y));
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if (drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if (drawEndX >= state->screen_width) drawEndX = state->screen_width - 1;

        const pixel_t *tex = state->assets_map[sprite.texture_id];

        for (int x = drawStartX; x < drawEndX; x += state->ray_res) {
            int texX = (x - (-spriteWidth / 2 + spriteScreenX)) * TEXTURE_SIZE / spriteWidth;

            if (transform.y < state->zbuffer[x / state->ray_res]) {
                for (int y = drawStartY; y < drawEndY; y += state->ray_res) {
                    int d = ((y - vmove)*256 - state->screen_height * 128 + spriteHeight * 128);
                    int texY = (d * TEXTURE_SIZE) / (spriteHeight * 256);

                    pixel_t texel = tex[texY * TEXTURE_SIZE + texX];
                    if (texel != EMPTY_PIXEL) {
                        texel = color_brightness(texel, -(transform.y / MAX_RENDER_DIST));
                        draw_rectangle(x, y, state->ray_res, state->ray_res, texel);
                    }
                }
            }
        }
    }
}

GameState state = {0};

void _init_game() {
    state.screen_width = 100;
    state.screen_height = 100;
    state.game_title = "Raycast";
    state.target_fps = 60;
    state.ray_res = 2;
    init_game(&state);
    state.player.dir = Vector2Normalize(state.player.dir);
    state.zbuffer = malloc(sizeof(float) * (state.screen_width / state.ray_res));
    renderer_init(
        state.screen_width,
        state.screen_height,
        state.game_title,
        state.target_fps
        );
    inputs_init();
}

void draw_game() {
  draw_background(&state);
  draw_walls(&state);
  draw_entities(&state);
}


void _update_game() {
  begin_drawing();
  update_game(&state);
  render_screen();
  end_drawing();
}

void _free_game() {
    free(state.zbuffer);
}
