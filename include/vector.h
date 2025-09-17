#ifndef vector_h
#define vector_h
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <C:\SDL2\include\SDL.h>
#include <dc/vector.h>
#include <dc/vec3f.h>
#include "shz_vector.h"


typedef struct{
    int x;
    int y;
}vec2i_t;

typedef struct {
    int v[4];
} Vec4i;


/////////////////////////
/// Vector 2D functions 
/////////////////////////
shz_vec2_t vec2_new(float x, float y);
/////////////////////////
/// Vector 3D functions 
/////////////////////////
shz_vec3_t vec3_new(float x, float y, float z);
shz_vec3_t vec3_from_vec4(shz_vec4_t v);
/////////////////////////
/// Vector 4D functions 
/////////////////////////
shz_vec4_t vec4_from_vec3(shz_vec3_t v);
shz_vec2_t vec2_from_vec4(shz_vec4_t v);
#endif