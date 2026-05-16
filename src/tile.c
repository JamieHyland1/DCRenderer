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
    float x0 = tri->points[0].x, x1 = tri->points[1].x, x2 = tri->points[2].x;
    float y0 = tri->points[0].y, y1 = tri->points[1].y, y2 = tri->points[2].y;
    
    float min_x = fminf(x0, fminf(x1, x2));
    float max_x = fmaxf(x0, fmaxf(x1, x2));
    float min_y = fminf(y0, fminf(y1, y2));
    float max_y = fmaxf(y0, fmaxf(y1, y2));

    tri->x1 = (int)floorf(min_x);
    tri->y1 = (int)floorf(min_y);
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
                tile_t *tile = &tiles[j];          
                if(tile->tile_tri_count < MAX_TRIANGLES_PER_TILE){
                    tile->triangles[tile->tile_tri_count] = tri;
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
        if(num_triangles_to_render == 0) continue;

        // Instead of two separate loops, combine them:
        for (int row = 0; row < TILE_Y; row++) {
            shz_memcpy32(
                tile_buffer + row * TILE_X,
                tile->start_pos + row * WINDOW_WIDTH,
                sizeof(uint16_t) * TILE_X
            );
            shz_memcpy32(
                tile_z_buffer + row * TILE_X,
                tile->z_start_pos + row * WINDOW_WIDTH,
                sizeof(float) * TILE_X
            );
        }

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
                case DRAW_TEXTURED_SCANLINE: {
                    const texture_t *texture = get_texture(tri->id);
                    draw_textured_triangle_scanline(tri, texture);
                    break;
                }
                default:
                    break;
            }
        }

        // Write back to main buffer (write - use shz_sq_memcpy32)
        tile->tile_tri_count = 0;
        for (int row = 0; row < TILE_Y; row++) {
            shz_sq_memcpy32(
                tile->start_pos + row * WINDOW_WIDTH,
                tile_buffer + row * TILE_X,
                sizeof(uint16_t) * TILE_X
            );

            shz_sq_memcpy32(
                tile->z_start_pos + row * WINDOW_WIDTH,
                tile_z_buffer + row * TILE_X,
                sizeof(float) * TILE_X
            );
        }
    }
}


bool is_triangle_in_bin(const tile_t *tile, const triangle_t *tri) {
    return (tri->x1 < tile->x2 &&
            tri->x2 > tile->x1 &&
            tri->y1 < tile->y2 &&
            tri->y2 > tile->y1);
}
