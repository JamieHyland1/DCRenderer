#ifndef TILE_H
#define TILE_H
#include "pipeline.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <kos.h>

#define MAX_TRIANGLES_PER_TILE 100

#define TILE_X 64
#define TILE_Y 32

typedef struct{
  int x1;
  int y1;
  int x2;
  int y2;
  int tile_tri_count;
  triangle_t* triangles[MAX_TRIANGLES_PER_TILE];
  uint16_t* start_pos;
  float* z_start_pos;
}tile_t;

extern tile_t* tiles;
extern uint16_t* tile_buffer;
extern float* tile_z_buffer;
extern int x_tiles;
extern int y_tiles;
 

void setup_tiles();
void bin_triangles(const int num_triangles_to_render);
void DRAW_tiles(int DRAW_mode);
bool is_triangle_in_bin(const tile_t* tile, const triangle_t* tri);


#endif
