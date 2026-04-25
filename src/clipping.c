#include "../include/clipping.h"
#include <math.h>
#include "../include/texture.h"
#include <kos.h>
#include "shz_trig.h"
#include "shz_matrix.h"
#include "shz_xmtrx.h"
#define NUM_PLANES 6

plane_t frustum_planes[NUM_PLANES];

shz_sincos_t pairX, pairY;
void init_frustum_planes(float fov_x, float fov_y, float znear, float zfar){
    pairX = shz_sincosf(fov_x*0.5);
    pairY = shz_sincosf(fov_y*0.5);
    
    float cos_half_fov_x = pairX.cos;
    float sin_half_fov_x = pairX.sin;

    float cos_half_fov_y = pairY.cos;
    float sin_half_fov_y = pairY.sin;

    shz_vec3_t origin;
    origin.x = 0.0f;
    origin.y = 0.0f;
    origin.z = 0.0f; 

    frustum_planes[LEFT_FRUSTUM_PLANE].point  = origin;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal = (shz_vec3_t){{{cos_half_fov_x, 0, sin_half_fov_x}}};


    frustum_planes[RIGHT_FRUSTUM_PLANE].point  = origin;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal = (shz_vec3_t){{{-cos_half_fov_x, 0, sin_half_fov_x}}};

    frustum_planes[TOP_FRUSTUM_PLANE].point  = origin;
    frustum_planes[TOP_FRUSTUM_PLANE].normal = (shz_vec3_t){{{0, -cos_half_fov_y, sin_half_fov_y}}};

    frustum_planes[BOTTOM_FRUSTUM_PLANE].point  = origin;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal = (shz_vec3_t){{{0, cos_half_fov_y, sin_half_fov_y}}};

    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y += 0.05;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.y -= 0.05;

    frustum_planes[NEAR_FRUSTUM_PLANE].point  = (shz_vec3_t){{{0, 0, znear}}};
    frustum_planes[NEAR_FRUSTUM_PLANE].normal = (shz_vec3_t){{{0, 0, 1}}};

    
    frustum_planes[FAR_FRUSTUM_PLANE].point  = (shz_vec3_t){{{0, 0, zfar}}};
    frustum_planes[FAR_FRUSTUM_PLANE].normal = (shz_vec3_t){{{0, 0, -1}}};
}

void triangles_from_polygon(polygon_t* polygon, triangle_t triangles[], int* num_triangles){
    for(int i = 0; i < polygon->num_vertices-2; i++){
            int index0 = 0;
            int index1 = i+1;
            int index2 = i+2;

            triangles[i].points[0] = vec4_from_vec3(polygon->vertices[index0]);
            triangles[i].points[1] = vec4_from_vec3(polygon->vertices[index1]);
            triangles[i].points[2] = vec4_from_vec3(polygon->vertices[index2]);


            triangles[i].texcoords[0] = (polygon->texcoords[index0]);
            triangles[i].texcoords[1] = (polygon->texcoords[index1]);
            triangles[i].texcoords[2] = (polygon->texcoords[index2]);
        }

        *num_triangles = polygon->num_vertices-2;
}

float float_lerp(float a, float b, float t){
    return a + t * (b - a);
}

polygon_t create_polygon_from_triangle(shz_vec3_t v0, shz_vec3_t v1, shz_vec3_t v2, tex2_t t0, tex2_t t1, tex2_t t2){
    polygon_t polygon = {.vertices = {v0,v1,v2},
    .texcoords = {t0,t1,t2},
    .num_vertices = 3};

    return polygon;
}

