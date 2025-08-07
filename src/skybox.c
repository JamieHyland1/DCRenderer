#include "../include/skybox.h"

skybox_t skybox;
vec3f_t ray_dirs[480][640]; 

void init_skybox(){
    png_to_img("rd/front.png",  0, &skybox.negz);
    png_to_img("rd/back.png",   0, &skybox.posz);
    png_to_img("rd/top.png",    0, &skybox.posy);
    png_to_img("rd/bottom.png", 0, &skybox.negy);
    png_to_img("rd/left.png",   0, &skybox.negx);
    png_to_img("rd/right.png",  0, &skybox.posx);
}

void compute_ray(){
   float aspect_ratio = 640.0f / 480.0f;
    float fov = 90.0f * (M_PI / 180.0f); // radians
    float half_height = tanf(fov * 0.5f);
    float half_width = aspect_ratio * half_height;

    for(int y = 0; y < 480; y++){
        float ndc_y = 1.0f - 2.0f * ((y + 0.5f) / 480.0f); // NDC Y
        for(int x = 0; x < 640; x++){
            float ndc_x = 2.0f * ((x + 0.5f) / 640.0f) - 1.0f; // NDC X

            vec3f_t dir;
            dir.x = ndc_x * half_width;
            dir.y = ndc_y * half_height;
            dir.z = -1.0f; // Forward (right-handed)

            // Normalize
            float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
            ray_dirs[y][x].x = dir.x / len;
            ray_dirs[y][x].y = dir.y / len;
            ray_dirs[y][x].z = dir.z / len;
        }
    }
}

void draw_skybox(){
 for(int y = 0; y < 480; y++){
        for(int x = 0; x < 640; x++){
            vec3f_t dir = ray_dirs[y][x];
            float abs_x = fabsf(dir.x);
            float abs_y = fabsf(dir.y);
            float abs_z = fabsf(dir.z);

            float u, v;
            kos_img_t* face;

            // Identify which face of the cube to use
            if(abs_x >= abs_y && abs_x >= abs_z){
                if(dir.x > 0){
                    face = &skybox.posx;
                    u = -dir.z / abs_x;
                    v = -dir.y / abs_x;
                } else {
                    face = &skybox.negx;
                    u = dir.z / abs_x;
                    v = -dir.y / abs_x;
                }
            } else if(abs_y >= abs_x && abs_y >= abs_z){
                if(dir.y > 0){
                    face = &skybox.posy;
                    u = dir.x / abs_y;
                    v = dir.z / abs_y;
                } else {
                    face = &skybox.negy;
                    u = dir.x / abs_y;
                    v = -dir.z / abs_y;
                }
            } else {
                if(dir.z > 0){
                    face = &skybox.posz;
                    u = dir.x / abs_z;
                    v = -dir.y / abs_z;
                } else {
                    face = &skybox.negz;
                    u = -dir.x / abs_z;
                    v = -dir.y / abs_z;
                }
            }

            // Convert from [-1,1] to [0,1]
            u = (u + 1.0f) * 0.5f;
            v = (v + 1.0f) * 0.5f;

            int tex_x = (int)(u * (face->w - 1));
            int tex_y = (int)(v * (face->h - 1));

            // Sample the texture (assuming 16-bit RGB565)
            uint16_t* data = (uint16_t*)face->data;
            uint16_t color = data[tex_y * face->w + tex_x];

            // Draw directly to framebuffer (or use your draw_pixel)
            draw_pixel(x, y, color);

            // Optionally skip Z buffer update (skybox should be infinitely far)
        }
    }
}