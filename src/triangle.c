#include "../include/renderer.h"



uint16_t* pixel_buffer;

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

    float area   = (float)orient2d(v0, v1, v2);
    float inv_area = 1/area;
    
    int w0_row = orient2d(v1, v2, &p);
    int w1_row = orient2d(v2, v0, &p);
    int w2_row = orient2d(v0, v1, &p);



    for(p.y = miny; p.y <= maxy; ++p.y) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;
        for(p.x = minx; p.x <= maxx; ++p.x) {
            if ((w0 | w1 | w2) >= 0){
                float b0 = (float)w0 * inv_area;
                float b1 = (float)w1 * inv_area;
                float b2 = (float)w2 * inv_area;

                float u = fipr(b0, b1, b2, 0.0f, uv0->x, uv1->x, uv2->x, 0.0f);
                float v =  fipr(b0, b1, b2, 0.0f, uv0->y, uv1->y, uv2->y, 0.0f);

                int tex_x = (int)(u * (tex_width - 1));
                int tex_y = (int)((1.0f - v) * (tex_height - 1));

                uint16_t color = tex_data[tex_y * tex_width + tex_x];
                draw_pixel(p.x, p.y, color);
            }
       
            w0 += A12;
            w1 += A20;
            w2 += A01;
        }

        w0_row += (B12);
        w1_row += (B20);
        w2_row += (B01);
    }
  
}

void draw_textured_triangle_f(vector_t* v0, vector_t* v1, vector_t* v2,
                              vec2_t* uv0, vec2_t* uv1, vec2_t* uv2,
                              kos_img_t texture) {
    uint16_t* tex_data = (uint16_t*)texture.data;
    int tex_width = texture.w;
    int tex_height = texture.h;

    // Compute bounding box
    int minx = (int)fmaxf(floorf(fminf(fminf(v0->x, v1->x), v2->x)), 0.0f);
    int miny = (int)fmaxf(floorf(fminf(fminf(v0->y, v1->y), v2->y)), 0.0f);
    int maxx = (int)fminf(ceilf(fmaxf(fmaxf(v0->x, v1->x), v2->x)), 640 - 1);
    int maxy = (int)fminf(ceilf(fmaxf(fmaxf(v0->y, v1->y), v2->y)), 480 - 1);

    // Convert vertices to int positions for rasterization
    vec2i_t p0 = {(int)v0->x, (int)v0->y};
    vec2i_t p1 = {(int)v1->x, (int)v1->y};
    vec2i_t p2 = {(int)v2->x, (int)v2->y};

    float area = (float)orient2d(&p0, &p1, &p2);
    if (area == 0.0f) return; // Degenerate triangle
    float inv_area = 1.0f / area;

    // Precompute perspective-correct factors
    float one_over_z0 = 1.0f / v0->z;
    float one_over_z1 = 1.0f / v1->z;
    float one_over_z2 = 1.0f / v2->z;

    float u0_z = uv0->x * one_over_z0;
    float v0_z = uv0->y * one_over_z0;
    float u1_z = uv1->x * one_over_z1;
    float v1_z = uv1->y * one_over_z1;
    float u2_z = uv2->x * one_over_z2;
    float v2_z = uv2->y * one_over_z2;

    vec2i_t p = {minx, miny};

    // Initial edge function values at top-left corner
    int w0_row = orient2d(&p1, &p2, &p);
    int w1_row = orient2d(&p2, &p0, &p);
    int w2_row = orient2d(&p0, &p1, &p);

    for (p.y = miny; p.y <= maxy; ++p.y) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minx; p.x <= maxx; ++p.x) {
            if ((w0 | w1 | w2) >= 0) {
                float b0 = (float)w0 * inv_area;
                float b1 = (float)w1 * inv_area;
                float b2 = (float)w2 * inv_area;

                // Interpolate 1/z
                float one_over_z = b0 * one_over_z0 + b1 * one_over_z1 + b2 * one_over_z2;

                // Interpolate u/z and v/z
                float u_over_z = b0 * u0_z + b1 * u1_z + b2 * u2_z;
                float v_over_z = b0 * v0_z + b1 * v1_z + b2 * v2_z;

                // Recover perspective-correct u and v
                float u = u_over_z / one_over_z;
                float v = v_over_z / one_over_z;

                // Texture lookup
                int tex_x = (int)(u * (tex_width - 1));
                int tex_y = (int)((1.0f - v) * (tex_height - 1)); // Flip V if needed

                // Clamp texture coordinates
                if (tex_x < 0) tex_x = 0;
                if (tex_x >= tex_width) tex_x = tex_width - 1;
                if (tex_y < 0) tex_y = 0;
                if (tex_y >= tex_height) tex_y = tex_height - 1;

                uint16_t color = tex_data[tex_y * tex_width + tex_x];

                float z = 1.0f / one_over_z;
                if (z > get_z_buffer_at(p.x, p.y)) {
                    update_zbuffer(p.x, p.y, z);
                    draw_pixel(p.x, p.y, color);
                }
            }

            // Edge function increments in X
            w0 += (p1.y - p2.y);
            w1 += (p2.y - p0.y);
            w2 += (p0.y - p1.y);
        }

        // Edge function increments in Y
        w0_row += (p2.x - p1.x);
        w1_row += (p0.x - p2.x);
        w2_row += (p1.x - p0.x);
    }
}

