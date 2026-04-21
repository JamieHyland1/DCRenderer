#ifndef pipeline_h
#define pipeline_h

#pragma once

#define MAX_TRIANGLES_PER_MESH 10000
extern enum cull_method cull_mode;
extern float znear;

extern mat4_t projection_matrix;
extern mat4_t view_matrix;
extern mat4_t world_matrix;

extern matrix_t v_mat;
extern matrix_t w_mat;
extern matrix_t p_mat;
extern matrix_t pv_mat;

extern triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
extern triangle_t skybox_triangles_to_render[N_CUBE_FACES];
extern int num_triangles_to_render;
extern int num_skybox_triangles_to_render;

void process_graphics_pipeline(object_t *obj);
shz_vec4_t vec4_from_vec3f(shz_vec3_t v);

#endif