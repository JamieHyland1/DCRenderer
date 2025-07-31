#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdbool.h>
#include <stdint.h>
#include <dc/vector.h>
#include "vector.h"
#include "matrix.h"
#include "mesh.h"
#include "triangle.h"
#include "camera.h"
#include "clipping.h"

typedef enum {
    CULL_NONE,
    CULL_BACKFACE
} cull_method_t;

typedef enum {
    RENDER_WIRE,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
} render_method_t;

#define MAX_TRIANGLES_PER_MESH 10000

extern cull_method_t cull_mode;
extern render_method_t render_mode;
extern int num_triangles_to_render;
extern triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
extern mat4_t world_matrix;
extern mat4_t projection_matrix;
extern mat4_t view_matrix;

//TODO Break up process_graphics_pipeline into smaller functions
void process_graphics_pipeline(mesh_t *mesh);

#endif
