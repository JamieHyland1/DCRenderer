#include "../include/matrix.h"
#include <math.h>
#include "../include/vector.h"
mat4_t mat4_identity(){
    mat4_t I = {{
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1}
    }};

    return I;
}

mat4_t mat4_make_scale(float sx, float sy, float sz){
    // swap the values in the identity matrix with our scale values 
    mat4_t scale_m = mat4_identity();
    scale_m.m[0][0] = sx;
    scale_m.m[1][1] = sy;
    scale_m.m[2][2] = sz;

    return scale_m;
}

mat4_t mat4_make_translation(float tx, float ty, float tz){
    // swap the values in the identity matrix with our scale values 
    mat4_t translation_m = mat4_identity();
    translation_m.m[0][3] = tx;
    translation_m.m[1][3] = ty;
    translation_m.m[2][3] = tz;

    return translation_m;
}

mat4_t matrix_mult_mat4(mat4_t a, mat4_t b){
    mat4_t result;
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j ++){
            result.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
        }
    }

    return result;
}

vector_t matrix_mult_vec4(mat4_t m, vector_t v){
    vector_t result;
    result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
    result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
    result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
    result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;

    return result;
}
// Rotate around Z axis
// | 1    0       0     0 |   | x |
// | 0  cosf(a) -sinf(a)  0 |   | y |
// | 0  sinf(a)  cosf(a)  0 | * | z |
// | 0    0       0     1 |   | 1 |

mat4_t mat4_look_at(vec3f_t eye, vec3f_t target, vec3f_t up){
    //get the vector from the target to the camera
    vec3f_t z = vec3_sub(target,eye); // forward vector
    z = vec_normalize(z);
    vec3f_t x = vec3_cross(up,z); // right vector
    x = vec_normalize(x);
    vec3f_t y = vec3_cross(z,x); // up vector (according to camera rotation)

    mat4_t view_matrix = {{
        {x.x,x.y,x.z, -vec_dot(x,eye)},
        {y.x,y.y,y.z, -vec_dot(y,eye)},
        {z.x,z.y,z.z, -vec_dot(z,eye)},
        {0,   0,  0,             1    },
    }};

    return view_matrix;

}

mat4_t mat4_rotate_z(float angle){
    mat4_t m = mat4_identity();

    m.m[0][0] = cosf(angle);
    m.m[0][1] = -sinf(angle);
    m.m[1][0] = sinf(angle);
    m.m[1][1] = cosf(angle);

    return m;

}


// Rotation around X axis
// | 1    0       0     0 |   | x |
// | 0  cosf(a) -sinf(a)  0 |   | y | 
// | 0  sinf(a)  cosf(a)  0 | * | z |
// | 0    0       0     1 |   | 1 |

mat4_t mat4_rotate_x(float angle){
    mat4_t m = mat4_identity();

    m.m[1][1] =  cosf(angle);
    m.m[1][2] = -sinf(angle);
    m.m[2][1] =  sinf(angle);
    m.m[2][2] =  cosf(angle);

    return m;

}

// Rotation around Y axis
// |  cosf(a)  0  sinf(a)  0 |   | x |
// |    0     1    0     0 |   | y |
// | -sinf(a)  0  cosf(a)  0 | * | z |
// |    0     0    0     1 |   | 1 |
 
mat4_t mat4_rotate_y(float angle){
    mat4_t m = mat4_identity();

    m.m[0][0] =  cosf(angle);
    m.m[0][2] =  sinf(angle);
    m.m[2][0] = -sinf(angle);
    m.m[2][2] =  cosf(angle);

    return m;

}


mat4_t mat4_make_perspective(float fov, float aspect, float znear, float zfar){
    mat4_t m = {{{ 0 }}};
    m.m[0][0] = aspect * (1 / tan(fov / 2));
    m.m[1][1] = 1 / tan(fov / 2);
    m.m[2][2] = zfar / (zfar - znear);
    m.m[3][3] = (-zfar * znear) / (zfar - znear);
    m.m[3][2] = 1;

    return m;
}

vector_t mat4_mul_vec4_project(mat4_t proj, vector_t v){
    vector_t result = matrix_mult_vec4(proj,v);
    if(result.w != 0.0){
        result.x /= result.w;
        result.y /= result.w;
        result.z /= result.w;
    }
    return result;
}

