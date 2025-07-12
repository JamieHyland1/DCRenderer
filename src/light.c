#include <stdio.h>
#include <stdint.h>
#include "../include/light.h"


static light_t main_light;

void init_light(vec3f_t direction){
    main_light.direction = direction;
}

vec3f_t get_light_direction(){
    return main_light.direction;
}

////////////////////////////////////////////////
// apply color change based off light intensity
////////////////////////////////////////////////

uint32_t light_apply_intensity(uint32_t color, float percentage_factor){
    uint32_t a = (color & 0xFF000000);
    uint32_t r = (color & 0x00FF0000) * percentage_factor;
    uint32_t g = (color & 0x0000FF00) * percentage_factor;
    uint32_t b = (color & 0x000000FF) * percentage_factor;

    uint32_t new_color = a | (r & 0x00FF0000) |  (g & 0x0000FF00) | (b & 0x000000FF);

    return new_color;
}

