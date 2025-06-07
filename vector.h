#ifndef vector_h
#define vector_h
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct{
    float x;
    float y;
}vec2_t;

typedef struct{
    float x;
    float y;
    float z;
}vec3_t;

typedef struct{
    float x;
    float y;
    float z;
    float w;
}vec4_t;


vec3_t vec3_rotate_x(vec3_t v, float angle);
vec3_t vec3_rotate_y(vec3_t v, float angle);
vec3_t vec3_rotate_z(vec3_t v, float angle);

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
vec3_t vec3_new(float x, float y, float z);
float  vec3_length(vec3_t v);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_mult(vec3_t v, float f);
vec3_t vec3_div(vec3_t v,  float f);
void vec3_normalize(vec3_t* a);
vec3_t vec3_cross(vec3_t a, vec3_t b);
vec3_t vec3_from_vec4(vec4_t v);

float vec3_dot(vec3_t a, vec3_t b);


/////////////////////////
/// Vector 4D functions 
/////////////////////////
vec4_t vec4_from_vec3(vec3_t v);
vec2_t vec2_from_vec4(vec4_t v);
#endif