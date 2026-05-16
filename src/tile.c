#include "../include/renderer.h"
#include "../include/core.h"
#include <dc/minifont.h>
#include "shz_mem.h"
#include <stdint.h>
#include <stdlib.h>


tile_t* tiles;
uint16_t* tile_buffer;
float* tile_z_buffer;

int x_tiles = 0;
int y_tiles = 0;
int num_tiles = 0;

size_t tile_buffer_size;

void setup_tiles(){
    tile_buffer_size = sizeof(uint16_t) * TILE_X * TILE_Y;
    x_tiles = WINDOW_WIDTH/TILE_X;
    y_tiles = WINDOW_HEIGHT/TILE_Y;
    num_tiles = x_tiles * y_tiles;

    tiles = (tile_t*)aligned_alloc(32, sizeof(tile_t) * (x_tiles * y_tiles));
    tile_buffer = (uint16_t*)aligned_alloc(32, sizeof(uint16_t) * (TILE_X * TILE_Y));
    tile_z_buffer = (float*)aligned_alloc(32, sizeof(float) * (TILE_X * TILE_Y));

    for(int y = 0; y < y_tiles; y++){
        for(int x = 0; x < x_tiles; x++){
            int x1 = x  * TILE_X;
            int y1 = y  * TILE_Y;
            int x2 = x1 + TILE_X;
            int y2 = y1 + TILE_Y;
            tile_t tile = {x1, y1, x2, y2, 0, {0}};
            tile.start_pos = buffer + (y * TILE_Y) * WINDOW_WIDTH + (x * TILE_X);
            tile.z_start_pos = z_buffer + (y * TILE_Y) * WINDOW_WIDTH + (x * TILE_X);
            tiles[y * x_tiles + x] = tile;
        }
    } 
}

static inline void generate_triangle_bounds(triangle_t *tri) {
    float min_x = tri->points[0].x;
    float max_x = tri->points[0].x;

    float min_y = tri->points[0].y;
    float max_y = tri->points[0].y;

    for (int i = 1; i < 3; i++) {
        float x = tri->points[i].x;
        float y = tri->points[i].y;

        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
        if (y < min_y) min_y = y;
        if (y > max_y) max_y = y;
    }

    tri->x1 = (int)floorf(min_x);
    tri->y1 = (int)floorf(min_y);

    /*
        x2/y2 are exclusive bounds.

        So if x1 = 10 and x2 = 21,
        the covered pixel range is x = 10..20.
    */
    tri->x2 = (int)ceilf(max_x) + 1;
    tri->y2 = (int)ceilf(max_y) + 1;
}

void bin_triangles(int num_triangles_to_render){
    for (int i = 0; i < num_tiles; i++) {
    tiles[i].tile_tri_count = 0;
}
    for(int i = 0; i < num_triangles_to_render; i++){
        triangle_t *tri = &triangles_to_render[i];  
        generate_triangle_bounds(tri);              

        int tx0 = tri->x1 / TILE_X;
        int tx1 = (tri->x2 - 1) / TILE_X;
        int ty0 = tri->y1 / TILE_Y;
        int ty1 = (tri->y2 - 1) / TILE_Y;

        // clamp to valid tile range
        if (tx0 < 0) tx0 = 0;
        if (tx1 >= x_tiles) tx1 = x_tiles - 1;
        if (ty0 < 0) ty0 = 0;
        if (ty1 >= y_tiles) ty1 = y_tiles - 1;

        for(int ty = ty0; ty <= ty1; ty++){
            for(int tx = tx0; tx <= tx1; tx++){
                int j = ty * x_tiles + tx;
                tile_t *tile = &tiles[j];           // fix 2: pointer, not copy
                bool in_bin = is_triangle_in_bin(tile, tri);
                if(in_bin && tile->tile_tri_count < MAX_TRIANGLES_PER_TILE){
                    tile->triangles[tile->tile_tri_count] = tri;  // fix 3: store pointer to actual element
                    tile->tile_tri_count++;
                }
            }
        }
    }
}

void draw_tiles(int draw_mode){
    for(int i = num_tiles-1; i >= 0; i--){
        tile_t* tile = &tiles[i];
        int num_triangles_to_render = tile->tile_tri_count;
        if(num_triangles_to_render > 0){

            for (int row = 0; row < TILE_Y; row++) {
                memcpy(
                tile_buffer + row * TILE_X,
                tile->start_pos + row * WINDOW_WIDTH,
                sizeof(uint16_t) * TILE_X
                );
            }

            for (int row = 0; row < TILE_Y; row++) {
                memcpy(
                    tile_z_buffer + row * TILE_X,
                    tile->z_start_pos + row * WINDOW_WIDTH,
                    sizeof(float) * TILE_X
                );
            }

            draw_line(tile->x1, tile->y1, tile->x2, tile->y1, 0xFFFF);
            draw_line(tile->x1, tile->y1, tile->x1, tile->y2, 0xFFFF);
            draw_line(tile->x2, tile->y1, tile->x2, tile->y2, 0xFFFF);
            draw_line(tile->x1, tile->y2, tile->x2, tile->y2, 0xFFFF);

            tile_offset_x = tile->x1;
            tile_offset_y = tile->y1;

            for (int j = 0; j < num_triangles_to_render; j++)
            {
                triangle_t* tri = tile->triangles[j];
                switch (draw_mode) {
                    case DRAW_WIRE:
                        draw_triangle(tri, 0xFFFF);
                        break;

                    case DRAW_FILL_TRIANGLE:
                        draw_filled_triangle(tri, 0xF800);
                        break;

                    case DRAW_FILL_TRIANGLE_WIRE:
                        draw_filled_triangle_wire(tri, 0xF800);
                        break;

                    case DRAW_TEXTURED:
                        start_time = perf_cntr_timer_ns();
                        /* draw_textured_triangle(&tri); */
                        end_time = perf_cntr_timer_ns();
                        break;

                    case DRAW_TEXTURED_SCANLINE: {
                        const texture_t *texture = get_texture(tri->id);
                        // if (triangle_fully_inside_screen(&tri)) {
                        draw_textured_triangle_scanline(tri, texture);
                        // } else {
                        //     draw_textured_triangle_scanline_fast(&tri, texture);
                        // }
                        break;
                    }

                    default:
                        break;
                }
            }
            // push the tile buffers to the main buffer
            tile->tile_tri_count = 0;
            for (int row = 0; row < TILE_Y; row++) {
                shz_memcpy32(
                    tile->start_pos + row * WINDOW_WIDTH,
                    tile_buffer + row * TILE_X,
                    sizeof(uint16_t) * TILE_X
                );

                shz_memcpy32(
                    tile->z_start_pos + row * WINDOW_WIDTH,
                    tile_z_buffer  + row * TILE_X,
                    sizeof(float) * TILE_X
                );

            }
            for (int i = 0; i < TILE_X * TILE_Y; i++) {
                tile_z_buffer[i] = 1.0f;
            }

        }
    }
}


bool is_triangle_in_bin(const tile_t *tile, const triangle_t *tri) {
    return (tri->x1 < tile->x2 &&
            tri->x2 > tile->x1 &&
            tri->y1 < tile->y2 &&
            tri->y2 > tile->y1);
}
