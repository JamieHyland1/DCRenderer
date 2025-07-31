#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <dc/perf_monitor.h>
#include "../include/triangle.h"
#include "../include/display.h"
#include "../include/swap.h"
#include "../include/utils.h"

#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

#define SWAP(v1, v2, type) { \
type temp = v2; \
v2 = v1; \
v1 = temp; \
}

inline int orient2d(vec2i_t* a, vec2i_t* b, vec2i_t* c)
{
    return ((b->x-a->x)*(c->y-a->y) - (b->y-a->y)*(c->x-a->x));
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

void draw_filled_triangle(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, uint16_t color){
    int minx = v0->x < v1->x ? (v0->x < v2->x ? v0->x : v2->x) : (v1->x < v2->x ? v1->x : v2->x);
    int miny = v0->y < v1->y ? (v0->y < v2->y ? v0->y : v2->y) : (v1->y < v2->y ? v1->y : v2->y);
    int maxx = v0->x > v1->x ? (v0->x > v2->x ? v0->x : v2->x) : (v1->x > v2->x ? v1->x : v2->x);
    int maxy = v0->y > v1->y ? (v0->y > v2->y ? v0->y : v2->y) : (v1->y > v2->y ? v1->y : v2->y);

    int A01 = v0->y - v1->y, B01 = v1->x - v0->x;
    int A12 = v1->y - v2->y, B12 = v2->x - v1->x;
    int A20 = v2->y - v0->y, B20 = v0->x - v2->x;

    vec2i_t p = {minx, miny};

    int w0_row = orient2d(v1, v2, &p);
    int w1_row = orient2d(v2, v0, &p);
    int w2_row = orient2d(v0, v1, &p);


    for(p.y = miny; p.y <= maxy; ++p.y) {

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;
        for(p.x = minx; p.x <= maxx; ++p.x) {

            if ((w0 | w1 | w2) >= 0){
                draw_pixel(p.x,     p.y, color);
            }

            w0 += (A12);
            w1 += (A20);
            w2 += (A01);
        }

        w0_row += (B12);
        w1_row += (B20);
        w2_row += (B01);
    } 
}

void draw_filled_triangle_wire(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, uint16_t color){
    int minx = v0->x < v1->x ? (v0->x < v2->x ? v0->x : v2->x) : (v1->x < v2->x ? v1->x : v2->x);
    int miny = v0->y < v1->y ? (v0->y < v2->y ? v0->y : v2->y) : (v1->y < v2->y ? v1->y : v2->y);
    int maxx = v0->x > v1->x ? (v0->x > v2->x ? v0->x : v2->x) : (v1->x > v2->x ? v1->x : v2->x);
    int maxy = v0->y > v1->y ? (v0->y > v2->y ? v0->y : v2->y) : (v1->y > v2->y ? v1->y : v2->y);

    int A01 = v0->y - v1->y, B01 = v1->x - v0->x;
    int A12 = v1->y - v2->y, B12 = v2->x - v1->x;
    int A20 = v2->y - v0->y, B20 = v0->x - v2->x;

    vec2i_t p = {minx, miny};

    int w0_row = orient2d(v1, v2, &p);
    int w1_row = orient2d(v2, v0, &p);
    int w2_row = orient2d(v0, v1, &p);


    for(p.y = miny; p.y <= maxy; ++p.y) {

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;
        for(p.x = minx; p.x <= maxx; ++p.x) {
            int inside = (w0 | w1 | w2) >= 0;
            int edge_thresh = 75; 

            int on_edge =
                (w0 >= -edge_thresh && w0 <= edge_thresh) ||
                (w1 >= -edge_thresh && w1 <= edge_thresh) ||
                (w2 >= -edge_thresh && w2 <= edge_thresh);

            if (inside && on_edge){
                draw_pixel(p.x, p.y, 0XFFFF); // Draw wireframe pixel
            }
            else if(inside){
                draw_pixel(p.x, p.y, color); // Draw filled pixel
            }

            w0 += (A12);
            w1 += (A20);
            w2 += (A01);
        }

        w0_row += (B12);
        w1_row += (B20);
        w2_row += (B01);
    }
}

void draw_textured_triangle(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, vec2_t* uv0, vec2_t* uv1, vec2_t* uv2, kos_img_t texture){
    uint16_t* tex_data = (uint16_t*)texture.data;
    int tex_width = texture.w;
    int tex_height = texture.h;

    int minx = v0->x < v1->x ? (v0->x < v2->x ? v0->x : v2->x) : (v1->x < v2->x ? v1->x : v2->x);
    int miny = v0->y < v1->y ? (v0->y < v2->y ? v0->y : v2->y) : (v1->y < v2->y ? v1->y : v2->y);
    int maxx = v0->x > v1->x ? (v0->x > v2->x ? v0->x : v2->x) : (v1->x > v2->x ? v1->x : v2->x);
    int maxy = v0->y > v1->y ? (v0->y > v2->y ? v0->y : v2->y) : (v1->y > v2->y ? v1->y : v2->y);

    int A01 = v0->y - v1->y, B01 = v1->x - v0->x;
    int A12 = v1->y - v2->y, B12 = v2->x - v1->x;
    int A20 = v2->y - v0->y, B20 = v0->x - v2->x;

    vec2i_t p = {minx, miny};

    int w0_row = orient2d(v1, v2, &p);
    int w1_row = orient2d(v2, v0, &p);
    int w2_row = orient2d(v0, v1, &p);
    int area   = orient2d(v0, v1, v2);

    for(p.y = miny; p.y <= maxy; ++p.y) {

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;
        for(p.x = minx; p.x <= maxx; p.x += 2) {

            if ((w0 | w1 | w2) >= 0){
                float b0 = (float)w0 / (float)area;
                float b1 = (float)w1 / (float)area;
                float b2 = (float)w2 / (float)area;

                float u = b0 * uv0->x + b1 * uv1->x + b2 * uv2->x;
                float v = b0 * uv0->y + b1 * uv1->y + b2 * uv2->y;

                u = u < 0.0f ? 0.0f : (u > 1.0f ? 1.0f : u);
                v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);


                int tex_x = (int)(u * (tex_width - 1));
                int tex_y = (int)((1.0f - v) * (tex_height - 1));

                uint16_t color = tex_data[tex_y * tex_width + tex_x];
                draw_pixel(p.x, p.y, color);
            }
            int w0_next = w0 + A12, w1_next = w1 + A20, w2_next = w2 + A01;

            if ((w0_next | w1_next | w2_next) >= 0) {
                float b0 = (float)w0_next / (float)area;
                float b1 = (float)w1_next / (float)area;
                float b2 = (float)w2_next / (float)area;

                float u = b0 * uv0->x + b1 * uv1->x + b2 * uv2->x;
                float v = b0 * uv0->y + b1 * uv1->y + b2 * uv2->y;

                u = u < 0.0f ? 0.0f : (u > 1.0f ? 1.0f : u);
                v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);


                int tex_x = (int)(u * (tex_width - 1));
                int tex_y = (int)((1.0f - v) * (tex_height - 1));

                uint16_t color = tex_data[tex_y * tex_width + tex_x];
                draw_pixel(p.x+1, p.y, color);
            }
            w0 = w0_next + A12;
            w1 = w1_next + A20;
            w2 = w2_next + A01;
        }

        w0_row += (B12);
        w1_row += (B20);
        w2_row += (B01);
    }
}

vec3f_t get_triangle_face_normal(vector_t vertices[3]){
    vec3f_t v1 = vec3_sub(vec3_from_vec4(vertices[1]),vec3_from_vec4(vertices[0]));
    vec3f_t v2 = vec3_sub(vec3_from_vec4(vertices[2]),vec3_from_vec4(vertices[0]));
    vec3f_t tri_normal = vec3_cross(v1,v2);

    return tri_normal;
}
