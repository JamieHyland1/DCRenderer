#include "../include/scene.h"
#include "../include/pipeline.h"
#include "../include/input.h"
#include "../include/display.h"
#include "../include/mesh.h"
#include <kos.h>

int frame_count = 0;
bool isRunning = false;

bool init_sceen(void){
    vid_set_mode(DM_640x480 | DM_MULTIBUFFER, PM_RGB565);
    vid_mode->fb_count = 4;
    printf("Framebuffer count after vid_set_mode: %d\n", vid_mode->fb_count);
    printf("Vid get start width: %ld\n", vid_get_start(-1));

    render_mode = RENDER_TEXTURED;
    cull_mode = CULL_BACKFACE;
  
    float aspect_x = (float)get_window_width() / (float)get_window_height();
    float aspect_y = (float)get_window_height() / (float)get_window_width();

    float fov_y = M_PI / 3.0;
    float fov_x = atanf(tan(fov_y / 2) * aspect_x) * 2;

    float znear = 0.1;
    float zfar = 100.0;
    projection_matrix = mat4_make_perspective(fov_y, aspect_y, znear, zfar);

    init_frustum_planes(fov_x, fov_y, znear, zfar);
    load_mesh("rd/cube.obj", "rd/cube.png", vec3_new(1, 1, 1), vec3_new(0, 0, 18), vec3_new(0, 0, 0));
    load_mesh("rd/skull.obj", "rd/SamTexture.png", vec3_new(8,8,8), vec3_new(3,0,18), vec3_new(0,0,0));

    return true;
}


void update(void)
{
    num_triangles_to_render = 0;
    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++){
        mesh_t* mesh = get_mesh(mesh_index);
        process_graphics_pipeline(mesh);
    }
}
