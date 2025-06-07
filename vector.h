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

/////////////////////////
/// Vector 2D functions 
/////////////////////////
vec2_t vec2_new(float x, float y);
float  vec2_length(vec2_t* v);
void vec2_add(vec2_t* a, vec2_t* b, vec2_t* dest);
void vec2_sub(vec2_t* a, vec2_t* b, vec2_t* dest);
void vec2_mult(vec2_t* v, float f, vec2_t* dest);
void vec2_div(vec2_t* v,  float f, vec2_t* dest);
float vec2_dot(vec2_t* a, vec2_t* b);
void vec2_normalize(vec2_t* a);


/////////////////////////
/// Vector 3D functions 
/////////////////////////
vec3_t vec3_new(float x, float y, float z);
float  vec3_length(vec3_t* v);
void vec3_add(vec3_t* a, vec3_t* b, vec3_t* dest);
void vec3_sub(vec3_t* a, vec3_t* b, vec3_t* dest);
void vec3_mult(vec3_t* v, float f, vec3_t* dest);
void vec3_div(vec3_t* v,  float f, vec3_t* dest);
void vec3_cross(vec3_t* a, vec3_t* b, vec3_t* dest);
float vec3_dot(vec3_t* a, vec3_t* b);
void vec3_normalize(vec3_t* a);
void vec3_rotate_x(vec3_t* v, float angle, vec3_t* dest);
void vec3_rotate_y(vec3_t* v, float angle, vec3_t* dest);
void vec3_rotate_z(vec3_t* v, float angle, vec3_t* dest);



/////////////////////////
/// Vector 4D functions 
/////////////////////////
void vec2_from_vec4(vec4_t* v, vec2_t* dest);
void vec3_from_vec4(vec4_t* v, vec3_t* dest);
void vec4_from_vec3(vec3_t* v, vec4_t* dest);
#endif