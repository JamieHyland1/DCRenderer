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
#include "profile.h"
#include "tile.h"

enum cull_method
{
    CULL_NONE,
    CULL_BACKFACE
};

enum draw_method
{
    DRAW_WIRE,
    DRAW_FILL_TRIANGLE,
    DRAW_FILL_TRIANGLE_WIRE,
    DRAW_TEXTURED,
    DRAW_TEXTURED_SCANLINE
};
#endif
