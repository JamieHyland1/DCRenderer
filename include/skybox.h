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

typedef struct {
    uint8_t face;   // cube face index (0..5)
    uint16_t u_px;  // texture X coordinate in pixels
    uint16_t v_px;  // texture Y coordinate in pixels
} SkyboxSample;

extern skybox_t skybox;

void compute_skybox_uvs();
void draw_skybox();

#endif