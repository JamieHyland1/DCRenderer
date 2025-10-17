#include "../include/renderer.h"
#include "shz_scalar.h"
#include "shz_mem.h"
#include <stdint.h>

inline float edge_func(float x0, float y0, float x1, float y1, float x, float y) {
    shz_vec2_t edge = { x1 - x0, y1 - y0 };
    shz_vec2_t toP  = { x - x0,  y - y0 };
    return shz_vec2_cross(edge, toP);
}

static inline int is_top_left(float dx, float dy) {
    // Top-left rule: edge is top if dy > 0, or if dy == 0 and dx < 0
    return (dy > 0.0f) || (dy == 0.0f && dx < 0.0f);
}

static inline float clamp01(float a) {
    return a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
}

void draw_triangle(const triangle_t* tri, uint16_t color) {
    float x0 = tri->points[0].x;
    float y0 = tri->points[0].y;
    float x1 = tri->points[1].x;
    float y1 = tri->points[1].y;
    float x2 = tri->points[2].x;
    float y2 = tri->points[2].y;

    draw_linef(x0, y0, x1, y1, color);
    draw_linef(x1, y1, x2, y2, color);
    draw_linef(x2, y2, x0, y0, color);
}

void draw_filled_triangle(const triangle_t* tri, uint16_t color) {
    color = light_apply_intensity(color, tri->orientation_from_light);
    // Extract screen-space positions (already projected)
    float x0 = tri->points[0].x;
    float y0 = tri->points[0].y;
    float x1 = tri->points[1].x;
    float y1 = tri->points[1].y;
    float x2 = tri->points[2].x;
    float y2 = tri->points[2].y;

    // Compute bounding box
    float minx = fminf(x0, fminf(x1, x2));
    float miny = fminf(y0, fminf(y1, y2));
    float maxx = fmaxf(x0, fmaxf(x1, x2));
    float maxy = fmaxf(y0, fmaxf(y1, y2));

    // Round to integer pixel bounds
    int ix0 = (int)shz_floorf(minx);
    int iy0 = (int)shz_floorf(miny);
    int ix1 = (int)shz_ceilf(maxx);
    int iy1 = (int)shz_ceilf(maxy);

    // Precompute edge deltas
    float A01 = y0 - y1, B01 = x1 - x0;
    float A12 = y1 - y2, B12 = x2 - x1;
    float A20 = y2 - y0, B20 = x0 - x2;

    // Loop over bounding box
    for (int y = iy0; y <= iy1; y++) {
        for (int x = ix0; x <= ix1; x++) {
            // Sample at pixel center
            float px = (float)x ;
            float py = (float)y ;

            // Edge functions (using SH4 fipr)
            float w0 = fipr(x2 - x1, -(y2 - y1), 0.0f, 0.0f,
                            py - y1, px - x1, 0.0f, 0.0f);

            float w1 = fipr(x0 - x2, -(y0 - y2), 0.0f, 0.0f,
                            py - y2, px - x2, 0.0f, 0.0f);

            float w2 = fipr(x1 - x0, -(y1 - y0), 0.0f, 0.0f,
                            py - y0, px - x0, 0.0f, 0.0f);

            // Inside test
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                draw_pixel(x, y, color);
            }
        }
    }
}

