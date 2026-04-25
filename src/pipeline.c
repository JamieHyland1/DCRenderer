#include <stdio.h>
#include "../include/renderer.h"
#include "dc/syscalls.h"
#include "shz_trig.h"
#include "shz_matrix.h"
#include "shz_xmtrx.h"
#include "shz_fpscr.h"

mat4_t world_matrix;
mat4_t projection_matrix;
mat4_t view_matrix;

matrix_t v_mat;
matrix_t w_mat;
matrix_t p_mat;
matrix_t pv_mat;

triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
triangle_t skybox_triangles_to_render[N_CUBE_FACES];
int num_triangles_to_render = 0;
int num_skybox_triangles_to_render = 0;

enum cull_method cull_mode;
float znear = 0.1f;

// =================== DEBUG VARS ==================
#ifdef DEBUG
uint64_t dbg_pipeline_world_ns      = 0;
uint64_t dbg_pipeline_transform_ns  = 0;
uint64_t dbg_pipeline_cull_ns       = 0;
uint64_t dbg_pipeline_clip_ns       = 0;
uint64_t dbg_pipeline_project_ns    = 0;
uint64_t dbg_pipeline_emit_ns       = 0;

uint64_t dbg_pipeline_objects       = 0;
uint64_t dbg_pipeline_faces         = 0;
uint64_t dbg_pipeline_faces_culled  = 0;
uint64_t dbg_pipeline_polys_clipped = 0;
uint64_t dbg_pipeline_tris_emitted  = 0;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline in various stages
//
//  [ MODEL SPACE ] <- Original mesh vertices, all centered around the origin 0,0,0
//
//       [ WORLD SPACE ] <- Multiply vertices by world matrix, essentially "places" meshes in the correct spot in the scene
//
//           [ CAMERA SPACE ] <- Multiple vertices by the view matrix, essentially putting them in view relative to the camera
//
//               [ CLIPPING ] <- Clip any vertices in the meshes that are outside the bounds of the frustum plane
//
//                   [ PROJECTION ] <- Multiple by the projection matrix, flatten the 3D world into a 2D image
//
//                       [ IMAGE SPACE ] <- Apply perspective divide on the meshes adding depth to the scene
//
//                           [ SCREEN SPACE ] <- Final image is now ready to be rendered to the screen
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void process_graphics_pipeline(object_t* obj){
    mesh_t* mesh = get_mesh(obj->id);

    #ifdef DEBUG
        dbg_pipeline_objects++;
        uint64_t t0;
    #endif

        // World matrix
    #ifdef DEBUG
        t0 = perf_cntr_timer_ns();
    #endif


    shz_xmtrx_init_identity();
    shz_xmtrx_apply_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    shz_xmtrx_apply_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    shz_xmtrx_apply_rotation_z(mesh->rotation.z);
    shz_xmtrx_apply_rotation_y(mesh->rotation.y += 0.001f);
    shz_xmtrx_apply_rotation_x(mesh->rotation.x);
    shz_xmtrx_store_4x4((shz_mat4x4_t *)&w_mat);
    shz_xmtrx_load_4x4((const shz_mat4x4_t *)&v_mat);
    shz_xmtrx_apply_4x4((const shz_mat4x4_t *)&w_mat);


    #ifdef DEBUG
        dbg_pipeline_world_ns += (perf_cntr_timer_ns() - t0);
    #endif

    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++)
    {
        #ifdef DEBUG
                dbg_pipeline_faces++;
        #endif

        face_t current_face = mesh->faces[i];

        shz_vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[current_face.a];
        face_vertices[1] = mesh->vertices[current_face.b];
        face_vertices[2] = mesh->vertices[current_face.c];

        shz_vec4_t transformed_vertices[3];

        #ifdef DEBUG
                t0 = perf_cntr_timer_ns();
        #endif
        for (int j = 0; j < 3; j++)
        {
            shz_vec4_t current_vertex = vec4_from_vec3(face_vertices[j]);

            float x = current_vertex.x;
            float y = current_vertex.y;
            float z = current_vertex.z;
            float w = current_vertex.w;

            mat_trans_single3_nodiv(x, y, z);

            current_vertex.x = x;
            current_vertex.y = y;
            current_vertex.z = z;
            current_vertex.w = w;
            transformed_vertices[j] = current_vertex;
        }
        #ifdef DEBUG
                dbg_pipeline_transform_ns += (perf_cntr_timer_ns() - t0);
        #endif

        shz_vec3_t origin = shz_vec3_init(0, 0, 0);

        #ifdef DEBUG
                t0 = perf_cntr_timer_ns();
        #endif
        shz_vec3_t cameraRay = shz_vec_sub(origin, vec3_from_vec4(transformed_vertices[0]));
        shz_vec3_t tri_normal = get_triangle_face_normal(transformed_vertices);
        tri_normal = shz_vec_normalize(tri_normal);

        float orientation_from_camera = shz_vec_dot(tri_normal, cameraRay);
        if (cull_mode == CULL_BACKFACE)
        {
            if (orientation_from_camera < 0)
            {
                #ifdef DEBUG
                    dbg_pipeline_faces_culled++;
                    dbg_pipeline_cull_ns += (perf_cntr_timer_ns() - t0);
                #endif
                continue;
            }
        }
        #ifdef DEBUG
                dbg_pipeline_cull_ns += (perf_cntr_timer_ns() - t0);
        #endif

        // Clipping stage
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            current_face.a_uv,
            current_face.b_uv,
            current_face.c_uv);

        #ifdef DEBUG
                t0 = perf_cntr_timer_ns();
        #endif
        shz_mat4x4_t saved_xmtrx;
        shz_xmtrx_store_4x4(&saved_xmtrx);
        shz_xmtrx_init_identity();
        clip_polygon_against_frustum(&polygon);
        shz_xmtrx_load_4x4((const shz_mat4x4_t *)&saved_xmtrx);

        triangle_t tris_after_clipping[MAX_NUM_POLY_TRIS];
        int num_triangles_after_clipping = 0;
        triangles_from_polygon(&polygon, tris_after_clipping, &num_triangles_after_clipping);
        #ifdef DEBUG
                dbg_pipeline_polys_clipped++;
                dbg_pipeline_clip_ns += (perf_cntr_timer_ns() - t0);
        #endif

        for (int t = 0; t < num_triangles_after_clipping; t++)
        {
            triangle_t triangle_after_clipping = tris_after_clipping[t];
            shz_vec4_t projected_points[3];

            #ifdef DEBUG
                        t0 = perf_cntr_timer_ns();
            #endif
            for (int j = 0; j < 3; j++)
            {
                projected_points[j] = mat4_mul_vec4_project(projection_matrix, triangle_after_clipping.points[j]);

                projected_points[j].x *= WINDOW_WIDTH * 0.5f;
                projected_points[j].y *= WINDOW_HEIGHT * 0.5f;
                projected_points[j].y *= -1;
                projected_points[j].x += WINDOW_WIDTH * 0.5f;
                projected_points[j].y += WINDOW_HEIGHT * 0.5f;
            }
            #ifdef DEBUG
                        dbg_pipeline_project_ns += (perf_cntr_timer_ns() - t0);
            #endif

            float orientation_from_light = shz_vec_dot(tri_normal, get_light_direction());
            orientation_from_light = orientation_from_light < 0.15f ? 0.15f : orientation_from_light;
            orientation_from_light = orientation_from_light > 1.00f ? 1.00f : orientation_from_light;

            triangle_t triangle_to_render = {
                .points = {
                    {{{projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w}}},
                    {{{projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w}}},
                    {{{projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w}}},
                },
                .texcoords = {
                    {triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v},
                    {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v},
                    {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v}
                },
                .id = obj->id,
                .orientation_from_light = 0xFFFF
            };

            #ifdef DEBUG
                        t0 = perf_cntr_timer_ns();
            #endif
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
            {
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render++;
                #ifdef DEBUG
                                dbg_pipeline_tris_emitted++;
                #endif
            }
            #ifdef DEBUG
                        dbg_pipeline_emit_ns += (perf_cntr_timer_ns() - t0);
            #endif
        }
    }
}

