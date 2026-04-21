// core.h
#ifndef CORE_H
#define CORE_H

#include "array.h"
#include "debug.h"
#include "vmu.h"
#include "vector.h"
#include "matrix.h"
#include "object.h"
#include "render_target.h"
#include "pipeline.h"
#include "renderer.h"
#include "input.h"
    
enum cull_method
{
    CULL_NONE,
    CULL_BACKFACE
};

enum render_method
{
    RENDER_WIRE,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_SCANLINE
};
#endif