void draw_textured_triangle_new(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, vec2_t* uv0, vec2_t* uv1, vec2_t* uv2, kos_img_t texture) {
    uint16_t* tex_data = (uint16_t*)texture.data;
                                    // Sort vertices by Y
    if (v1->y < v0->y) { SWAP(v0, v1, vec2i_t*); SWAP(uv0, uv1, vec2_t*); }
    if (v2->y < v0->y) { SWAP(v0, v2, vec2i_t*); SWAP(uv0, uv2, vec2_t*); }
    if (v2->y < v1->y) { SWAP(v1, v2, vec2i_t*); SWAP(uv1, uv2, vec2_t*); }

    int total_height = v2->y - v0->y;
    if (total_height == 0) return; // Degenerate triangle

    for (int y = v0->y; y <= v2->y; y++) {

        int second_half = y > v1->y || v1->y == v0->y;
        int segment_height = second_half ? (v2->y - v1->y) : (v1->y - v0->y);
        if (segment_height == 0) continue;

        float alpha = (float)(y - v0->y) / total_height;
        float beta = (float)(y - (second_half ? v1->y : v0->y)) / segment_height;

        // Interpolate X
        float ax = v0->x + (v2->x - v0->x) * alpha;
        float bx = second_half
            ? v1->x + (v2->x - v1->x) * beta
            : v0->x + (v1->x - v0->x) * beta;

        // Interpolate UV
        float au = uv0->x + (uv2->x - uv0->x) * alpha;
        float av = uv0->y + (uv2->y - uv0->y) * alpha;

        float bu = second_half
            ? uv1->x + (uv2->x - uv1->x) * beta
            : uv0->x + (uv1->x - uv0->x) * beta;

        float bv = second_half
            ? uv1->y + (uv2->y - uv1->y) * beta
            : uv0->y + (uv1->y - uv0->y) * beta;

        if (ax > bx) {
            SWAP(ax, bx, float);
            SWAP(au, bu, float);
            SWAP(av, bv, float);
        }

        int x_start = (int)ax;
        int x_end = (int)bx;
        float t_step = (bx - ax) != 0.0f ? 1.0f / (bx - ax) : 0.0f;
        float t = 0.0f;

        for (int x = x_start; x <= x_end; x++) {
            float u = au + (bu - au) * t;
            float v = av + (bv - av) * t;

            int tex_u = (int)(u * texture.w);
            int tex_v = (int)(v * texture.h);

            uint16_t color = tex_data[tex_v * texture.w + tex_u];
            draw_pixel(x, y, color);

            t += t_step;
        }
    }
}

vec3f_t get_triangle_face_normal(vector_t vertices[3]){
    vec3f_t v1 = vec3_sub(vec3_from_vec4(vertices[1]),vec3_from_vec4(vertices[0]));
    vec3f_t v2 = vec3_sub(vec3_from_vec4(vertices[2]),vec3_from_vec4(vertices[0]));
    vec3f_t tri_normal = vec3_cross(v1,v2);

    return tri_normal;
}