/* void translate_meshes(){
    int num_meshes = get_num_meshes();
    for(int i = 0; i < num_meshes; i++){
        mesh_t* mesh = get_mesh(obj->id);

        #ifdef DEBUG
            dbg_pipeline_objects++;
            uint64_t t0;
        #endif

            // World matrix
        #ifdef DEBUG
            t0 = perf_cntr_timer_ns();
        #endif


        shz_xmtrx_init_identity();
        shz_xmtrx_apply_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
        shz_xmtrx_apply_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
        shz_xmtrx_apply_rotation_z(mesh->rotation.z);
        shz_xmtrx_apply_rotation_y(mesh->rotation.y += 0.001f);
        shz_xmtrx_apply_rotation_x(mesh->rotation.x);
        shz_xmtrx_store_4x4((shz_mat4x4_t *)&w_mat);
        shz_xmtrx_load_4x4((const shz_mat4x4_t *)&v_mat);
        shz_xmtrx_apply_4x4((const shz_mat4x4_t *)&w_mat);


        #ifdef DEBUG
            dbg_pipeline_world_ns += (perf_cntr_timer_ns() - t0);
        #endif

             
    }
} */



void print_pipeline_debug_stats(int frame_count){
    #ifdef DEBUG
        if (frame_count <= 0) {
            printf("No frames recorded for pipeline debug stats.\n");
            return;
        }

        printf("Pipeline world setup: %.3f ms/frame\n",
            (double)dbg_pipeline_world_ns / (double)frame_count / 1e6);
        printf("Pipeline transform: %.3f ms/frame\n",
            (double)dbg_pipeline_transform_ns / (double)frame_count / 1e6);
        printf("Pipeline cull: %.3f ms/frame\n",
            (double)dbg_pipeline_cull_ns / (double)frame_count / 1e6);
        printf("Pipeline clip: %.3f ms/frame\n",
            (double)dbg_pipeline_clip_ns / (double)frame_count / 1e6);
        printf("Pipeline project: %.3f ms/frame\n",
            (double)dbg_pipeline_project_ns / (double)frame_count / 1e6);
        printf("Pipeline emit: %.3f ms/frame\n",
            (double)dbg_pipeline_emit_ns / (double)frame_count / 1e6);

        printf("Objects/frame: %.2f\n",
            (double)dbg_pipeline_objects / (double)frame_count);
        printf("Faces/frame: %.2f\n",
            (double)dbg_pipeline_faces / (double)frame_count);
        printf("Faces culled/frame: %.2f\n",
            (double)dbg_pipeline_faces_culled / (double)frame_count);
        printf("Polys clipped/frame: %.2f\n",
            (double)dbg_pipeline_polys_clipped / (double)frame_count);
        printf("Triangles emitted/frame: %.2f\n",
            (double)dbg_pipeline_tris_emitted / (double)frame_count);
    #else
        (void)frame_count;
        printf("Pipeline debug stats require DEBUG build.\n");
    #endif
}