void clip_polygon_against_plane(polygon_t* polygon, int frustum_plane){
    shz_vec3_t plane_point  = frustum_planes[frustum_plane].point;
    shz_vec3_t plane_normal = frustum_planes[frustum_plane].normal;

    shz_vec3_t inside_vertices [MAX_NUM_POLY_VERTS];
    tex2_t inside_texcoords[MAX_NUM_POLY_VERTS];
    int num_inside_vertices = 0;

    shz_vec3_t* current_vertex   = &polygon->vertices[0];
    tex2_t* current_texcoord = &polygon->texcoords[0];

    shz_vec3_t* previous_vertex    = &polygon->vertices[polygon->num_vertices-1];
    tex2_t* previous_texcoords = &polygon->texcoords[polygon->num_vertices-1];

    float current_dot  = 0;
    float previous_dot = shz_vec_dot(shz_vec_sub(*previous_vertex,plane_point),plane_normal);

    while(current_vertex != &polygon->vertices[polygon->num_vertices]){
        current_dot  = shz_vec_dot(shz_vec_sub(*current_vertex,plane_point),plane_normal);


        //if the current dot product X previous dot product is less than 0 we know that one of them is outside the viewing plane
        // 1  *  1 =  1 => both are in the plane
        //-1  * -1 =  1 => both are outside the plane
        //-1  *  1 = -1 => one of the dot products are outside while one is inside
        if(current_dot * previous_dot < 0){
            float t = current_dot / (current_dot-previous_dot);
            shz_vec3_t c_v = vec3_new(current_vertex->x,current_vertex->y,current_vertex->z);
            shz_vec3_t p_v = vec3_new(previous_vertex->x,previous_vertex->y,previous_vertex->z);
            shz_vec3_t I   = shz_vec_add(c_v,shz_vec3_scale(shz_vec_sub(p_v,c_v),t));

            tex2_t I_UV =   {
                .u = float_lerp(current_texcoord->u,previous_texcoords->u,t),
                .v = float_lerp(current_texcoord->v,previous_texcoords->v,t)
            };


            inside_vertices[num_inside_vertices] = vec3_new(I.x,I.y,I.z);
            inside_texcoords[num_inside_vertices] = I_UV;
            num_inside_vertices++;
        }

        // If the current vertex is inside the plane
        if(current_dot > 0){
            inside_vertices [num_inside_vertices] = vec3_new(current_vertex->x,current_vertex->y,current_vertex->z);
            inside_texcoords[num_inside_vertices] = new_tex2(current_texcoord->u,current_texcoord->v);
            num_inside_vertices++;
        }

        //move to the next vertex
        previous_dot = current_dot;
        previous_vertex = current_vertex;
        previous_texcoords = current_texcoord;
        current_texcoord++;
        current_vertex++;
    }
    for(int i = 0; i < num_inside_vertices; i++){
        polygon->vertices[i] = vec3_new(inside_vertices[i].x,inside_vertices[i].y,inside_vertices[i].z);
        polygon->texcoords[i] = new_tex2(inside_texcoords[i].u,inside_texcoords[i].v);

    }
    polygon->num_vertices = num_inside_vertices;
}


void clip_polygon(polygon_t* polygon){
    clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);
}

static inline shz_vec4_t make_plane_eq(plane_t p) {
    shz_vec4_t eq = {0};
    float d = -shz_vec_dot(p.normal, p.point);
    eq.x = p.normal.x;
    eq.y = p.normal.y;
    eq.z = p.normal.z;
    eq.w = d;
    return eq;
}

static inline bool clip_against_plane_shz_t(shz_vec3_t v0,
                                            shz_vec3_t v1,
                                            plane_t plane,
                                            float *t_out,
                                            shz_vec3_t *out) {
    shz_vec3_t line_dir = shz_vec_sub(v1, v0);
    float denom = shz_vec_dot(plane.normal, line_dir);

    if (fabsf(denom) < 1e-6f) {
        return false;
    }

    float t =  shz_divf((shz_vec_dot(plane.normal, plane.point) - shz_vec_dot(plane.normal, v0)),denom);
    *t_out = t;
    *out = shz_vec_add(v0, shz_vec3_scale(line_dir, t));
    return true;
}
static inline void check_against_frustum_planes(
    shz_vec3_t *inside_vertices,
    tex2_t *inside_texcoords,
    int *num_inside_vertices,
    const shz_vec3_t *current_vertex,
    const shz_vec3_t *previous_vertex,
    const tex2_t *current_texcoord,
    const tex2_t *previous_texcoords,
    int index
) {
    shz_vec3_t I;
    float t;

    if (clip_against_plane_shz_t(*current_vertex,
                                 *previous_vertex,
                                 frustum_planes[index],
                                 &t,
                                 &I)) {
        tex2_t I_UV = {
            .u = shz_lerpf(current_texcoord->u, previous_texcoords->u, t),
            .v = shz_lerpf(current_texcoord->v, previous_texcoords->v, t)
        };

        inside_vertices[*num_inside_vertices] = I;
        inside_texcoords[*num_inside_vertices] = I_UV;
        (*num_inside_vertices)++;
    }
}

