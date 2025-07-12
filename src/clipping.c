#include "../include/clipping.h"
#include <math.h>
#include "../include/texture.h"
#include <kos.h>
#define NUM_PLANES 6

plane_t frustum_planes[NUM_PLANES];


void init_frustum_planes(float fov_x, float fov_y, float znear, float zfar){
    float cos_half_fov_x = cos(fov_x/2);
    float sin_half_fov_x = sin(fov_x/2);

    float cos_half_fov_y = cos(fov_y/2);
    float sin_half_fov_y = sin(fov_y/2);


    vec3f_t origin = {0,0,0};
    
    frustum_planes[LEFT_FRUSTUM_PLANE].point  = origin;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal = (vec3f_t){cos_half_fov_x,0,sin_half_fov_x};

    
    
    frustum_planes[RIGHT_FRUSTUM_PLANE].point  = origin;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal = (vec3f_t){-cos_half_fov_x,0,sin_half_fov_x};

    frustum_planes[TOP_FRUSTUM_PLANE].point  = origin;
    frustum_planes[TOP_FRUSTUM_PLANE].normal = (vec3f_t){0,-cos_half_fov_y,sin_half_fov_y};

    frustum_planes[BOTTOM_FRUSTUM_PLANE].point  = origin;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal = (vec3f_t){0,cos_half_fov_y,sin_half_fov_y};

    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y += 0.05;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.y -= 0.05;

    frustum_planes[NEAR_FRUSTUM_PLANE].point  = (vec3f_t){0,0,znear};
    frustum_planes[NEAR_FRUSTUM_PLANE].normal = (vec3f_t){0,0,1};

    
    frustum_planes[FAR_FRUSTUM_PLANE].point  = (vec3f_t){0,0,zfar};
    frustum_planes[FAR_FRUSTUM_PLANE].normal = (vec3f_t){0,0,-1};
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

polygon_t create_polygon_from_triangle(vec3f_t v0, vec3f_t v1, vec3f_t v2, tex2_t t0, tex2_t t1, tex2_t t2){
    polygon_t polygon = {.vertices = {v0,v1,v2},
    .texcoords = {t0,t1,t2},
    .num_vertices = 3};

    return polygon;
}

void clip_polygon_against_plane(polygon_t* polygon, int frustum_plane){
    vec3f_t plane_point  = frustum_planes[frustum_plane].point;
    vec3f_t plane_normal = frustum_planes[frustum_plane].normal;

    vec3f_t inside_vertices [MAX_NUM_POLY_VERTS];
    tex2_t inside_texcoords[MAX_NUM_POLY_VERTS];
    int num_inside_vertices = 0;

    vec3f_t* current_vertex   = &polygon->vertices[0];
    tex2_t* current_texcoord = &polygon->texcoords[0];

    vec3f_t* previous_vertex    = &polygon->vertices[polygon->num_vertices-1];
    tex2_t* previous_texcoords = &polygon->texcoords[polygon->num_vertices-1];

    float current_dot  = 0;
    float previous_dot = vec_dot(vec3_sub(*previous_vertex,plane_point),plane_normal);

    while(current_vertex != &polygon->vertices[polygon->num_vertices]){
        current_dot  = vec_dot(vec3_sub(*current_vertex,plane_point),plane_normal);


        //if the current dot product X previous dot product is less than 0 we know that one of them is outside the viewing plane
        // 1  *  1 =  1 => both are in the plane
        //-1  * -1 =  1 => both are outside the plane
        //-1  *  1 = -1 => one of the dot products are outside while one is inside
        if(current_dot * previous_dot < 0){
            float t = current_dot / (current_dot-previous_dot);
            vec3f_t c_v = vec3_new(current_vertex->x,current_vertex->y,current_vertex->z);
            vec3f_t p_v = vec3_new(previous_vertex->x,previous_vertex->y,previous_vertex->z);
            vec3f_t I   = vec3_add(c_v,vec3_mult(vec3_sub(p_v,c_v),t));

            //Same as I above just written out at once
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

