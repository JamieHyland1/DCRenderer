
#include <stdio.h>
#include "vector.h"
#include "display.h"
#include "mesh.h"
#include "triangle.h"
#include <png/png.h>
#include <stdio.h>
#include <malloc.h>
#include <kos/img.h>
#define PX_PER_LINE 640 // Dreamcast native resolution width
int frame_count = 0;
#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];
triangle_t triangles_to_render[100];
int triangle_count = 0;
uint64_t start_time = 0, end_time = 0;
float fps = 0.0f;
float fov_factor = 640.0f;
vec3_t camera_position = {.x = 0, .y = 0, .z = 0};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};
int projected_point_count = 0;
kos_img_t img;
  uint16_t* pixels;
bool is_running = true;
uint32_t cube_vertex_colors[N_CUBE_VERTICES] = {
    0xFF0000, // Red
    0x00FF00, // Green
    0x0000FF, // Blue
    0xFFFF00, // Yellow
    0xFF00FF, // Magenta
    0x00FFFF, // Cyan
    0xFFFFFF, // White
    0x000000 // Black
};

bool setup(){
    vid_set_mode(DM_640x480  | DM_MULTIBUFFER, PM_RGB565);
    vid_mode->fb_count = 2;
    int point_count = 0;
     for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = { .x = x, .y = y, .z = z };
               // new_point.z 
                cube_points[point_count++] = new_point;
            }
        }
    }


    png_to_img("rd/Anya.png", 0, &img);
    if (!img.data) {
        printf("Failed to load image\n");
        return false;
    }

    pixels = (uint16_t*)malloc(512 * 512 * sizeof(uint16_t));
 
    return true;
}

void project(vec3_t* point, vec2_t* result) {
    if(point->z <= 0.1f) {
        result->x = 0;
        result->y = 0;
        return; // Avoid division by zero
    }
    result->x = (fov_factor * point->x) / point->z; // Assuming 640x480
    result->y = (fov_factor * point->y) / point->z;
}

void update(){
    triangle_count = 0; // Reset triangle count for each frame
    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if(cont) {
        cont_state_t *state = (cont_state_t *)maple_dev_status(cont);

        if(!state)
            return;

        cube_rotation.y = fmodf(cube_rotation.y - 0.01f, 2.0f * M_PI);
        cube_rotation.x = fmodf(cube_rotation.x + 0.01f, 2.0f * M_PI);

    }
    for(int i = 0; i < N_CUBE_FACES; i ++) {
        face_t current_face = mesh_faces[i];
        vec3_t face_verts[3];

        // Get and rotate each vertex
        for (int j = 0; j < 3; j++) {
            vec3_t v = mesh_vertices[(&current_face.a)[j] - 1];
            vec3_t temp;
            vec3_rotate_x(&v, cube_rotation.x, &temp);
            vec3_rotate_y(&temp, cube_rotation.y, &v);

            // Translate by camera position
            v.z += 5;
            
            v.x -= camera_position.x;
            v.y -= camera_position.y;
            v.z -= camera_position.z;


            face_verts[j] = v;
        }

        vec3_t a = face_verts[0];
        vec3_t b = face_verts[1];
        vec3_t c = face_verts[2];

        vec3_t ab, ac, normal;
        vec3_sub(&b, &a, &ab);
        vec3_sub(&c, &a, &ac);
        vec3_cross(&ab, &ac, &normal);
        vec3_normalize(&normal);

        // Backface culling: use camera ray
        vec3_t camera_ray;
        vec3_sub(&(vec3_t){0,0,0}, &a, &camera_ray);
        if (vec3_dot(&normal, &camera_ray) <= 0) {
            continue;
        }

        // Project triangle
        triangle_t triangle;
        for (int j = 0; j < 3; j++) {
            vec2_t projected;
            project(&face_verts[j], &projected);
            if (projected.x == 0 && projected.y == 0) {
                break;
            }
            triangle.points[j] = projected;
        }
        if (triangle_count < 100) {
            triangles_to_render[triangle_count++] = triangle;
        }
    }
}



void draw_image(int dst_x, int dst_y, int width, int height, const uint16_t *pixels) {
    for (int y = 0; y < height; y++) {
        int screen_y = dst_y + y;
        if (screen_y < 0 || screen_y >= 480) continue;

        for (int x = 0; x < width; x++) {
            int screen_x = dst_x + x;
            if (screen_x < 0 || screen_x >= PX_PER_LINE) continue;

            uint16_t color = pixels[y * width + x];
            draw_pixel(screen_x, screen_y, color);
        }
    }
}



void render(){
    vid_clear(25, 25, 25);  // Clear i

    draw_image(0, 0, 512, 512, pixels);
    // minifont_draw_str(vram_s + get_offset(20,10), 640, "Hello World, from Jamies Renderer!");

    // char status_str[64];
    // snprintf(status_str, sizeof(status_str), "is_running: %s", is_running ? "true" : "false");
    // minifont_draw_str(vram_s + get_offset(20, 30), 640, status_str);
    // for(int i = 0; i < triangle_count; i++) {
    //     triangle_t triangle = triangles_to_render[i];



    //     // Draw the triangle
    //     // draw_filled_triangle(
    //     //     (int)triangle.points[0].x + 320, (int)triangle.points[0].y + 240, 0.0f, 1.0f,
    //     //     (int)triangle.points[1].x + 320, (int)triangle.points[1].y + 240, 0.0f, 1.0f,
    //     //     (int)triangle.points[2].x + 320, (int)triangle.points[2].y + 240, 0.0f, 1.0f,
    //     //     cube_vertex_colors[i % N_CUBE_VERTICES]);



    //     draw_triangle(
    //         (int)triangle.points[0].x + 320, (int)triangle.points[0].y + 240,
    //         (int)triangle.points[1].x + 320, (int)triangle.points[1].y + 240,
    //         (int)triangle.points[2].x + 320, (int)triangle.points[2].y + 240,
    //         0xFFFFFF); // White color for the outline
        
    // }
}


int main(int argc, char **argv) {
    is_running = setup();
    while(1) {
        vid_waitvbl();
        update();
        render();
        vid_flip(frame_count%2);
        frame_count++;
    }

    return 0;
}