void draw_filled_triangle_wire(const triangle_t* tri, uint16_t color) {
    float x0 = tri->points[0].x;
    float y0 = tri->points[0].y;
    float x1 = tri->points[1].x;
    float y1 = tri->points[1].y;
    float x2 = tri->points[2].x;
    float y2 = tri->points[2].y;

    float minx = fminf(x0, fminf(x1, x2));
    float miny = fminf(y0, fminf(y1, y2));
    float maxx = fmaxf(x0, fmaxf(x1, x2));
    float maxy = fmaxf(y0, fmaxf(y1, y2));

    int ix0 = (int)shz_floorf(minx);
    int iy0 = (int)shz_floorf(miny);
    int ix1 = (int)shz_ceilf(maxx);
    int iy1 = (int)shz_ceilf(maxy);

    // Edge deltas
    float A01 = y0 - y1, B01 = x1 - x0;
    float A12 = y1 - y2, B12 = x2 - x1;
    float A20 = y2 - y0, B20 = x0 - x2;

    // Subpixel start
    float px = (float)ix0 ;
    float py = (float)iy0 ;

    // Initial edge function values at (px, py)
    float w0_row = fipr(x2 - x1, -(y2 - y1), 0, 0, py - y1, px - x1, 0, 0);
    float w1_row = fipr(x0 - x2, -(y0 - y2), 0, 0, py - y2, px - x2, 0, 0);
    float w2_row = fipr(x1 - x0, -(y1 - y0), 0, 0, py - y0, px - x0, 0, 0);



    for (int y = iy0; y <= iy1; y++) {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;

        for (int x = ix0; x <= ix1; x++) {
            int inside = (w0 >= 0 && w1 >= 0 && w2 >= 0);

            if (inside) {
                draw_pixel(x, y, color);
            }

            // step across row
            w0 += A12;
            w1 += A20;
            w2 += A01;
        }

        // step down one row
        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
    }

    draw_linef(x0, y0, x1, y1, 0xFFFF);
    draw_linef(x1, y1, x2, y2, 0xFFFF);
    draw_linef(x2, y2, x0, y0, 0xFFFF);
}
// Depreciated but just keeping here for now as its a good before and after test 
void draw_textured_triangle(const triangle_t *tri) {
    uint16_t *tex_data = (uint16_t *)tri->texture.data;
    const int tex_w = tri->texture.w;
    const int tex_h = tri->texture.h;

    // Vertex positions (screen space)
    const float x0 = tri->points[0].x, y0 = tri->points[0].y, w0 = tri->points[0].w, z0 = tri->points[0].z;
    const float x1 = tri->points[1].x, y1 = tri->points[1].y, w1 = tri->points[1].w, z1 = tri->points[1].z;
    const float x2 = tri->points[2].x, y2 = tri->points[2].y, w2 = tri->points[2].w, z2 = tri->points[2].z;

    float area = edge_func(x0, y0, x1, y1, x2, y2);

    // Early out: backface or degeneratearea
    if (fabsf(area) < 1e-6f) return;
    const float inv_area = 1.0f / area;

    // Perspective-correct setup
    const float invW0 = 1.0f / w0;
    const float invW1 = 1.0f / w1;
    const float invW2 = 1.0f / w2;

    const float u0 = tri->texcoords[0].u, v0t = tri->texcoords[0].v;
    const float u1 = tri->texcoords[1].u, v1t = tri->texcoords[1].v;
    const float u2 = tri->texcoords[2].u, v2t = tri->texcoords[2].v;

    const float u0o = u0 * invW0, v0o = v0t * invW0, z0o = z0 * invW0;
    const float u1o = u1 * invW1, v1o = v1t * invW1, z1o = z1 * invW1;
    const float u2o = u2 * invW2, v2o = v2t * invW2, z2o = z2 * invW2;

    // Bounding box
    float minxf = fminf(x0, fminf(x1, x2));
    float minyf = fminf(y0, fminf(y1, y2));
    float maxxf = fmaxf(x0, fmaxf(x1, x2));
    float maxyf = fmaxf(y0, fmaxf(y1, y2));

    int minx = (int)shz_floorf(minxf);
    int miny = (int)shz_floorf(minyf);
    int maxx = (int)shz_ceilf(maxxf) - 1;
    int maxy = (int)shz_ceilf(maxyf) - 1;

    if (minx > maxx || miny > maxy) return;

    // Edge deltas
    const float dx01 = x1 - x0, dy01 = y1 - y0;
    const float dx12 = x2 - x1, dy12 = y2 - y1;
    const float dx20 = x0 - x2, dy20 = y0 - y2;

    // Top-left flags
    const int tl01 = is_top_left(dx01, dy01);
    const int tl12 = is_top_left(dx12, dy12);
    const int tl20 = is_top_left(dx20, dy20);

    // Start values at top-left pixel center
    float start_x = (float)minx ;
    float start_y = (float)miny ;

    float e01_row = edge_func(x0, y0, x1, y1, start_x, start_y);
    float e12_row = edge_func(x1, y1, x2, y2, start_x, start_y);
    float e20_row = edge_func(x2, y2, x0, y0, start_x, start_y);

    // Edge function step increments
    const float step_e01_x = (y0 - y1);
    const float step_e12_x = (y1 - y2);
    const float step_e20_x = (y2 - y0);

    const float step_e01_y = (x1 - x0);
    const float step_e12_y = (x2 - x1);
    const float step_e20_y = (x0 - x2);

    // Lighting
    const float intensity = clamp01(tri->orientation_from_light);

    // Rasterize
    for (int py = miny; py <= maxy; ++py) {
        float e01 = e01_row;
        float e12 = e12_row;
        float e20 = e20_row;

        for (int px = minx; px <= maxx; ++px) {
            const int c01 = (e01 > 0.0f) || (e01 == 0.0f && tl01);
            const int c12 = (e12 > 0.0f) || (e12 == 0.0f && tl12);
            const int c20 = (e20 > 0.0f) || (e20 == 0.0f && tl20);

            if (c01 && c12 && c20) {
                const float w0b = e12 * inv_area;
                const float w1b = e20 * inv_area;
                const float w2b = e01 * inv_area;

                const float invW = w0b * invW0 + w1b * invW1 + w2b * invW2;

                if (invW > 0.0f) {
                    // Perspective correct interpolate
                    const float u_over_w = w0b * u0o + w1b * u1o + w2b * u2o;
                    const float v_over_w = w0b * v0o + w1b * v1o + w2b * v2o;
                    const float z_over_w = w0b * z0o + w1b * z1o + w2b * z2o;

                    float u = u_over_w / invW;
                    float v = v_over_w / invW;
                    float z = z_over_w * invW;   // I was originally dividing by invW here

                    // Z-buffer test
                    float old_z = get_z_buffer_at(px, py);
                    if (z > old_z) {  // closer or empty
                        u = clamp01(u);
                        v = clamp01(v);

                        int tx = (int)(u * (float)(tex_w - 1));
                        int ty = (int)((1.0f - v) * (float)(tex_h - 1));

                        uint16_t color = tex_data[ty * tex_w + tx];
                        uint16_t lit_color = light_apply_intensity(color, intensity);
                        draw_pixel(px, py, lit_color);

                        update_zbuffer(px, py, z);
                    }
                }
            }

            e01 += step_e01_x;
            e12 += step_e12_x;
            e20 += step_e20_x;
        }

        e01_row += step_e01_y;
        e12_row += step_e12_y;
        e20_row += step_e20_y;
    }
}

