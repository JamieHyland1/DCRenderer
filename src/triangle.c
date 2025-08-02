#include <stdlib.h>
#include <time.h>
#include "../include/triangle.h"
#include "../include/display.h"
#include <math.h>
#include <dc/perf_monitor.h>
#include "../include/utils.h"



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
    #ifdef DEBUG_TIME
        perf_monitor();
    #endif
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
        for(p.x = minx; p.x <= maxx; ++p.x) {

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
       
            w0 += A12;//w0_next + A12;
            w1 += A20;
            w2 += A01;
        }

        w0_row += (B12);
        w1_row += (B20);
        w2_row += (B01);
    }
}

void draw_textured_triangle_new(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, vec2_t* uv0, vec2_t* uv1, vec2_t* uv2, kos_img_t texture) {
    #ifdef DEBUG_CYCLES
    perf_cntr_clear(PRFC0);
    perf_cntr_clear(PRFC1);
    perf_cntr_start(PRFC0, PMCR_ELAPSED_TIME_MODE, PMCR_COUNT_CPU_CYCLES);
    perf_cntr_start(PRFC1, PMCR_OPERAND_CACHE_READ_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
    #endif

    uint16_t* tex_data = (uint16_t*)texture.data;
    int tex_width = texture.w;
    int tex_height = texture.h;

    // Ensure tex_data is 32-byte aligned
    // if ((size_t)tex_data % 32 != 0) {
    //     printf("Warning: texture.data not 32-byte aligned: %p\n", (void*)tex_data);
    // }

    // Compute bounding box
    int minx = v0->x < v1->x ? (v0->x < v2->x ? v0->x : v2->x) : (v1->x < v2->x ? v1->x : v2->x);
    int miny = v0->y < v1->y ? (v0->y < v2->y ? v0->y : v2->y) : (v1->y < v2->y ? v1->y : v2->y);
    int maxx = v0->x > v1->x ? (v0->x > v2->x ? v0->x : v2->x) : (v1->x > v2->x ? v1->x : v2->x);
    int maxy = v0->y > v1->y ? (v0->y > v2->y ? v0->y : v2->y) : (v1->y > v2->y ? v1->y : v2->y);

    // Ensure maxx is aligned to 4 pixels for complete 4-pixel groups
    maxx = (maxx + 3) & ~3; // Round up to multiple of 4

    // Edge equations for barycentric coordinates
    int A01 = v0->y - v1->y, B01 = v1->x - v0->x;
    int A12 = v1->y - v2->y, B12 = v2->x - v1->x;
    int A20 = v2->y - v0->y, B20 = v0->x - v2->x;

    vec2i_t p = {minx, miny};

    int w0_row = orient2d(v1, v2, &p);
    int w1_row = orient2d(v2, v0, &p);
    int w2_row = orient2d(v0, v1, &p);
    int area = orient2d(v0, v1, v2);

    for (p.y = miny; p.y <= maxy; ++p.y) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minx; p.x <= maxx; p.x += 4) {
            // Check if the first pixel of the 4-pixel group is inside the triangle
            if ((w0 | w1 | w2) >= 0) {
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

            // Second pixel
            int w0_next1 = w0 + A12;
            int w1_next1 = w1 + A20;
            int w2_next1 = w2 + A01;
            if ((w0_next1 | w1_next1 | w2_next1) >= 0) {
                float b0 = (float)w0_next1 / (float)area;
                float b1 = (float)w1_next1 / (float)area;
                float b2 = (float)w2_next1 / (float)area;

                float u = b0 * uv0->x + b1 * uv1->x + b2 * uv2->x;
                float v = b0 * uv0->y + b1 * uv1->y + b2 * uv2->y;

                u = u < 0.0f ? 0.0f : (u > 1.0f ? 1.0f : u);
                v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);

                int tex_x = (int)(u * (tex_width - 1));
                int tex_y = (int)((1.0f - v) * (tex_height - 1));

                uint16_t color = tex_data[tex_y * tex_width + tex_x];
                draw_pixel(p.x + 1, p.y, color);
            }

            // Third pixel
            int w0_next2 = w0_next1 + A12;
            int w1_next2 = w1_next1 + A20;
            int w2_next2 = w2_next1 + A01;
            if ((w0_next2 | w1_next2 | w2_next2) >= 0) {
                float b0 = (float)w0_next2 / (float)area;
                float b1 = (float)w1_next2 / (float)area;
                float b2 = (float)w2_next2 / (float)area;

                float u = b0 * uv0->x + b1 * uv1->x + b2 * uv2->x;
                float v = b0 * uv0->y + b1 * uv1->y + b2 * uv2->y;

                u = u < 0.0f ? 0.0f : (u > 1.0f ? 1.0f : u);
                v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);

                int tex_x = (int)(u * (tex_width - 1));
                int tex_y = (int)((1.0f - v) * (tex_height - 1));

                uint16_t color = tex_data[tex_y * tex_width + tex_x];
                draw_pixel(p.x + 2, p.y, color);
            }

            // Fourth pixel
            int w0_next3 = w0_next2 + A12;
            int w1_next3 = w1_next2 + A20;
            int w2_next3 = w2_next2 + A01;
            if ((w0_next3 | w1_next3 | w2_next3) >= 0) {
                float b0 = (float)w0_next3 / (float)area;
                float b1 = (float)w1_next3 / (float)area;
                float b2 = (float)w2_next3 / (float)area;

                float u = b0 * uv0->x + b1 * uv1->x + b2 * uv2->x;
                float v = b0 * uv0->y + b1 * uv1->y + b2 * uv2->y;

                u = u < 0.0f ? 0.0f : (u > 1.0f ? 1.0f : u);
                v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);

                int tex_x = (int)(u * (tex_width - 1));
                int tex_y = (int)((1.0f - v) * (tex_height - 1));

                uint16_t color = tex_data[tex_y * tex_width + tex_x];
                draw_pixel(p.x + 3, p.y, color);
            }

            // Update for next 4-pixel group
            w0 = w0_next3 + A12;
            w1 = w1_next3 + A20;
            w2 = w2_next3 + A01;
        }

        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
    }

    #ifdef DEBUG_CYCLES
    perf_cntr_stop(PRFC0);
    perf_cntr_stop(PRFC1);
    uint64_t cycles = perf_cntr_count(PRFC0);
    uint64_t read_misses = perf_cntr_count(PRFC1);
    printf("Draw Textured Triangle (4-pixel): %llu cycles, %llu read misses\n", cycles, read_misses);
    #endif
}

vec3f_t get_triangle_face_normal(vector_t vertices[3]){
    vec3f_t v1 = vec3_sub(vec3_from_vec4(vertices[1]),vec3_from_vec4(vertices[0]));
    vec3f_t v2 = vec3_sub(vec3_from_vec4(vertices[2]),vec3_from_vec4(vertices[0]));
    vec3f_t tri_normal = vec3_cross(v1,v2);

    return tri_normal;
}
