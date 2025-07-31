#ifndef clipping_h
#define clipping_h

#include "triangle.h"
#include "vector.h"

#define MAX_NUM_POLY_VERTS 10
#define MAX_NUM_POLY_TRIS 10
enum {
    LEFT_FRUSTUM_PLANE,
    RIGHT_FRUSTUM_PLANE,
    TOP_FRUSTUM_PLANE,
    BOTTOM_FRUSTUM_PLANE,
    NEAR_FRUSTUM_PLANE,
    FAR_FRUSTUM_PLANE
};

typedef struct 
{
  vec3f_t point;
  vec3f_t normal;
} plane_t;

typedef struct{
  vec3f_t vertices[MAX_NUM_POLY_VERTS];
  tex2_t texcoords[MAX_NUM_POLY_VERTS];
  int num_vertices;


}polygon_t;


polygon_t create_polygon_from_triangle(vec3f_t v0, vec3f_t v1, vec3f_t v2, tex2_t t0, tex2_t t1, tex2_t t2);

void triangles_from_polygon(polygon_t* polygon, triangle_t triangles_after_clipping[], int* num_triangles);  

void clip_polygon(polygon_t* polygon);
void clip_polygon_against_plane(polygon_t* polygon, int frustum_plane);
void init_frustum_planes(float fov_x,float fov_y, float znear, float zfar);

#endif