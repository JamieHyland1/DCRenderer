#ifndef SKYBOX_H
#define SKYBOX_H
#include <png/png.h>
#include "vector.h"
#include "display.h"
typedef struct {
    kos_img_t posx; // right
    kos_img_t negx; // left
    kos_img_t posy; // top
    kos_img_t negy; // bottom
    kos_img_t posz; // front
    kos_img_t negz; // back
} skybox_t;

extern skybox_t skybox;

void init_skybox();
void compute_ray();
void draw_skybox();

#endif