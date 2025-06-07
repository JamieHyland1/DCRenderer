#include <math.h>
#include "vector.h"

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector 2D functions
///////////////////////////////////////////////////////////////////////////////

vec2_t vec2_new(float x, float y){
    return (vec2_t){x,y};
}

float vec2_length(vec2_t* v) {
    return sqrt(v->x * v->x + v->y * v->y);
}

void vec2_add(vec2_t* a, vec2_t* b, vec2_t* dest) {
    dest->x = a->x + b->x;
    dest->y = a->y + b->y;
}

void vec2_sub(vec2_t* a, vec2_t* b, vec2_t* dest) {
    dest->x = a->x - b->x;
    dest->y = a->y - b->y;
}

void vec2_mult(vec2_t* v, float factor, vec2_t* dest) {
    dest->x = v->x * factor;
    dest->y = v->y * factor;
}

void vec2_div(vec2_t* v,  float factor, vec2_t* dest) {
    dest->x = v->x / factor;
    dest->y = v->y / factor;
}

float vec2_dot(vec2_t* a, vec2_t* b) {
    return (a->x * b->x) + (a->y * b->y);
}

void vec2_normalize(vec2_t* v) {
    float length = sqrt(v->x * v->x + v->y * v->y);
    v->x /= length;
    v->y /= length;
}

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector 3D functions
///////////////////////////////////////////////////////////////////////////////
vec3_t vec3_new(float x, float y, float z){
    return (vec3_t){x,y,z};
}

float vec3_length(vec3_t* v) {
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

void vec3_add(vec3_t* a, vec3_t* b, vec3_t* dest) {
    dest->x = a->x + b->x;
    dest->y = a->y + b->y;
    dest->z = a->z + b->z;

}

void vec3_sub(vec3_t* a, vec3_t* b, vec3_t* dest){
    dest->x = a->x - b->x;
    dest->y = a->y - b->y;
    dest->z = a->z - b->z;
}

void vec3_mult(vec3_t* v, float factor, vec3_t* dest){
    dest->x = v->x * factor;
    dest->y = v->y * factor;
    dest->z = v->z * factor;
}

void vec3_div(vec3_t* v,  float factor, vec3_t* dest) {
    dest->x = v->x / factor;
    dest->y = v->y / factor;
    dest->z = v->z / factor;
}

void vec3_cross(vec3_t* a, vec3_t* b, vec3_t* dest) {
    dest->x = a->y * b->z - a->z * b->y;
    dest->y = a->z * b->x - a->x * b->z;
    dest->z = a->x * b->y - a->y * b->x;
}

float vec3_dot(vec3_t* a, vec3_t* b) {
    return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

void vec3_normalize(vec3_t* v) {
    float length = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}
void vec3_rotate_x(vec3_t* v, float angle, vec3_t* dest) {
    dest->x = v->x;
    dest->y = v->y * cosf(angle) - v->z * sinf(angle);
    dest->z = v->y * sinf(angle) + v->z * cosf(angle);
}

void vec3_rotate_y(vec3_t* v, float angle, vec3_t* dest) {
    dest->x = v->x * cos(angle) + v->z * sin(angle);
    dest->y = v->y;
    dest->z = -v->x * sin(angle) + v->z * cos(angle);   
}

void vec3_rotate_z(vec3_t* v, float angle, vec3_t* dest) {
    dest->x = v->x * cos(angle) - v->y * sin(angle);
    dest->y = v->x * sin(angle) + v->y * cos(angle);
    dest->z = v->z;
}

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector conversion functions
///////////////////////////////////////////////////////////////////////////////

void vec2_from_vec4(vec4_t* v, vec2_t* dest) {
    dest->x = v->x;
    dest->y = v->y;
}

void vec3_from_vec4(vec4_t* v, vec3_t* dest) {
    dest->x = v->x;
    dest->y = v->y;
    dest->z = v->z;
}

void vec4_from_vec3(vec3_t* v, vec4_t* dest) {
    dest->x = v->x;
    dest->y = v->y;
    dest->z = v->z;
    dest->w = 1.0f;
}