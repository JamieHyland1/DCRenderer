#ifndef vector_h
#define vector_h
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <C:\SDL2\include\SDL.h>
#include <dc/vector.h>
#include <dc/vec3f.h>
typedef struct{
    float x;
    float y;
}vec2_t;




vec3f_t vec3_rotate_x(vec3f_t v, float angle);
vec3f_t vec3_rotate_y(vec3f_t v, float angle);
vec3f_t vec3_rotate_z(vec3f_t v, float angle);

/////////////////////////
/// Vector 2D functions 
/////////////////////////
vec2_t vec2_new(float x, float y);
float  vec2_length(vec2_t v);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);
vec2_t vec2_mult(vec2_t v, float f);
vec2_t vec2_div(vec2_t v,  float f);
void vec2_normalize(vec2_t* a);
float vec2_dot(vec2_t a, vec2_t b);

/////////////////////////
/// Vector 3D functions 
/////////////////////////
vec3f_t vec3_new(float x, float y, float z);
vec3f_t vec3_add(vec3f_t a, vec3f_t b);
vec3f_t vec3_sub(vec3f_t a, vec3f_t b);
vec3f_t vec3_mult(vec3f_t v, float f);
vec3f_t vec3_div(vec3f_t v,  float f);
vec3f_t vec3_cross(vec3f_t a, vec3f_t b);
vec3f_t vec3_from_vec4(vector_t v);


/////////////////////////
/// Vector 4D functions 
/////////////////////////
vector_t vec4_from_vec3(vec3f_t v);
vec2_t vec2_from_vec4(vector_t v);
#endif