#include "../include/utils.h"
#include <dc/perf_monitor.h>
void debug_start(void) {
//    perf_monitor_start();
}

void debug_end(void) {
    // perf_monitor_stop();
    // perf_monitor_print(stdout);
}



// void process_graphics_pipeline_old(mesh_t *mesh){
//     vec3f_t target = get_camera_lookat_target();

//     view_matrix = mat4_look_at(get_camera_pos(), target, get_camera_up());

//     mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
//     mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);

//     mat4_t rotation_matrix_x = mat4_rotate_x(mesh->rotation.x);
//     mat4_t rotation_matrix_y = mat4_rotate_y(mesh->rotation.y += 0.02);
//     mat4_t rotation_matrix_z = mat4_rotate_z(mesh->rotation.z);

//     world_matrix = mat4_identity();

//     // Multiply Scale -> Rotation -> Translation ORDER MATTERS >:(
//     world_matrix = matrix_mult_mat4(scale_matrix, world_matrix);
//     world_matrix = matrix_mult_mat4(rotation_matrix_z, world_matrix);
//     world_matrix = matrix_mult_mat4(rotation_matrix_y, world_matrix);
//     world_matrix = matrix_mult_mat4(rotation_matrix_x, world_matrix);
//     world_matrix = matrix_mult_mat4(translation_matrix, world_matrix);

//     int num_faces = array_length(mesh->faces);
//     for (int i = 0; i < num_faces; i++)
//     {

//         face_t current_face = mesh->faces[i];

//         vec3f_t face_vertices[3];

//         face_vertices[0] = mesh->vertices[current_face.a];
//         face_vertices[1] = mesh->vertices[current_face.b];
//         face_vertices[2] = mesh->vertices[current_face.c];

//         vector_t transformed_vertices[3];

//         for (int j = 0; j < 3; j++)
//         {
//             vector_t current_vertex = vec4_from_vec3(face_vertices[j]);

//             // Create a world matrix
//             current_vertex = matrix_mult_vec4(world_matrix, current_vertex);
//             current_vertex = matrix_mult_vec4(view_matrix, current_vertex);
//             transformed_vertices[j] = current_vertex;
//         }
//         vec3f_t origin = {0, 0, 0};
//         /////////////////
//         // Cull back faces
//         /////////////////
//         vec3f_t cameraRay = vec3_sub(origin, vec3_from_vec4(transformed_vertices[0]));
//         vec3f_t tri_normal = get_triangle_face_normal(transformed_vertices);
//         tri_normal = vec_normalize(tri_normal);

//         float orientation_from_camera = vec_dot(tri_normal, cameraRay);
//         if (cull_mode == CULL_BACKFACE)
//         {
//             if (orientation_from_camera < 0)
//             {
//                 continue;
//             }
//         }

//         ////////////////
//         // Clipping stage
//         ////////////////

//         // Create a polygon from the transformed triangle
//         polygon_t polygon = create_polygon_from_triangle(
//             vec3_from_vec4(transformed_vertices[0]),
//             vec3_from_vec4(transformed_vertices[1]),
//             vec3_from_vec4(transformed_vertices[2]),
//             current_face.a_uv,
//             current_face.b_uv,
//             current_face.c_uv);

//         // Clip polygon with the view frustum
//         clip_polygon(&polygon);

//         // // printf("number of vertices after clipping: %d\n", polygon.num_vertices);

//         // Break clipped polygon into triangles to be rendered
//         triangle_t tris_after_clipping[MAX_NUM_POLY_TRIS];
//         int num_triangles_after_clipping = 0;

//         triangles_from_polygon(&polygon, tris_after_clipping, &num_triangles_after_clipping);

//         // Loop through triangles after clipping

//         for (int t = 0; t < num_triangles_after_clipping; t++)
//         {

//             triangle_t triangle_after_clipping = tris_after_clipping[t];

//             vector_t projected_points[3];

//             for (int j = 0; j < 3; j++)
//             {
//                 projected_points[j] = mat4_mul_vec4_project(projection_matrix, triangle_after_clipping.points[j]);

//                 // Scale to middle of screen
//                 projected_points[j].x *= get_window_width() * 0.5f;
//                 projected_points[j].y *= get_window_height() * 0.5f;

//                 // Invert Y values to account for flipped y screen coordinate
//                 projected_points[j].y *= -1;

//                 // Translate to middle of screen
//                 projected_points[j].x += get_window_width() * 0.5f;
//                 projected_points[j].y += get_window_height() * 0.5f;
//             }
//             triangle_t triangle_to_render = {
//                 .points = {
//                     {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
//                     {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
//                     {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
//                 },
//                 .texcoords = {{triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v}, {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v}, {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v}},
//                 .texture = mesh->img
//             };
//             if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
//             {
//                 triangles_to_render[num_triangles_to_render] = triangle_to_render;
//                 num_triangles_to_render++;
//             }
//         }
//     }

// }