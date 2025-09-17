#include <math.h>
#include "../include/vector.h"
#include <shz_vector.h>

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector 3D functions
///////////////////////////////////////////////////////////////////////////////
shz_vec3_t vec3_new(float x, float y, float z){
    return shz_vec3_init(x,y,z);
}

 shz_vec3_t vec3_rotate_x(shz_vec3_t v, float angle) {
    vec3f_t rv = vec_rotd_yz((vec3f_t){v.x,v.y,v.z}, (vec3f_t){ 0, 0, 0 }, angle);
    return (shz_vec3_t){rv.x,rv.y,rv.z};
}

shz_vec3_t vec3_rotate_y(shz_vec3_t v, float angle) {
    vec3f_t rv = vec_rotd_xz((vec3f_t){v.x,v.y,v.z}, (vec3f_t){ 0, 0, 0 }, angle);
    return (shz_vec3_t){rv.x,rv.y,rv.z};
}

shz_vec3_t vec3_rotate_z(shz_vec3_t v, float angle) {
    vec3f_t rv = vec_rotd_xy((vec3f_t){v.x,v.y,v.z}, (vec3f_t){ 0, 0, 0 }, angle);
    return (shz_vec3_t){rv.x,rv.y,rv.z};
}

///////////////////////////////////////////////////////////////////////////////
// Implementations of Vector conversion functions
///////////////////////////////////////////////////////////////////////////////
shz_vec4_t vec4_from_vec3(shz_vec3_t v) {
    shz_vec4_t result = shz_vec4_init( v.x, v.y, v.z, 1.0 );
    return result;
}

shz_vec3_t vec3_from_vec4(shz_vec4_t v) {
    shz_vec3_t result = shz_vec3_init( v.x, v.y, v.z );
    return result;
}

shz_vec2_t vec2_from_vec4(shz_vec4_t v) {
    shz_vec2_t result = shz_vec2_init( v.x, v.y );
    return result;
}