#include "yari.h"

#include <stdlib.h>

#define THRESHOLD 0.0001

static int compare_sprite_dist(const void *a, const void *b) {
    const YrEntity *sa = (const YrEntity *)a;
    const YrEntity *sb = (const YrEntity *)b;
    if (sa->dist < sb->dist) return 1;
    if (sa->dist > sb->dist) return -1;
    return 0;
}

static inline int fixed16_to_int(int value) {
    return value >= 0 ? (value >> 16) : -((-value) >> 16);
}

static inline float yr_projection_plane_scale(const YrGameState *state) {
    static float base_scale = 0.0f;
    if (base_scale == 0.0f) base_scale = tanf(YR_FOV_ANGLE / 2.0f);
    return base_scale * (float)state->screen_width / (float)state->screen_height;
}

static inline void yr_draw_texture_column(
    int x,
    int y,
    int width,
    int height,
    const yr_pixel_t *texture,
    int texture_x,
    int texture_y,
    int texture_height,
    float brightness,
    bool skip_empty
) {
    if (width <= 0 || height <= 0 || texture_height <= 0 || !texture) return;
    if (texture_x < 0 || texture_x >= YR_TEXTURE_SIZE) return;

    int step = width;
    int tex_pos = (int)(((int64_t)texture_y * (YR_TEXTURE_SIZE << 16)) / texture_height);
    int tex_step = (int)(((int64_t)step * (YR_TEXTURE_SIZE << 16)) / texture_height);

    for (int row = 0; row < height; row += step, tex_pos += tex_step) {
        int tex_y = fixed16_to_int(tex_pos);
        if (tex_y < 0 || tex_y >= YR_TEXTURE_SIZE) continue;

        yr_pixel_t texel = texture[tex_y * YR_TEXTURE_SIZE + texture_x];
        if (skip_empty && texel == YR_EMPTY_PIXEL) continue;

        int block_h = step;
        if (row + block_h > height) block_h = height - row;

        texel = yr_color_brightness(texel, brightness);
        yr_draw_rectangle(x, y + row, width, block_h, texel);
    }
}

/**
 * Performs raycasting for a single vertical slice of the screen to find wall intersections and render them.
 * Uses DDA algorithm to step through the grid map and find the first wall hit by the ray.
 * It also applies distance-based brightness to the wall slice.
 * @param state The current game state containing camera, map, and rendering information.
 * @param dir The direction vector of the ray being cast.
 * @param slice_x The x-coordinate of the vertical slice on the screen to render.
 */
