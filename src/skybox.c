#include "../include/skybox.h"
#include "shz_vector.h"
skybox_t skybox;
shz_vec2_t uvs[5][480][640]; 

void init_skybox(){
    png_to_img("rd/front.png",  0, &skybox.negz);
    png_to_img("rd/back.png",   0, &skybox.posz);
    png_to_img("rd/top.png",    0, &skybox.posy);
    png_to_img("rd/bottom.png", 0, &skybox.negy);
    png_to_img("rd/left.png",   0, &skybox.negx);
    png_to_img("rd/right.png",  0, &skybox.posx);
}

void compute_skybox_uvs(){
   
}

void draw_skybox(){
 
}