void clip_polygon_against_frustum(polygon_t *polygon) {
    shz_vec3_t inside_vertices[MAX_NUM_POLY_VERTS];
    tex2_t inside_texcoords[MAX_NUM_POLY_VERTS];
    int num_inside_vertices = 0;

    shz_vec3_t *current_vertex = &polygon->vertices[0];
    tex2_t *current_texcoord = &polygon->texcoords[0];

    shz_vec3_t *previous_vertex = &polygon->vertices[polygon->num_vertices - 1];
    tex2_t *previous_texcoords = &polygon->texcoords[polygon->num_vertices - 1];

    shz_vec4_t left_eq   = make_plane_eq(frustum_planes[LEFT_FRUSTUM_PLANE]);
    shz_vec4_t right_eq  = make_plane_eq(frustum_planes[RIGHT_FRUSTUM_PLANE]);
    shz_vec4_t top_eq    = make_plane_eq(frustum_planes[TOP_FRUSTUM_PLANE]);
    shz_vec4_t bottom_eq = make_plane_eq(frustum_planes[BOTTOM_FRUSTUM_PLANE]);
    shz_vec4_t near_eq   = make_plane_eq(frustum_planes[NEAR_FRUSTUM_PLANE]);
    shz_vec4_t far_eq    = make_plane_eq(frustum_planes[FAR_FRUSTUM_PLANE]);

    shz_mat4x4_t m = {
        left_eq.x,   right_eq.x,   top_eq.x,   bottom_eq.x,
        left_eq.y,   right_eq.y,   top_eq.y,   bottom_eq.y,
        left_eq.z,   right_eq.z,   top_eq.z,   bottom_eq.z,
        left_eq.w,   right_eq.w,   top_eq.w,   bottom_eq.w
    };

    shz_xmtrx_load_4x4(&m);

    while (current_vertex != &polygon->vertices[polygon->num_vertices]) {
        shz_vec4_t cur4 = {0};
        cur4.x = current_vertex->x;
        cur4.y = current_vertex->y;
        cur4.z = current_vertex->z;
        cur4.w = 1.0f;

        shz_vec4_t prev4 = {0};
        prev4.x = previous_vertex->x;
        prev4.y = previous_vertex->y;
        prev4.z = previous_vertex->z;
        prev4.w = 1.0f;

        shz_vec4_t cur_dist  = shz_xmtrx_transform_vec4(cur4);
        shz_vec4_t prev_dist = shz_xmtrx_transform_vec4(prev4);

        float curr_near_dist = fipr(cur4.x,  cur4.y,  cur4.z,  cur4.w,
                                    near_eq.x, near_eq.y, near_eq.z, near_eq.w);
        float prev_near_dist = fipr(prev4.x, prev4.y, prev4.z, prev4.w,
                                    near_eq.x, near_eq.y, near_eq.z, near_eq.w);

        float curr_far_dist  = fipr(cur4.x,  cur4.y,  cur4.z,  cur4.w,
                                    far_eq.x, far_eq.y, far_eq.z, far_eq.w);
        float prev_far_dist  = fipr(prev4.x, prev4.y, prev4.z, prev4.w,
                                    far_eq.x, far_eq.y, far_eq.z, far_eq.w);

        bool current_inside_all =
            (cur_dist.x >= 0.0f) &&
            (cur_dist.y >= 0.0f) &&
            (cur_dist.z >= 0.0f) &&
            (cur_dist.w >= 0.0f) &&
            (curr_near_dist >= 0.0f) &&
            (curr_far_dist >= 0.0f);

        /* side planes */
        for (int i = 0; i < 4; i++) {
            float cd = cur_dist.e[i];
            float pd = prev_dist.e[i];

            if (cd * pd < 0.0f) {
                check_against_frustum_planes(
                    inside_vertices,
                    inside_texcoords,
                    &num_inside_vertices,
                    current_vertex,
                    previous_vertex,
                    current_texcoord,
                    previous_texcoords,
                    i
                );
            }
        }

        /* near plane */
        if (curr_near_dist * prev_near_dist < 0.0f) {
            check_against_frustum_planes(
                inside_vertices,
                inside_texcoords,
                &num_inside_vertices,
                current_vertex,
                previous_vertex,
                current_texcoord,
                previous_texcoords,
                NEAR_FRUSTUM_PLANE
            );
        }

        /* far plane */
        if (curr_far_dist * prev_far_dist < 0.0f) {
            check_against_frustum_planes(
                inside_vertices,
                inside_texcoords,
                &num_inside_vertices,
                current_vertex,
                previous_vertex,
                current_texcoord,
                previous_texcoords,
                FAR_FRUSTUM_PLANE
            );
        }

        if (current_inside_all) {
            inside_vertices[num_inside_vertices] = *current_vertex;
            inside_texcoords[num_inside_vertices] = *current_texcoord;
            num_inside_vertices++;
        }

        previous_vertex = current_vertex;
        previous_texcoords = current_texcoord;
        current_vertex++;
        current_texcoord++;
    }

    for (int i = 0; i < num_inside_vertices; i++) {
        polygon->vertices[i] = inside_vertices[i];
        polygon->texcoords[i] = inside_texcoords[i];
    }

    polygon->num_vertices = num_inside_vertices;
}