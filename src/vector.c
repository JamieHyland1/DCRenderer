#include <math.h>
#include "../include/vector.h"

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector 2D functions
///////////////////////////////////////////////////////////////////////////////

vec2_t vec2_new(float x, float y){
    return (vec2_t){x,y};
}

float vec2_length(vec2_t v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

vec2_t vec2_add(vec2_t a, vec2_t b) {
    vec2_t result = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return result;
}

vec2_t vec2_sub(vec2_t a, vec2_t b) {
    vec2_t result = {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
    return result;
}

vec2_t vec2_mul(vec2_t v, float factor) {
    vec2_t result = {
        .x = v.x * factor,
        .y = v.y * factor
    };
    return result;
}

vec2_t vec2_div(vec2_t v, float factor) {
    vec2_t result = {
        .x = v.x / factor,
        .y = v.y / factor
    };
    return result;
}

float vec2_dot(vec2_t a, vec2_t b) {
    return (a.x * b.x) + (a.y * b.y);
}

void vec2_normalize(vec2_t* v) {
    float length = sqrt(v->x * v->x + v->y * v->y);
    v->x /= length;
    v->y /= length;
}

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector 3D functions
///////////////////////////////////////////////////////////////////////////////
vec3f_t vec3_new(float x, float y, float z){
    return (vec3f_t){x,y,z};
}




vec3f_t vec3_add(vec3f_t a, vec3f_t b) {
    vec3f_t result = {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
    return result;
}

vec3f_t vec3_sub(vec3f_t a, vec3f_t b) {
    vec3f_t result = {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
    return result;
}

vec3f_t vec3_mult(vec3f_t v, float factor) {
    vec3f_t result = {
        .x = v.x * factor,
        .y = v.y * factor,
        .z = v.z * factor
    };
    return result;
}

vec3f_t vec3_div(vec3f_t v, float factor) {
    vec3f_t result = {
        .x = v.x / factor,
        .y = v.y / factor,
        .z = v.z / factor
    };
    return result;
}

vec3f_t vec3_cross(vec3f_t a, vec3f_t b) {
    vec3f_t result = {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
    };
    return result;
}

vec3f_t vec3_rotate_x(vec3f_t v, float angle) {
    vec3f_t rotated_vector = vec_rotd_yz(v, (vec3f_t){ 0, 0, 0 }, angle);
    return rotated_vector;
}

vec3f_t vec3_rotate_y(vec3f_t v, float angle) {
    vec3f_t rotated_vector = vec_rotd_xz(v, (vec3f_t){ 0, 0, 0 }, angle);
    return rotated_vector;
}

vec3f_t vec3_rotate_z(vec3f_t v, float angle) {
    vec3f_t rotated_vector = vec_rotd_xy(v, (vec3f_t){ 0, 0, 0 }, angle);
    return rotated_vector;
}

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector conversion functions
///////////////////////////////////////////////////////////////////////////////
vector_t vec4_from_vec3(vec3f_t v) {
    vector_t result = { v.x, v.y, v.z, 1.0 };
    return result;
}

vec3f_t vec3_from_vec4(vector_t v) {
    vec3f_t result = { v.x, v.y, v.z };
    return result;
}

vec2_t vec2_from_vec4(vector_t v) {
    vec2_t result = { v.x, v.y };
    return result;
}