void draw_textured_triangle_scanline(const triangle_t *tri){
    uint16_t *tex_data = (uint16_t *)tri->texture.data;
    const int tex_w = tri->texture.w;
    const int tex_h = tri->texture.h;

    // Vertex positions
    float x_top    = tri->points[0].x, y_top    = tri->points[0].y;
    float x_mid    = tri->points[1].x, y_mid    = tri->points[1].y;
    float x_bottom = tri->points[2].x, y_bottom = tri->points[2].y;

    float u_top    = tri->texcoords[0].u, v_top    = tri->texcoords[0].v; 
    float u_mid    = tri->texcoords[1].u, v_mid    = tri->texcoords[1].v; 
    float u_bottom = tri->texcoords[2].u, v_bottom = tri->texcoords[2].v; 
    
    // Sort vertices by Y (top -> mid -> bottom), also swap texcoords
    if (y_top > y_mid) {
        float t;

        t = y_top;    y_top = y_mid;    y_mid = t;
        t = x_top;    x_top = x_mid;    x_mid = t;
        t = u_top;    u_top = u_mid;    u_mid = t;
        t = v_top;    v_top = v_mid;    v_mid = t;
    }
    if (y_mid > y_bottom) {
        float t;
        t = y_mid;    y_mid = y_bottom;    y_bottom = t;
        t = x_mid;    x_mid = x_bottom;    x_bottom = t;
        t = u_mid;    u_mid = u_bottom;    u_bottom = t;
        t = v_mid;    v_mid = v_bottom;    v_bottom = t;
    }
    if (y_top > y_mid) {
        float t;
        t = y_top;    y_top = y_mid;    y_mid = t;
        t = x_top;    x_top = x_mid;    x_mid = t;
        t = u_top;    u_top = u_mid;    u_mid = t;
        t = v_top;    v_top = v_mid;    v_mid = t;
    }

    float height_top_to_mid    = y_mid - y_top;
    float height_top_to_bottom = y_bottom - y_top;
    float height_mid_to_bottom = y_bottom - y_mid;

    if (height_top_to_bottom == 0.0f) return; // Degenerate triangle

    float slope_top_to_mid    = (height_top_to_mid != 0.0f) ? shz_divf((x_mid - x_top), height_top_to_mid) : 0.0f;
    float slope_top_to_bottom = shz_divf((x_bottom - x_top), height_top_to_bottom);
    float slope_u_top_to_mid    = (height_top_to_mid != 0.0f) ? shz_divf((u_mid - u_top), height_top_to_mid) : 0.0f;
    float slope_v_top_to_mid    = (height_top_to_mid != 0.0f) ? shz_divf((v_mid - v_top), height_top_to_mid) : 0.0f;
    float slope_u_top_to_bottom = shz_divf((u_bottom - u_top), height_top_to_bottom);
    float slope_v_top_to_bottom = shz_divf((v_bottom - v_top), height_top_to_bottom);

    float x_left = x_top;
    float x_right = x_top;
    float u_left = u_top; 
    float v_left = v_top; 
    float u_right = u_top; 
    float v_right = v_top; 

    float u_scale = (float)(tex_w - 1);
    float v_scale = (float)(tex_h - 1);

    // --------------------------
    // Upper part: from top -> mid
    // --------------------------
    for (int y = (int)shz_ceilf(y_top); y <= (int)shz_floorf(y_mid); y++) {
        
        int x_start = (int)shz_floorf(x_left);
        int x_end   = (int)shz_ceilf(x_right);
        if (x_start > x_end) { int t=x_start; x_start=x_end; x_end=t; }

        float span_width = (x_right - x_left);
        if (fabsf(span_width) < 1e-6f){ 
            span_width = 1.0f;
        }
        float inv_span = shz_invf(span_width);

        float u = (u_left * u_scale) + ((x_start - x_left) * inv_span) * (u_right - u_left) * u_scale;
        float v = (v_left * v_scale) + ((x_start - x_left) * inv_span) * (v_right - v_left) * v_scale;

        float du_dx = (u_right - u_left) * inv_span * u_scale;
        float dv_dx = (v_right - v_left) * inv_span * v_scale;

        for (int x = x_start; x <= x_end; x++) {
            int tx = (int)u;
            int ty = tex_h - 1 - (int)v;

            // Sample texture
            uint16_t color = tex_data[ty * tex_w + tx];
            draw_pixel(x, y, color);

            // Increment texture coordinates
            u += du_dx;
            v += dv_dx;
        }

        // step left edge along top->mid, right edge along top->bottom
        x_left  += slope_top_to_mid;
        x_right += slope_top_to_bottom;
        u_left  += slope_u_top_to_mid;
        v_left  += slope_v_top_to_mid;
        u_right += slope_u_top_to_bottom;
        v_right += slope_v_top_to_bottom;
    }

    // ----------------------------
    // Lower part: from mid -> bottom
    // ----------------------------
    if (height_mid_to_bottom == 0.0f) return;

    // Slopes for lower part
    float slope_x_left  = (x_bottom - x_mid) / height_mid_to_bottom;
    float slope_u_left  = (u_bottom - u_mid) / height_mid_to_bottom;
    float slope_v_left  = (v_bottom - v_mid) / height_mid_to_bottom;

    float slope_x_right = slope_top_to_bottom;   
    float slope_u_right = slope_u_top_to_bottom;
    float slope_v_right = slope_v_top_to_bottom;

    // Reset left edge to start from the mid vertex
    x_left = x_mid;
    u_left = u_mid;
    v_left = v_mid;

    // Right edge continues from where it was at y_mid
    for (int y = (int)shz_ceilf(y_mid); y <= (int)shz_floorf(y_bottom); y++) {
        int x_start = (int)shz_floorf(x_left);
        int x_end   = (int)shz_ceilf(x_right);

        if (x_start > x_end) { int t = x_start; x_start = x_end; x_end = t; }

        float span_width = (x_right - x_left);
        if (fabsf(span_width) < 1e-6f) {
            x_left  += slope_x_left;
            x_right += slope_x_right;
            u_left  += slope_u_left;
            v_left  += slope_v_left;
            u_right += slope_u_right;
            v_right += slope_v_right;
            continue; // skip degenerate span
        }

        float inv_span = shz_invf(span_width);

        float u = (u_left * u_scale) + ((x_start - x_left) * inv_span) * (u_right - u_left) * u_scale;
        float v = (v_left * v_scale) + ((x_start - x_left) * inv_span) * (v_right - v_left) * v_scale;

        float du_dx = (u_right - u_left) * inv_span * u_scale;
        float dv_dx = (v_right - v_left) * inv_span * v_scale;

        for (int x = x_start; x <= x_end; x++) {
            int tx = (int)u;
            int ty = tex_h - 1 - (int)v;
            // was gonna test out mess around with trying to batch 4 texel fetches using the SH4 matrix
            // matrix_t uv;
            // uv[0][0] = u;
            // uv[1][0] = v;
            // uv[0][1] = u + du_dx;
            // uv[1][1] = v + dv_dx;
            // uv[0][2] = u + 2*du_dx;
            // uv[1][2] = v + 2*dv_dx;
            // uv[0][3] = u + 3*du_dx;
            // uv[1][3] = v + 3*dv_dx;
            // shz_xmtrx_load_4x4(&uv);



            // start_time = perf_cntr_timer_ns();
            uint16_t color = tex_data[ty * tex_w + tx];
            draw_pixel(x, y, color);
            // end_time = perf_cntr_timer_ns();
            // avg += end_time - start_time;

            u += du_dx;
            v += dv_dx;
        }

        x_left  += slope_x_left;
        x_right += slope_x_right;
        u_left  += slope_u_left;
        v_left  += slope_v_left;
        u_right += slope_u_right;
        v_right += slope_v_right;
    }
}

shz_vec3_t get_triangle_face_normal(shz_vec4_t vertices[3]){
    shz_vec3_t v1 = shz_vec3_sub(vec3_from_vec4(vertices[1]),vec3_from_vec4(vertices[0]));
    shz_vec3_t v2 = shz_vec3_sub(vec3_from_vec4(vertices[2]),vec3_from_vec4(vertices[0]));
    shz_vec3_t tri_normal = shz_vec3_cross(v1, v2);

    return tri_normal;
}
