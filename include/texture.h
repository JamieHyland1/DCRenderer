#ifndef texture_h
#define texture_h
#include "upng.h"
#include <stdint.h>

typedef struct{
    float u;
    float v;
}tex2_t;

tex2_t new_tex2(float u, float v);
#endif