void yr_raycast_walls(YrGameState *state, Vector2 dir, int slice_x) {
    YrCamera *p = &state->camera;
    float v_shift_base = p->horizon * state->screen_height * 0.5f;
    float roll = ((float)slice_x - state->screen_width * 0.5f) * tanf(p->angle);
    int v_shift = (int)(v_shift_base + roll);
    int z_index = slice_x / state->ray_res;

    if (dir.x > -THRESHOLD && dir.x < THRESHOLD) dir.x = (dir.x < 0.0f) ? -THRESHOLD : THRESHOLD;
    if (dir.y > -THRESHOLD && dir.y < THRESHOLD) dir.y = (dir.y < 0.0f) ? -THRESHOLD : THRESHOLD;

    int cell_x = (int)p->pos.x;
    int cell_y = (int)p->pos.y;
    float abs_dir_x = dir.x < 0.0f ? -dir.x : dir.x;
    float abs_dir_y = dir.y < 0.0f ? -dir.y : dir.y;
    float delta_dist_x = 1.0f / abs_dir_x;
    float delta_dist_y = 1.0f / abs_dir_y;
    float dist_x; // distance from the ray's origin to the next vertical grid line
    float dist_y; // distance from the ray's origin to the next horizontal grid line
    int step_x;
    int step_y;

    // Determine the step direction and initial distances to the next grid lines based on the ray's direction.
    if (dir.x < 0.0f) {
        // If the ray is pointing left, step_x is -1 and dist_x is the distance to the left grid line.
        step_x = -1;
        dist_x = (p->pos.x - (float)cell_x) * delta_dist_x;
    } else {
        // If the ray is pointing right, step_x is 1 and dist_x is the distance to the right grid line.
        step_x = 1;
        dist_x = ((float)cell_x + 1.0f - p->pos.x) * delta_dist_x;
    }

    if (dir.y < 0.0f) {
        // If the ray is pointing up, step_y is -1 and dist_y is the distance to the upper grid line.
        step_y = -1;
        dist_y = (p->pos.y - (float)cell_y) * delta_dist_y;
    } else {
        // If the ray is pointing down, step_y is 1 and dist_y is the distance to the lower grid line.
        step_y = 1;
        dist_y = ((float)cell_y + 1.0f - p->pos.y) * delta_dist_y;
    }

    float ray_dist = 0.0f;
    bool hit_vertical = false;

    while (ray_dist <= YR_MAX_RENDER_DIST) {
        // calculate the next grid cell the ray will intersect
        if (dist_x < dist_y) {
            cell_x += step_x;
            ray_dist = dist_x;
            dist_x += delta_dist_x;
            hit_vertical = true;
        } else {
            cell_y += step_y;
            ray_dist = dist_y;
            dist_y += delta_dist_y;
            hit_vertical = false;
        }

        if (cell_x < 0 || cell_x >= state->map_cols || cell_y < 0 || cell_y >= state->map_rows) {
            break;
        }

        uint8_t map_cell = state->map[cell_y * state->map_cols + cell_x];
        if (!map_cell) continue; // empty cell, keep raycasting

        float dist = ray_dist;
        if (dist < THRESHOLD) dist = THRESHOLD;
        if (dist > YR_MAX_RENDER_DIST) break; // stop if the distance exceeds the maximum render distance

        state->zbuffer[z_index] = dist; // store distance in z-buffer for sprite rendering
        int h = (int)((float)state->screen_height / dist);
        float bright_factor = 1.0f / dist - 0.9f;
        if (bright_factor > 0.0f) bright_factor = 0.0f;

        // If the map cell is a special color-coded cell (>= 128), draw a solid color rectangle instead of a texture.
        if (map_cell >= 128) {
            yr_pixel_t c = yr_color_brightness(yr_color_map[map_cell - 128], bright_factor);
            yr_draw_rectangle(slice_x, (state->screen_height - h) / 2 + v_shift, state->ray_res, h, c);
            return;
        }

        // If the map cell corresponds to a texture, calculate the appropriate texture coordinates and draw the textured column.
        const yr_pixel_t *tex = state->assets_map[map_cell];
        Vector2 rs = {
            .x = p->pos.x + dir.x * ray_dist,
            .y = p->pos.y + dir.y * ray_dist
        };
        float diff = hit_vertical ? rs.y - (float)((int)rs.y) : rs.x - (float)((int)rs.x);
        if (diff < 0.0f) diff += 1.0f;

        int texture_x = (int)(diff * (float)YR_TEXTURE_SIZE);
        if (texture_x < 0) texture_x = 0;
        if (texture_x >= YR_TEXTURE_SIZE) texture_x = YR_TEXTURE_SIZE - 1;
        if (hit_vertical) texture_x = YR_TEXTURE_SIZE - texture_x - 1;

        int hmax = h;
        if (hmax > state->screen_height) hmax = state->screen_height;
        int overflow_screen = (h - hmax) / 2;
        int draw_y = (state->screen_height - hmax) / 2 + v_shift;
        yr_draw_texture_column(
            slice_x,
            draw_y,
            state->ray_res,
            hmax,
            tex,
            texture_x,
            overflow_screen,
            h,
            bright_factor,
            false);
        return;
    }

    state->zbuffer[z_index] = YR_MAX_RENDER_DIST;
}

/**
 * Renders the walls of the scene by performing raycasting for each vertical slice of the screen.
 * This function iterates over the screen width, casting rays and drawing the corresponding wall slices.
 */
