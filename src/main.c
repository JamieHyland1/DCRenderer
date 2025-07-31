#include <kos.h>
#include "../include/scene.h"
#include "../include/input.h"
#include "../include/display.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
// This is a port of the Pikuma 3D Renderer to the Dreamcast
// It is a simple 3D renderer that uses a software rasterizer to render 3D models
// Written in C for the KallistiOS library 
// The renderer uses a simple graphics pipeline to render 3D models
// It supports basic features such as perspective projection, backface culling, and texture mapping
// It is ported by me :) Jamie 
////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *args[])
{
    isRunning = init_sceen();

    while (isRunning)
    {
        vid_waitvbl();
        process_input();
        update();
        render();
        vid_flip(vid_mode->fb_count);
        frame_count++;
       
    }
    return 0;
}
