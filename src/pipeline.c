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


/* Remember to fix this to remove the skybox from the regular mesh pipeline */
/* void process_skybox() {
    mesh_t* mesh = get_mesh(0);

    shz_xmtrx_init_identity();
    shz_xmtrx_apply_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);

    shz_xmtrx_apply_rotation_z(mesh->rotation.z);
   // shz_xmtrx_apply_rotation_y(mesh->rotation.y += 0.01f);
    shz_xmtrx_apply_rotation_x(mesh->rotation.x);

    shz_xmtrx_apply_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    // its trying to replace these
    shz_xmtrx_store_4x4(&w_mat);
    shz_xmtrx_load_4x4(&v_mat);
    shz_xmtrx_apply_4x4(&w_mat);
    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++) {
        face_t current_face = mesh->faces[i];

        shz_vec3_t face_vertices[3] = {
            mesh->vertices[current_face.a],
            mesh->vertices[current_face.b],
            mesh->vertices[current_face.c]
        };

        shz_vec4_t transformed_vertices[3];
        for (int j = 0; j < 3; j++) {
            shz_vec4_t v = vec4_from_vec3(face_vertices[j]);

            float x = v.x, y = v.y, z = v.z;
            mat_trans_single3_nodiv(x, y, z);

            v.x = x; v.y = y; v.z = z;
            transformed_vertices[j] = v;
        }

        shz_vec3_t origin = shz_vec3_init(0, 0, 0);
        /////////////////
        // Cull back faces
        /////////////////
        shz_vec3_t cameraRay = shz_vec_sub(origin, vec3_from_vec4(transformed_vertices[0]));
        shz_vec3_t tri_normal = get_triangle_face_normal(transformed_vertices);
        tri_normal = shz_vec_normalize(tri_normal);

        // float orientation_from_camera = shz_vec_dot(tri_normal, cameraRay);
        // if (cull_mode == CULL_BACKFACE)
        // {
        //     if (orientation_from_camera < 0)
        //     {
        //         continue;
        //     }
        // }

        

        // Near-plane safety guard (optional but recommended)
        if (transformed_vertices[0].z <= znear ||
            transformed_vertices[1].z <= znear ||
            transformed_vertices[2].z <= znear) {
            continue;
        }

        // Skip clipping: triangle "after clipping" is the original
        triangle_t tri;
        tri.points[0] = transformed_vertices[0];
        tri.points[1] = transformed_vertices[1];
        tri.points[2] = transformed_vertices[2];
        tri.texcoords[0] = current_face.a_uv;
        tri.texcoords[1] = current_face.b_uv;
        tri.texcoords[2] = current_face.c_uv;

        // Project
        shz_vec4_t projected_points[3];
        for (int j = 0; j < 3; j++) {
            projected_points[j] = mat4_mul_vec4_project(projection_matrix, tri.points[j]);

            projected_points[j].x *= WINDOW_WIDTH * 0.5f;
            projected_points[j].y *= WINDOW_HEIGHT * 0.5f;
            projected_points[j].y *= -1;
            projected_points[j].x += WINDOW_WIDTH * 0.5f;
            projected_points[j].y += WINDOW_HEIGHT * 0.5f;

            // Force far depth (choose correct constant for your z-test)
            projected_points[j].z = 1.0f;
        }

        triangle_t triangle_to_render = {
            .points = {
                {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
            },
            .texcoords = {
                {tri.texcoords[0].u, tri.texcoords[0].v},
                {tri.texcoords[1].u, tri.texcoords[1].v},
                {tri.texcoords[2].u, tri.texcoords[2].v}
            },
            .texture = &mesh->texture_info,
            .orientation_from_light = 0xFFFF
        };

        if (num_skybox_triangles_to_render < N_CUBE_FACES) {
            skybox_triangles_to_render[num_skybox_triangles_to_render++] = triangle_to_render;
        }
    }
   // printf("Processed skybox with %d triangles\n", num_skybox_triangles_to_render );
} */


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
void process_graphics_pipeline(object_t* obj)
{
    mesh_t* mesh = get_mesh(obj->id);
    // World matrix
    shz_xmtrx_init_identity();
    shz_xmtrx_apply_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    shz_xmtrx_apply_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    shz_xmtrx_apply_rotation_z(mesh->rotation.z);
    shz_xmtrx_apply_rotation_y(mesh->rotation.y += 0.01f);
    shz_xmtrx_apply_rotation_x(mesh->rotation.x );
    // its trying to replace these
    shz_xmtrx_store_4x4(&w_mat);
    shz_xmtrx_load_4x4(&v_mat);
    shz_xmtrx_apply_4x4(&w_mat);
    //
    // shz_xmtrx_load_4x4_apply_store(&pv_mat, &v_mat, &w_mat);
    // shz_xmtrx_load_4x4(&w_mat);
    // shz_xmtrx_invert();
    // shz_xmtrx_transpose();


    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++)
    {

        face_t current_face = mesh->faces[i];

        shz_vec3_t face_vertices[3];

        face_vertices[0] = mesh->vertices[current_face.a];
        face_vertices[1] = mesh->vertices[current_face.b];
        face_vertices[2] = mesh->vertices[current_face.c];

        shz_vec4_t transformed_vertices[3];

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
        shz_vec3_t origin = shz_vec3_init(0, 0, 0);
        /////////////////
        // Cull back faces
        /////////////////
        shz_vec3_t cameraRay = shz_vec_sub(origin, vec3_from_vec4(transformed_vertices[0]));
        shz_vec3_t tri_normal = get_triangle_face_normal(transformed_vertices);
        tri_normal = shz_vec_normalize(tri_normal);

        float orientation_from_camera = shz_vec_dot(tri_normal, cameraRay);
        if (cull_mode == CULL_BACKFACE)
        {
            if (orientation_from_camera < 0)
            {
                continue;
            }
        }

        ////////////////
        // Clipping stage
        ////////////////

        // Create a polygon from the transformed triangle
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            current_face.a_uv,
            current_face.b_uv,
            current_face.c_uv);


        // TODO refactor this to use FIPVR 
        clip_polygon(&polygon);


        // Break clipped polygon into triangles to be rendered
        triangle_t tris_after_clipping[MAX_NUM_POLY_TRIS];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, tris_after_clipping, &num_triangles_after_clipping);

        for (int t = 0; t < num_triangles_after_clipping; t++)
        {

            triangle_t triangle_after_clipping = tris_after_clipping[t];

            shz_vec4_t projected_points[3];
            for (int j = 0; j < 3; j++)
            {
                // TODO switch to sh4zam mtrx code
                //  float x = triangle_after_clipping.points[j].x;
                //  float y = triangle_after_clipping.points[j].y;
                //  float z = triangle_after_clipping.points[j].z;
                //  float w = triangle_after_clipping.points[j].w;

                // mat_trans_single4(x, y, z, w);
                // //mat_trans_single3_nomod(x, y, z, projected_points[j].x, projected_points[j].y, projected_points[j].z);

                // projected_points[j].x = x / w;
                // projected_points[j].y = y / w;
                // projected_points[j].z = z / w;
                // projected_points[j].w = w / w;

                projected_points[j] = mat4_mul_vec4_project(projection_matrix, triangle_after_clipping.points[j]);

                // Scale to middle of screen
                projected_points[j].x *= WINDOW_WIDTH * 0.5f;
                projected_points[j].y *= WINDOW_HEIGHT * 0.5f;

                // Invert Y values to account for flipped y screen coordinate
                projected_points[j].y *= -1;

                // Translate to middle of screen
                projected_points[j].x += WINDOW_WIDTH * 0.5f;
                projected_points[j].y += WINDOW_HEIGHT * 0.5f;
            }

            ////////////////
            //Lighting stage  TODO; add proper lighiting when sh4zam gets inv_mtrx_transpose
            ////////////////

            //  shz_vec3_t tri_center = {
            //     (triangle_after_clipping.points[0].x +
            //     triangle_after_clipping.points[1].x +
            //     triangle_after_clipping.points[2].x) / 3.0f,
            //     (triangle_after_clipping.points[0].y +
            //     triangle_after_clipping.points[1].y +
            //     triangle_after_clipping.points[2].y) / 3.0f,
            //     (triangle_after_clipping.points[0].z +
            //     triangle_after_clipping.points[1].z +
            //     triangle_after_clipping.points[2].z) / 3.0f,
            // };

            // shz_vec3_t L0 = shz_vec_normalize(shz_vec_sub(point_lights[0].position, tri_center));
            // shz_vec3_t L1 = shz_vec_normalize(shz_vec_sub(point_lights[1].position, tri_center));
            // shz_vec3_t L2 = shz_vec_normalize(shz_vec_sub(point_lights[2].position, tri_center));
            // shz_vec3_t L3 = shz_vec_normalize(shz_vec_sub(point_lights[3].position, tri_center));
            

            // shz_matrix_4x4_t light_matrix = {
            //     .elem = {
            //         L0.x, L0.y, L0.z, 0.0f,
            //         L1.x, L1.y, L1.z, 0.0f,
            //         L2.x, L2.y, L2.z, 0.0f,
            //         L3.x, L3.y, L3.z, 0.0f
            //     }
            // };

            // shz_xmtrx_init_identity();
            // shz_xmtrx_load_4x4(&light_matrix);
           


            float orientation_from_light = shz_vec_dot(tri_normal, get_light_direction());
            orientation_from_light = orientation_from_light < 0.15 ? 0.15 : orientation_from_light;
            orientation_from_light = orientation_from_light > 1.00 ? 1.00 : orientation_from_light;
            

            triangle_t triangle_to_render = {
                .points = {
                    {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                    {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                    {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
                },
                .texcoords = {{triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v}, {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v}, {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v}},
                .id = obj->id,
                .orientation_from_light = 0xFFFF};
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
            {
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render++;
            }
        }
    }
}