void yr_draw_walls(YrGameState *state) {
    YrCamera *p = &state->camera;
    float scale = yr_projection_plane_scale(state);
    Vector2 plane = { .x = -p->dir.y * scale, .y = p->dir.x * scale };
    int screen_width = state->screen_width;
    int ray_res = state->ray_res;
    float camera_x = -1.0f;
    float camera_step = 2.0f * (float)ray_res / (float)screen_width;

    for (int slice_x = 0; slice_x < screen_width; slice_x += ray_res, camera_x += camera_step) {
        Vector2 ray = {
            .x = p->dir.x + plane.x * camera_x,
            .y = p->dir.y + plane.y * camera_x
        };
        yr_raycast_walls(state, ray, slice_x);
    }
}


/**
 * Renders the background (floor and ceiling) of the scene using a raycasting approach.
 * For each vertical slice of the screen, it calculates the corresponding floor and ceiling texture coordinates and draws the textured columns. 
 * It also applies distance-based brightness to create a sense of depth.
 * If no floor or ceiling texture is provided, it fills the respective areas with a solid color (black).
 */
void yr_draw_background(YrGameState *state) {
    YrCamera *p = &state->camera;
    float scale = yr_projection_plane_scale(state);
    Vector2 plane = { .x = -p->dir.y * scale, .y = p->dir.x * scale };
    Vector2 r0 = { .x = p->dir.x - plane.x, .y = p->dir.y - plane.y };
    Vector2 r1 = { .x = p->dir.x + plane.x, .y = p->dir.y + plane.y };

    Vector2 ray_dir = Vector2Subtract(r1, r0);
    float inv_sw = 1.0f / (float)state->screen_width;
    int sw = state->screen_width;
    int sh = state->screen_height;
    int rr = state->ray_res;
    float h_cam = (float)sh * 0.5f;
    float half_h = (float)sh * 0.5f;

    const yr_pixel_t *floor_tex = NULL;
    const yr_pixel_t *ceil_tex = NULL;
    if (state->floor_texture) floor_tex = state->assets_map[state->floor_texture];
    if (state->ceil_texture) ceil_tex = state->assets_map[state->ceil_texture];

    float tan_angle = tanf(p->angle);
    float half_w = (float)sw * 0.5f;

    for (int x = 0; x < sw; x += rr) {
        float horizon = half_h + p->horizon * half_h + ((float)x - half_w) * tan_angle;
        int hz = (int)horizon;
        if (hz < 0) hz = 0;
        if (hz > sh) hz = sh;

        // Calculate the base ray direction for the current vertical slice
        Vector2 base_ray = Vector2Add(r0, Vector2Scale(ray_dir, (float)x * inv_sw));

        if (ceil_tex) {
            for (int y = 0; y < hz; y += rr) {
                float row_dist = h_cam / (float)(hz - y); // distance from the camera to the point on the ceiling corresponding to this pixel row
                if (row_dist >= YR_MAX_RENDER_DIST) {
                    yr_draw_rectangle(x, y, rr, rr, YR_BLACK);
                    continue;
                }
                float brightness = -(row_dist / YR_MAX_RENDER_DIST);
                Vector2 w = Vector2Add(p->pos, Vector2Scale(base_ray, row_dist)); // world coordinates of the point on the ceiling corresponding to this pixel row
                if (state->map_ceil) {
                    int cell_x = (int)w.x;
                    int cell_y = (int)w.y;
                    if(cell_x >= 0 && cell_x < state->map_cols && cell_y >= 0 && cell_y < state->map_rows && state->map_ceil[cell_y * state->map_cols + cell_x]) {
                        ceil_tex = state->assets_map[state->map_ceil[cell_y * state->map_cols + cell_x]];
                    } else {
                        ceil_tex = state->assets_map[state->ceil_texture];
                    }
                }
                int tx = ((int)(w.x * (float)YR_TEXTURE_SIZE)) & (YR_TEXTURE_SIZE - 1);
                int ty = ((int)(w.y * (float)YR_TEXTURE_SIZE)) & (YR_TEXTURE_SIZE - 1);
                yr_pixel_t c = yr_color_brightness(ceil_tex[ty * YR_TEXTURE_SIZE + tx], brightness);
                yr_draw_rectangle(x, y, rr, rr, c);
            }
        } else if (hz > 0) {
            yr_draw_rectangle(x, 0, rr, hz, YR_BLACK);
        }

        if (floor_tex) {
            for (int y = hz; y < sh; y += rr) {
                float row_dist = h_cam / (float)(y - hz + 1); // distance from the camera to the point on the floor corresponding to this pixel row
                if (row_dist >= YR_MAX_RENDER_DIST) {
                    yr_draw_rectangle(x, y, rr, rr, YR_BLACK);
                    continue;
                }
                float brightness = -(row_dist / YR_MAX_RENDER_DIST);
                Vector2 w = Vector2Add(p->pos, Vector2Scale(base_ray, row_dist)); // world coordinates of the point on the floor corresponding to this pixel row
                if (state->map_floor && w.x >= 0 && (int)w.x < state->map_cols && w.y >= 0 && (int)w.y < state->map_rows) {
                    int cell_x = (int)w.x;
                    int cell_y = (int)w.y;
                    if(cell_x >= 0 && cell_x < state->map_cols && cell_y >= 0 && cell_y < state->map_rows && state->map_floor[cell_y * state->map_cols + cell_x]) {
                        floor_tex = state->assets_map[state->map_floor[cell_y * state->map_cols + cell_x]];
                    } else {
                        floor_tex = state->assets_map[state->floor_texture];
                    }
                }
                int tx = ((int)(w.x * (float)YR_TEXTURE_SIZE)) & (YR_TEXTURE_SIZE - 1);
                int ty = ((int)(w.y * (float)YR_TEXTURE_SIZE)) & (YR_TEXTURE_SIZE - 1);
                yr_pixel_t c = yr_color_brightness(floor_tex[ty * YR_TEXTURE_SIZE + tx], brightness);
                yr_draw_rectangle(x, y, rr, rr, c);
            }
        } else if (sh - hz > 0) {
            yr_draw_rectangle(x, hz, rr, sh - hz, YR_BLACK);
        }
    }
}

