#include <stdio.h>
#include <stdint.h>
#include "../include/texture.h"

tex2_t new_tex2(float u, float v){
    tex2_t result;
    result.u = u;
    result.v = v;

    return result;
}