#ifndef light_h
#define light_h
#include "vector.h"

#define NUM_POINT_LIGHTS 4

typedef struct{
    shz_vec3_t direction;
    float intensity;

}light_t;

typedef struct {
    shz_vec3_t position;
    uint16_t color; 
} point_light_t;

extern point_light_t point_lights[NUM_POINT_LIGHTS];


void init_light(shz_vec3_t direction);
shz_vec3_t get_light_direction(void);
uint16_t light_apply_intensity(uint16_t color, float percentage_factor);

#endif