/**
 * Renders the entities (sprites) in the scene. It first calculates the distance of each entity from the camera, sorts them by distance, and then renders them in back-to-front order to ensure proper occlusion. 
 * For each entity, it calculates the appropriate screen position and size based on its distance and renders it using its associated texture. 
 * It also applies distance-based brightness to create a sense of depth.
 */
void yr_draw_entities(YrGameState *state) {
    YrCamera *p = &state->camera;
    // Update entity distances
    for (size_t i = 0; i < state->entities.length; i++) {
        if (state->entities.data[i].disabled) continue;
        state->entities.data[i].dist = Vector2Length(Vector2Subtract(state->entities.data[i].pos, p->pos));
    }

    // Sort entities by distance from the camera in descending order (farthest first) for proper rendering.
    qsort(state->entities.data, state->entities.length, sizeof(YrEntity), compare_sprite_dist);

    float half_screen = state->screen_width * 0.5f;
    float tan_angle = tanf(p->angle);
    float scale = yr_projection_plane_scale(state);
    float projection_scale = (float)state->screen_height;
    Vector2 plane = { .x = -p->dir.y * scale, .y = p->dir.x * scale };
    float invDet = 1.0f / (plane.x * p->dir.y - p->dir.x * plane.y);

    for (size_t i = 0; i < state->entities.length; i++) {
        if (state->entities.data[i].disabled) continue;
        YrEntity sprite = state->entities.data[i];

        /**
         * Calculate the position of the sprite on the screen using an inverse camera transformation.
         * This involves translating the sprite's world position relative to the camera, applying the inverse of the camera's rotation and projection to determine where it should appear on the screen.
         * The resulting screen coordinates are then used to determine the size and position of the sprite's texture on the screen, as well as its brightness based on distance from the camera.
         */
        Vector2 rel = Vector2Subtract(sprite.pos, p->pos);
        Vector2 transform = {
            .x = p->dir.y * rel.x - p->dir.x * rel.y,
            .y = -plane.y * rel.x + plane.x * rel.y
        };
        transform = Vector2Scale(transform, invDet);

        if (transform.y <= 0.0f || transform.y >= YR_MAX_RENDER_DIST) continue;

        int spriteScreenX = (int)(half_screen * (1 + transform.x / transform.y));
        int v_shift = (int)(p->horizon * state->screen_height * 0.5f + ((float)spriteScreenX - half_screen) * tan_angle);
        int vmove = (int)((sprite.vmove * projection_scale) / transform.y);

        int spriteHeight = abs((int)((projection_scale * (1.0 - sprite.vdiv)) / transform.y));
        if (spriteHeight <= 0) continue;
        int spriteTop = (state->screen_height - spriteHeight) / 2 + vmove + v_shift;
        int spriteBottom = spriteTop + spriteHeight;
        int drawStartY = spriteTop;
        if (drawStartY < 0) drawStartY = 0;
        int drawEndY = spriteBottom;
        if (drawEndY > state->screen_height) drawEndY = state->screen_height;
        if (drawEndY <= drawStartY) continue;

        int spriteWidth = abs((int)((projection_scale * (1.0 - sprite.hdiv)) / transform.y));
        if (spriteWidth <= 0) continue;
        int spriteLeft = spriteScreenX - spriteWidth / 2;
        int spriteRight = spriteLeft + spriteWidth;
        int drawStartX = spriteLeft;
        if (drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteRight;
        if (drawEndX > state->screen_width) drawEndX = state->screen_width;
        if (drawEndX <= drawStartX) continue;

        const yr_pixel_t *tex = state->assets_map[sprite.texture_id];

        // Render the sprite column by column, applying distance-based brightness and checking against the z-buffer for proper occlusion with walls.
        for (int x = drawStartX; x < drawEndX; x += state->ray_res) {
            int texX = (x - spriteLeft) * YR_TEXTURE_SIZE / spriteWidth;
            if (texX < 0) texX = 0;
            if (texX >= YR_TEXTURE_SIZE) texX = YR_TEXTURE_SIZE - 1;

            if (transform.y < state->zbuffer[x / state->ray_res]) {
                int texture_y = drawStartY - spriteTop;
                yr_draw_texture_column(
                    x,
                    drawStartY,
                    state->ray_res,
                    drawEndY - drawStartY,
                    tex,
                    texX,
                    texture_y,
                    spriteHeight,
                    -(transform.y / YR_MAX_RENDER_DIST),
                    true);
            }
        }
    }

    // Update entities (call their update functions)
    for (size_t i = 0; i < state->entities.length; i++) {
        if (state->entities.data[i].disabled || state->entities.data[i].update == NULL) continue;
        state->entities.data[i].update(state, &state->entities.data[i], i);
    }
}

YrGameState state = {0};

void _yr_init_game() {
    state.screen_width = 100;
    state.screen_height = 100;
    state.game_title = "Yari";
    state.target_fps = 60;
    state.ray_res = 2;
    yr_init_game(&state);
    if (state.ray_res == 0) state.ray_res = 1;
    state.camera.dir = Vector2Normalize(state.camera.dir);
    state.zbuffer = malloc(sizeof(float) * ((state.screen_width + state.ray_res - 1) / state.ray_res));
    yr_renderer_init(
        state.screen_width,
        state.screen_height,
        state.game_title,
        state.target_fps
        );
    yr_inputs_init();
}

void yr_draw_game() {
  yr_draw_background(&state);
  yr_draw_walls(&state);
  yr_draw_entities(&state);
}


static float game_start_time = -1.0f;

void _yr_update_game() {
  yr_begin_drawing();
  if (game_start_time < 0.0f) game_start_time = yr_get_time();
  state.game_time = (uint32_t)((yr_get_time() - game_start_time) * 1000.0f);
  yr_update_game(&state);
  yr_render_screen();
  yr_end_drawing();
}

void _yr_free_game() {
    free(state.zbuffer);
}
