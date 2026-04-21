#include "../include/renderer.h"
#include "shz_scalar.h"
#include "shz_mem.h"
#include <stdint.h>

inline float edge_func(float x0, float y0, float x1, float y1, float x, float y) {
    shz_vec2_t edge = {{{ x1 - x0, y1 - y0 }}};
    shz_vec2_t toP  = {{{ x - x0,  y - y0 }}};
    return shz_vec2_cross(edge, toP);
}

static inline int is_top_left(float dx, float dy) {
    // Top-left rule: edge is top if dy > 0, or if dy == 0 and dx < 0
    return (dy > 0.0f) || (dy == 0.0f && dx < 0.0f);
}

static inline float clamp01(float a) {
    return a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
}



static int dbg_prints_left = 40;
static int tex_dbg_once = 0;

static inline void dbg_span_print(
    int y,
    float xL, float xR,
    float uL, float uR,
    float vL, float vR,
    int x_start, int x_end,
    int xs, int xe,
    float u, float v,
    int tx, int ty,
    int tx_masked, int ty_masked,
    const texture_t *tex
) {
    if (dbg_prints_left <= 0) return;
    dbg_prints_left--;

    printf("SPAN y=%d xL=%.3f xR=%.3f xstart=%d xend=%d xs=%d xe=%d "
           "uL=%.3f uR=%.3f vL=%.3f vR=%.3f "
           "u=%.3f v=%.3f raw_tx=%d raw_ty=%d masked_tx=%d masked_ty=%d "
           "cw=%d ch=%d tw=%lu th=%lu shift=%d wmask=%d hmask=%d\n",
           y, xL, xR, x_start, x_end, xs, xe,
           uL, uR, vL, vR,
           u, v, tx, ty, tx_masked, ty_masked,
           tex->cw, tex->ch,
           (unsigned long)tex->img.w, (unsigned long)tex->img.h,
           tex->w_shift, tex->w_mask, tex->h_mask);
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

    float x_top    = tri->points[0].x, y_top    = tri->points[0].y;
    float x_mid    = tri->points[1].x, y_mid    = tri->points[1].y;
    float x_bottom = tri->points[2].x, y_bottom = tri->points[2].y;

    /* Sort vertices by Y: top -> mid -> bottom */
    if (y_top > y_mid) {
        float t;
        t = y_top; y_top = y_mid; y_mid = t;
        t = x_top; x_top = x_mid; x_mid = t;
    }
    if (y_mid > y_bottom) {
        float t;
        t = y_mid; y_mid = y_bottom; y_bottom = t;
        t = x_mid; x_mid = x_bottom; x_bottom = t;
    }
    if (y_top > y_mid) {
        float t;
        t = y_top; y_top = y_mid; y_mid = t;
        t = x_top; x_top = x_mid; x_mid = t;
    }

    float height_top_to_mid    = y_mid - y_top;
    float height_top_to_bottom = y_bottom - y_top;
    float height_mid_to_bottom = y_bottom - y_mid;

    if (height_top_to_bottom == 0.0f) return;

    /* Edge slopes */
    float slope_top_to_mid    = (height_top_to_mid != 0.0f)
        ? shz_divf((x_mid - x_top), height_top_to_mid)
        : 0.0f;

    float slope_top_to_bottom = shz_divf((x_bottom - x_top), height_top_to_bottom);

    float x_left  = x_top;
    float x_right = x_top;

    /* -------------------------- */
    /* Upper half: top -> mid     */
    /* -------------------------- */
    if (height_top_to_mid > 0.0f) {
        int y0_raw = (int)shz_ceilf(y_top);
        int y1_raw = (int)shz_floorf(y_mid);

        int y0 = y0_raw;
        int y1 = y1_raw;

        if (y0 < 0) y0 = 0;
        if (y1 > WINDOW_HEIGHT - 1) y1 = WINDOW_HEIGHT - 1;

        if (y0 <= y1) {
            float dy = (float)(y0 - y0_raw);
            if (dy != 0.0f) {
                x_left  += slope_top_to_mid    * dy;
                x_right += slope_top_to_bottom * dy;
            }

            for (int y = y0; y <= y1; y++) {
                float xL = x_left;
                float xR = x_right;

                if (xL > xR) {
                    float t = xL; xL = xR; xR = t;
                }

                float span_width = xR - xL;
                if (span_width <= 0.0f) {
                    x_left  += slope_top_to_mid;
                    x_right += slope_top_to_bottom;
                    continue;
                }

                int x_start = (int)shz_floorf(xL);
                int x_end   = (int)shz_ceilf(xR);

                int xs0 = x_start;
                int xe0 = x_end;

                if (!(xe0 < 0 || xs0 >= WINDOW_WIDTH)) {
                    int xs = xs0;
                    int xe = xe0;

                    if (xs < 0) xs = 0;
                    if (xe > WINDOW_WIDTH - 1) xe = WINDOW_WIDTH - 1;

                    if (xs <= xe) {
                        uint16_t *dst = buffer + y * WINDOW_WIDTH + xs;
                        int count = xe - xs + 1;
                        int n8 = count & ~7;

                        for (int i = 0; i < n8; i += 8) {
                            dst[0] = color; dst[1] = color;
                            dst[2] = color; dst[3] = color;
                            dst[4] = color; dst[5] = color;
                            dst[6] = color; dst[7] = color;
                            dst += 8;
                        }

                        for (int i = n8; i < count; i++) {
                            *dst++ = color;
                        }
                    }
                }

                x_left  += slope_top_to_mid;
                x_right += slope_top_to_bottom;
            }
        } else {
            x_right += slope_top_to_bottom * height_top_to_mid;
        }
    }

    /* ---------------------------- */
    /* Lower half: mid -> bottom    */
    /* ---------------------------- */
    if (height_mid_to_bottom == 0.0f) return;

    float slope_x_left  = shz_divf((x_bottom - x_mid), height_mid_to_bottom);
    float slope_x_right = slope_top_to_bottom;

    x_left = x_mid;

    int y0_raw = (int)shz_ceilf(y_mid);
    int y1_raw = (int)shz_floorf(y_bottom);

    int y0 = y0_raw;
    int y1 = y1_raw;

    if (y0 < 0) y0 = 0;
    if (y1 > WINDOW_HEIGHT - 1) y1 = WINDOW_HEIGHT - 1;

    if (y0 > y1) return;

    float dy = (float)(y0 - y0_raw);
    if (dy != 0.0f) {
        x_left  += slope_x_left  * dy;
        x_right += slope_x_right * dy;
    }

    for (int y = y0; y <= y1; y++) {
        float xL = x_left;
        float xR = x_right;

        if (xL > xR) {
            float t = xL; xL = xR; xR = t;
        }

        float span_width = xR - xL;
        if (span_width <= 0.0f) {
            x_left  += slope_x_left;
            x_right += slope_x_right;
            continue;
        }

        int x_start = (int)shz_floorf(xL);
        int x_end   = (int)shz_ceilf(xR);

        int xs0 = x_start;
        int xe0 = x_end;

        if (!(xe0 < 0 || xs0 >= WINDOW_WIDTH)) {
            int xs = xs0;
            int xe = xe0;

            if (xs < 0) xs = 0;
            if (xe > WINDOW_WIDTH - 1) xe = WINDOW_WIDTH - 1;

            if (xs <= xe) {
                uint16_t *dst = buffer + y * WINDOW_WIDTH + xs;
                int count = xe - xs + 1;
                int n8 = count & ~7;

                for (int i = 0; i < n8; i += 8) {
                    dst[0] = color; dst[1] = color;
                    dst[2] = color; dst[3] = color;
                    dst[4] = color; dst[5] = color;
                    dst[6] = color; dst[7] = color;
                    dst += 8;
                }

                for (int i = n8; i < count; i++) {
                    *dst++ = color;
                }
            }
        }

        x_left  += slope_x_left;
        x_right += slope_x_right;
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
// void draw_textured_triangle(const triangle_t *tri) {
//     uint16_t *tex_data = (uint16_t *)tri->texture->img.data;
//     const int tex_w = tri->texture->img.w;
//     const int tex_h = tri->texture->img.h;

//     // Vertex positions (screen space)
//     const float x0 = tri->points[0].x, y0 = tri->points[0].y, w0 = tri->points[0].w, z0 = tri->points[0].z;
//     const float x1 = tri->points[1].x, y1 = tri->points[1].y, w1 = tri->points[1].w, z1 = tri->points[1].z;
//     const float x2 = tri->points[2].x, y2 = tri->points[2].y, w2 = tri->points[2].w, z2 = tri->points[2].z;

//     float area = edge_func(x0, y0, x1, y1, x2, y2);

//     // Early out: backface or degeneratearea
//     if (fabsf(area) < 1e-6f) return;
//     const float inv_area = 1.0f / area;

//     // Perspective-correct setup
//     const float invW0 = 1.0f / w0;
//     const float invW1 = 1.0f / w1;
//     const float invW2 = 1.0f / w2;

//     const float u0 = tri->texcoords[0].u, v0t = tri->texcoords[0].v;
//     const float u1 = tri->texcoords[1].u, v1t = tri->texcoords[1].v;
//     const float u2 = tri->texcoords[2].u, v2t = tri->texcoords[2].v;

//     const float u0o = u0 * invW0, v0o = v0t * invW0, z0o = z0 * invW0;
//     const float u1o = u1 * invW1, v1o = v1t * invW1, z1o = z1 * invW1;
//     const float u2o = u2 * invW2, v2o = v2t * invW2, z2o = z2 * invW2;

//     // Bounding box
//     float minxf = fminf(x0, fminf(x1, x2));
//     float minyf = fminf(y0, fminf(y1, y2));
//     float maxxf = fmaxf(x0, fmaxf(x1, x2));
//     float maxyf = fmaxf(y0, fmaxf(y1, y2));

//     int minx = (int)shz_floorf(minxf);
//     int miny = (int)shz_floorf(minyf);
//     int maxx = (int)shz_ceilf(maxxf) - 1;
//     int maxy = (int)shz_ceilf(maxyf) - 1;

//     if (minx > maxx || miny > maxy) return;

//     // Edge deltas
//     const float dx01 = x1 - x0, dy01 = y1 - y0;
//     const float dx12 = x2 - x1, dy12 = y2 - y1;
//     const float dx20 = x0 - x2, dy20 = y0 - y2;

//     // Top-left flags
//     const int tl01 = is_top_left(dx01, dy01);
//     const int tl12 = is_top_left(dx12, dy12);
//     const int tl20 = is_top_left(dx20, dy20);

//     // Start values at top-left pixel center
//     float start_x = (float)minx ;
//     float start_y = (float)miny ;

//     float e01_row = edge_func(x0, y0, x1, y1, start_x, start_y);
//     float e12_row = edge_func(x1, y1, x2, y2, start_x, start_y);
//     float e20_row = edge_func(x2, y2, x0, y0, start_x, start_y);

//     // Edge function step increments
//     const float step_e01_x = (y0 - y1);
//     const float step_e12_x = (y1 - y2);
//     const float step_e20_x = (y2 - y0);

//     const float step_e01_y = (x1 - x0);
//     const float step_e12_y = (x2 - x1);
//     const float step_e20_y = (x0 - x2);

//     // Lighting
//     const float intensity = clamp01(tri->orientation_from_light);

//     // Rasterize
//     for (int py = miny; py <= maxy; ++py) {
//         float e01 = e01_row;
//         float e12 = e12_row;
//         float e20 = e20_row;

//         for (int px = minx; px <= maxx; ++px) {
//             const int c01 = (e01 > 0.0f) || (e01 == 0.0f && tl01);
//             const int c12 = (e12 > 0.0f) || (e12 == 0.0f && tl12);
//             const int c20 = (e20 > 0.0f) || (e20 == 0.0f && tl20);

//             if (c01 && c12 && c20) {
//                 const float w0b = e12 * inv_area;
//                 const float w1b = e20 * inv_area;
//                 const float w2b = e01 * inv_area;

//                 const float invW = w0b * invW0 + w1b * invW1 + w2b * invW2;

//                 if (invW > 0.0f) {
//                     // Perspective correct interpolate
//                     const float u_over_w = w0b * u0o + w1b * u1o + w2b * u2o;
//                     const float v_over_w = w0b * v0o + w1b * v1o + w2b * v2o;
//                     const float z_over_w = w0b * z0o + w1b * z1o + w2b * z2o;

//                     float u = u_over_w / invW;
//                     float v = v_over_w / invW;
//                     float z = z_over_w * invW;   // I was originally dividing by invW here

//                     // Z-buffer test
//                     float old_z = get_z_buffer_at(px, py);
//                     if (z > old_z) {  // closer or empty
//                         u = clamp01(u);
//                         v = clamp01(v);

//                         int tx = (int)(u * (float)(tex_w - 1));
//                         int ty = (int)((1.0f - v) * (float)(tex_h - 1));

//                         uint16_t color = tex_data[ty * tex_w + tx];
//                         uint16_t lit_color = light_apply_intensity(color, intensity);
//                         draw_pixel(px, py, lit_color);

//                         update_zbuffer(px, py, z);
//                     }
//                 }
//             }

//             e01 += step_e01_x;
//             e12 += step_e12_x;
//             e20 += step_e20_x;
//         }

//         e01_row += step_e01_y;
//         e12_row += step_e12_y;
//         e20_row += step_e20_y;
//     }
// }

static inline int clampi(int x, int lo, int hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void draw_textured_triangle_scanline(const triangle_t *tri, const texture_t* text)
{

    const uint16_t *tex_data = (const uint16_t *)text->img.data;

    const int tex_h = text->img.h;      // padded storage height
    const int sample_w = text->cw;      // real content width
    const int sample_h = text->ch;      // real content height

    const int shift = text->w_shift;

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
        t = y_top; y_top = y_mid; y_mid = t;
        t = x_top; x_top = x_mid; x_mid = t;
        t = u_top; u_top = u_mid; u_mid = t;
        t = v_top; v_top = v_mid; v_mid = t;
    }
    if (y_mid > y_bottom) {
        float t;
        t = y_mid; y_mid = y_bottom; y_bottom = t;
        t = x_mid; x_mid = x_bottom; x_bottom = t;
        t = u_mid; u_mid = u_bottom; u_bottom = t;
        t = v_mid; v_mid = v_bottom; v_bottom = t;
    }
    if (y_top > y_mid) {
        float t;
        t = y_top; y_top = y_mid; y_mid = t;
        t = x_top; x_top = x_mid; x_mid = t;
        t = u_top; u_top = u_mid; u_mid = t;
        t = v_top; v_top = v_mid; v_mid = t;
    }

    float height_top_to_mid    = y_mid - y_top;
    float height_top_to_bottom = y_bottom - y_top;
    float height_mid_to_bottom = y_bottom - y_mid;

    if (height_top_to_bottom == 0.0f) return;

    // Precompute edge slopes (per Y)
    float slope_top_to_mid      = (height_top_to_mid != 0.0f) ? shz_divf((x_mid - x_top), height_top_to_mid) : 0.0f;
    float slope_top_to_bottom   = shz_divf((x_bottom - x_top), height_top_to_bottom);

    float slope_u_top_to_mid    = (height_top_to_mid != 0.0f) ? shz_divf((u_mid - u_top), height_top_to_mid) : 0.0f;
    float slope_v_top_to_mid    = (height_top_to_mid != 0.0f) ? shz_divf((v_mid - v_top), height_top_to_mid) : 0.0f;

    float slope_u_top_to_bottom = shz_divf((u_bottom - u_top), height_top_to_bottom);
    float slope_v_top_to_bottom = shz_divf((v_bottom - v_top), height_top_to_bottom);

    // Current edge values
    float x_left  = x_top;
    float x_right = x_top;
    float u_left  = u_top;
    float v_left  = v_top;
    float u_right = u_top;
    float v_right = v_top;

    // Scale UVs into real content texel space, not padded storage space
    const float u_scale = (float)(sample_w - 1);
    const float v_scale = (float)(sample_h - 1);

    // --------------------------
    // Upper part: from top -> mid
    // --------------------------
    if (height_top_to_mid > 0.0f) {
        int y0_raw = (int)shz_ceilf(y_top);
        int y1_raw = (int)shz_floorf(y_mid);

        int y0 = y0_raw;
        int y1 = y1_raw;
        if (y0 < 0) y0 = 0;
        if (y1 > WINDOW_HEIGHT - 1) y1 = WINDOW_HEIGHT - 1;

        if (y0 <= y1) {
            float dy = (float)(y0 - y0_raw);
            if (dy != 0.0f) {
                x_left  += slope_top_to_mid      * dy;
                x_right += slope_top_to_bottom   * dy;
                u_left  += slope_u_top_to_mid    * dy;
                v_left  += slope_v_top_to_mid    * dy;
                u_right += slope_u_top_to_bottom * dy;
                v_right += slope_v_top_to_bottom * dy;
            }

            for (int y = y0; y <= y1; y++) {
                float xL = x_left, xR = x_right;
                float uL = u_left, uR = u_right;
                float vL = v_left, vR = v_right;

                if (xL > xR) {
                    float t;
                    t = xL; xL = xR; xR = t;
                    t = uL; uL = uR; uR = t;
                    t = vL; vL = vR; vR = t;
                }

                float span_width = xR - xL;
                if (span_width <= 0.0f) {
                    x_left  += slope_top_to_mid;
                    x_right += slope_top_to_bottom;
                    u_left  += slope_u_top_to_mid;
                    v_left  += slope_v_top_to_mid;
                    u_right += slope_u_top_to_bottom;
                    v_right += slope_v_top_to_bottom;
                    continue;
                }

                int x_start = (int)shz_floorf(xL);
                int x_end   = (int)shz_ceilf(xR);

                float inv_span = shz_invf(span_width);
                float du_dx = (uR - uL) * inv_span * u_scale;
                float dv_dx = (vR - vL) * inv_span * v_scale;

                float dx = (float)x_start - xL;
                float u = (uL * u_scale) + dx * du_dx;
                float v = (float)(sample_h - 1) - ((vL * v_scale) + dx * dv_dx);

                int xs0 = x_start;
                int xe0 = x_end;

                if (!(xe0 < 0 || xs0 >= WINDOW_WIDTH)) {
                    int xs = xs0;
                    int xe = xe0;
                    if (xs < 0) xs = 0;
                    if (xe > WINDOW_WIDTH - 1) xe = WINDOW_WIDTH - 1;

                    if (xs <= xe) {
                        float clip = (float)(xs - xs0);
                        u += clip * du_dx;
                        v -= clip * dv_dx;

                        uint16_t *dst = buffer + y * WINDOW_WIDTH + xs;
                        int count = (xe - xs + 1);
                        const int max_tx = sample_w - 1;
                        const int max_ty = sample_h - 1;
                        int n2 = count & ~1;

                        for (int i = 0; i < n2; i += 2) {
                            int tx0 = clampi((int)u, 0, max_tx);
                            int ty0 = clampi((int)v, 0, max_ty);
                            u += du_dx; v -= dv_dx;

                            int tx1 = clampi((int)u, 0, max_tx);
                            int ty1 = clampi((int)v, 0, max_ty);
                            u += du_dx; v -= dv_dx;

                            dst[0] = tex_data[(ty0 << shift) + tx0];
                            dst[1] = tex_data[(ty1 << shift) + tx1];
                            dst += 2;
                        }

                        for (int i = n2; i < count; i++) {
                            int tx = clampi((int)u, 0, max_tx);
                            int ty = clampi((int)v, 0, max_ty);
                            *dst++ = tex_data[(ty << shift) + tx];
                            u += du_dx;
                            v -= dv_dx;
                        }
                    }
                }

                x_left  += slope_top_to_mid;
                x_right += slope_top_to_bottom;
                u_left  += slope_u_top_to_mid;
                v_left  += slope_v_top_to_mid;
                u_right += slope_u_top_to_bottom;
                v_right += slope_v_top_to_bottom;
            }
        } else {
            x_right += slope_top_to_bottom   * height_top_to_mid;
            u_right += slope_u_top_to_bottom * height_top_to_mid;
            v_right += slope_v_top_to_bottom * height_top_to_mid;
        }
    }

    // ----------------------------
    // Lower part: from mid -> bottom
    // ----------------------------
    if (height_mid_to_bottom == 0.0f) return;

    float slope_x_left   = shz_divf((x_bottom - x_mid), height_mid_to_bottom);
    float slope_u_left   = shz_divf((u_bottom - u_mid), height_mid_to_bottom);
    float slope_v_left   = shz_divf((v_bottom - v_mid), height_mid_to_bottom);

    float slope_x_right  = slope_top_to_bottom;
    float slope_u_right  = slope_u_top_to_bottom;
    float slope_v_right  = slope_v_top_to_bottom;

    x_left = x_mid;
    u_left = u_mid;
    v_left = v_mid;

    int y0_raw = (int)shz_ceilf(y_mid);
    int y1_raw = (int)shz_floorf(y_bottom);

    int y0 = y0_raw;
    int y1 = y1_raw;
    if (y0 < 0) y0 = 0;
    if (y1 > WINDOW_HEIGHT - 1) y1 = WINDOW_HEIGHT - 1;

    if (y0 > y1) return;

    float dy = (float)(y0 - y0_raw);
    if (dy != 0.0f) {
        x_left  += slope_x_left  * dy;
        u_left  += slope_u_left  * dy;
        v_left  += slope_v_left  * dy;

        x_right += slope_x_right * dy;
        u_right += slope_u_right * dy;
        v_right += slope_v_right * dy;
    }

    for (int y = y0; y <= y1; y++) {
        float xL = x_left, xR = x_right;
        float uL = u_left, uR = u_right;
        float vL = v_left, vR = v_right;

        if (xL > xR) {
            float t;
            t = xL; xL = xR; xR = t;
            t = uL; uL = uR; uR = t;
            t = vL; vL = vR; vR = t;
        }

        float span_width = xR - xL;
        if (span_width <= 0.0f) {
            x_left  += slope_x_left;
            u_left  += slope_u_left;
            v_left  += slope_v_left;

            x_right += slope_x_right;
            u_right += slope_u_right;
            v_right += slope_v_right;
            continue;
        }

        int x_start = (int)shz_floorf(xL);
        int x_end   = (int)shz_ceilf(xR);

        float inv_span = shz_invf(span_width);
        float du_dx = (uR - uL) * inv_span * u_scale;
        float dv_dx = (vR - vL) * inv_span * v_scale;

        float dx0 = (float)x_start - xL;
        float u = (uL * u_scale) + dx0 * du_dx;
        float v = (float)(tex_h - 1) - ((vL * v_scale) + dx0 * dv_dx);

        int xs0 = x_start;
        int xe0 = x_end;

        if (!(xe0 < 0 || xs0 >= WINDOW_WIDTH)) {
            int xs = xs0;
            int xe = xe0;
            if (xs < 0) xs = 0;
            if (xe > WINDOW_WIDTH - 1) xe = WINDOW_WIDTH - 1;

            if (xs <= xe) {
                float clip = (float)(xs - xs0);
                u += clip * du_dx;
                v -= clip * dv_dx;

                uint16_t *dst = buffer + y * WINDOW_WIDTH + xs;
                int count = (xe - xs + 1);
                int n2 = count & ~1;
                const int max_tx = sample_w - 1;
                const int max_ty = sample_h - 1;
                for (int i = 0; i < n2; i += 2) {
                    int tx0 = clampi((int)u, 0, max_tx);
                    int ty0 = clampi((int)v, 0, max_ty);
                    u += du_dx; v -= dv_dx;

                    int tx1 = clampi((int)u, 0, max_tx);
                    int ty1 = clampi((int)v, 0, max_ty);
                    u += du_dx; v -= dv_dx;

                    dst[0] = tex_data[(ty0 << shift) + tx0];
                    dst[1] = tex_data[(ty1 << shift) + tx1];
                    dst += 2;
                }

                for (int i = n2; i < count; i++) {
                    int tx = clampi((int)u, 0, max_tx);
                    int ty = clampi((int)v, 0, max_ty);
                    *dst++ = tex_data[(ty << shift) + tx];
                    u += du_dx;
                    v -= dv_dx;
                }
            }
        }

        x_left  += slope_x_left;
        u_left  += slope_u_left;
        v_left  += slope_v_left;

        x_right += slope_x_right;
        u_right += slope_u_right;
        v_right += slope_v_right;
    }
}
void draw_textured_triangle_scanline_fast(const triangle_t *tri, const texture_t *text)
{
    const uint16_t *tex_data = (const uint16_t *)text->img.data;

    const int sample_w = text->cw;
    const int sample_h = text->ch;
    const int shift    = text->w_shift;
    const int w_mask   = text->w_mask;
    const int h_mask   = text->h_mask;

    float x_top    = tri->points[0].x, y_top    = tri->points[0].y;
    float x_mid    = tri->points[1].x, y_mid    = tri->points[1].y;
    float x_bottom = tri->points[2].x, y_bottom = tri->points[2].y;

    float u_top    = tri->texcoords[0].u, v_top    = tri->texcoords[0].v;
    float u_mid    = tri->texcoords[1].u, v_mid    = tri->texcoords[1].v;
    float u_bottom = tri->texcoords[2].u, v_bottom = tri->texcoords[2].v;

    // sort by Y
    if (y_top > y_mid) {
        float t;
        t = y_top; y_top = y_mid; y_mid = t;
        t = x_top; x_top = x_mid; x_mid = t;
        t = u_top; u_top = u_mid; u_mid = t;
        t = v_top; v_top = v_mid; v_mid = t;
    }
    if (y_mid > y_bottom) {
        float t;
        t = y_mid; y_mid = y_bottom; y_bottom = t;
        t = x_mid; x_mid = x_bottom; x_bottom = t;
        t = u_mid; u_mid = u_bottom; u_bottom = t;
        t = v_mid; v_mid = v_bottom; v_bottom = t;
    }
    if (y_top > y_mid) {
        float t;
        t = y_top; y_top = y_mid; y_mid = t;
        t = x_top; x_top = x_mid; x_mid = t;
        t = u_top; u_top = u_mid; u_mid = t;
        t = v_top; v_top = v_mid; v_mid = t;
    }

    const float h_tm = y_mid - y_top;
    const float h_tb = y_bottom - y_top;
    const float h_mb = y_bottom - y_mid;

    if (h_tb == 0.0f) return;

    const float slope_x_tm = (h_tm != 0.0f) ? shz_divf(x_mid - x_top, h_tm) : 0.0f;
    const float slope_x_tb = shz_divf(x_bottom - x_top, h_tb);

    const float slope_u_tm = (h_tm != 0.0f) ? shz_divf(u_mid - u_top, h_tm) : 0.0f;
    const float slope_v_tm = (h_tm != 0.0f) ? shz_divf(v_mid - v_top, h_tm) : 0.0f;

    const float slope_u_tb = shz_divf(u_bottom - u_top, h_tb);
    const float slope_v_tb = shz_divf(v_bottom - v_top, h_tb);

    const float u_scale = (float)(sample_w - 1);
    const float v_scale = (float)(sample_h - 1);

    float x_left  = x_top;
    float x_right = x_top;
    float u_left  = u_top;
    float v_left  = v_top;
    float u_right = u_top;
    float v_right = v_top;

    // ---------- upper half ----------
    if (h_tm > 0.0f) {
        const int y0 = (int)shz_ceilf(y_top);
        const int y1 = (int)shz_floorf(y_mid);

        if (y0 <= y1) {
            const float dy = (float)y0 - y_top;

            x_left  += slope_x_tm * dy;
            x_right += slope_x_tb * dy;
            u_left  += slope_u_tm * dy;
            v_left  += slope_v_tm * dy;
            u_right += slope_u_tb * dy;
            v_right += slope_v_tb * dy;

            for (int y = y0; y <= y1; y++) {
                float xL = x_left, xR = x_right;
                float uL = u_left, uR = u_right;
                float vL = v_left, vR = v_right;

                if (xL > xR) {
                    float t;
                    t = xL; xL = xR; xR = t;
                    t = uL; uL = uR; uR = t;
                    t = vL; vL = vR; vR = t;
                }

                const float span_width = xR - xL;
                if (span_width > 0.0f) {
                    const int x_start = (int)shz_floorf(xL);
                    const int x_end   = (int)shz_ceilf(xR);

                    const float inv_span = shz_invf(span_width);
                    const float du_dx = (uR - uL) * inv_span * u_scale;
                    const float dv_dx = (vR - vL) * inv_span * v_scale;

                    const float dx = (float)x_start - xL;
                    float u = (uL * u_scale) + dx * du_dx;
                    float v = (float)(sample_h - 1) - ((vL * v_scale) + dx * dv_dx);

                    uint16_t *dst = buffer + y * WINDOW_WIDTH + x_start;
                    const int count = x_end - x_start + 1;
                    const int n4 = count & ~3;

                    for (int i = 0; i < n4; i += 4) {
                        int tx0 = ((int)u) & w_mask;
                        int ty0 = ((int)v) & h_mask;
                        u += du_dx; v -= dv_dx;

                        int tx1 = ((int)u) & w_mask;
                        int ty1 = ((int)v) & h_mask;
                        u += du_dx; v -= dv_dx;

                        int tx2 = ((int)u) & w_mask;
                        int ty2 = ((int)v) & h_mask;
                        u += du_dx; v -= dv_dx;

                        int tx3 = ((int)u) & w_mask;
                        int ty3 = ((int)v) & h_mask;
                        u += du_dx; v -= dv_dx;

                        dst[0] = tex_data[(ty0 << shift) + tx0];
                        dst[1] = tex_data[(ty1 << shift) + tx1];
                        dst[2] = tex_data[(ty2 << shift) + tx2];
                        dst[3] = tex_data[(ty3 << shift) + tx3];
                        dst += 4;
                    }

                    for (int i = n4; i < count; i++) {
                        int tx = ((int)u) & w_mask;
                        int ty = ((int)v) & h_mask;
                        *dst++ = tex_data[(ty << shift) + tx];
                        u += du_dx;
                        v -= dv_dx;
                    }
                }

                x_left  += slope_x_tm;
                x_right += slope_x_tb;
                u_left  += slope_u_tm;
                v_left  += slope_v_tm;
                u_right += slope_u_tb;
                v_right += slope_v_tb;
            }
        } else {
            x_right += slope_x_tb * h_tm;
            u_right += slope_u_tb * h_tm;
            v_right += slope_v_tb * h_tm;
        }
    }

    // ---------- lower half ----------
    if (h_mb == 0.0f) return;

    const float slope_x_mb = shz_divf(x_bottom - x_mid, h_mb);
    const float slope_u_mb = shz_divf(u_bottom - u_mid, h_mb);
    const float slope_v_mb = shz_divf(v_bottom - v_mid, h_mb);

    x_left = x_mid;
    u_left = u_mid;
    v_left = v_mid;

    {
        const int y0 = (int)shz_ceilf(y_mid);
        const int y1 = (int)shz_floorf(y_bottom);
        if (y0 > y1) return;

        const float dy = (float)y0 - y_mid;

        x_left  += slope_x_mb * dy;
        u_left  += slope_u_mb * dy;
        v_left  += slope_v_mb * dy;

        x_right += slope_x_tb * dy;
        u_right += slope_u_tb * dy;
        v_right += slope_v_tb * dy;

        for (int y = y0; y <= y1; y++) {
            float xL = x_left, xR = x_right;
            float uL = u_left, uR = u_right;
            float vL = v_left, vR = v_right;

            if (xL > xR) {
                float t;
                t = xL; xL = xR; xR = t;
                t = uL; uL = uR; uR = t;
                t = vL; vL = vR; vR = t;
            }

            const float span_width = xR - xL;
            if (span_width > 0.0f) {
                const int x_start = (int)shz_floorf(xL);
                const int x_end   = (int)shz_ceilf(xR);

                const float inv_span = shz_invf(span_width);
                const float du_dx = (uR - uL) * inv_span * u_scale;
                const float dv_dx = (vR - vL) * inv_span * v_scale;

                const float dx = (float)x_start - xL;
                float u = (uL * u_scale) + dx * du_dx;
                float v = (float)(sample_h - 1) - ((vL * v_scale) + dx * dv_dx);

                uint16_t *dst = buffer + y * WINDOW_WIDTH + x_start;
                const int count = x_end - x_start + 1;
                const int n4 = count & ~3;

                for (int i = 0; i < n4; i += 4) {
                    int tx0 = ((int)u) & w_mask;
                    int ty0 = ((int)v) & h_mask;
                    u += du_dx; v -= dv_dx;

                    int tx1 = ((int)u) & w_mask;
                    int ty1 = ((int)v) & h_mask;
                    u += du_dx; v -= dv_dx;

                    int tx2 = ((int)u) & w_mask;
                    int ty2 = ((int)v) & h_mask;
                    u += du_dx; v -= dv_dx;

                    int tx3 = ((int)u) & w_mask;
                    int ty3 = ((int)v) & h_mask;
                    u += du_dx; v -= dv_dx;

                    dst[0] = tex_data[(ty0 << shift) + tx0];
                    dst[1] = tex_data[(ty1 << shift) + tx1];
                    dst[2] = tex_data[(ty2 << shift) + tx2];
                    dst[3] = tex_data[(ty3 << shift) + tx3];
                    dst += 4;
                }

                for (int i = n4; i < count; i++) {
                    int tx = ((int)u) & w_mask;
                    int ty = ((int)v) & h_mask;
                    *dst++ = tex_data[(ty << shift) + tx];
                    u += du_dx;
                    v -= dv_dx;
                }
            }

            x_left  += slope_x_mb;
            u_left  += slope_u_mb;
            v_left  += slope_v_mb;

            x_right += slope_x_tb;
            u_right += slope_u_tb;
            v_right += slope_v_tb;
        }
    }
}

  int triangle_outside_screen(const triangle_t *tri) {
    float x0 = tri->points[0].x, y0 = tri->points[0].y;
    float x1 = tri->points[1].x, y1 = tri->points[1].y;
    float x2 = tri->points[2].x, y2 = tri->points[2].y;

    float min_x = x0;
    float max_x = x0;
    float min_y = y0;
    float max_y = y0;

    if (x1 < min_x) min_x = x1;
    if (x1 > max_x) max_x = x1;
    if (x2 < min_x) min_x = x2;
    if (x2 > max_x) max_x = x2;

    if (y1 < min_y) min_y = y1;
    if (y1 > max_y) max_y = y1;
    if (y2 < min_y) min_y = y2;
    if (y2 > max_y) max_y = y2;

    return (max_x < 0.0f || min_x >= (float)WINDOW_WIDTH ||
            max_y < 0.0f || min_y >= (float)WINDOW_HEIGHT);
}

  int triangle_fully_inside_screen(const triangle_t *tri) {
    float x0 = tri->points[0].x, y0 = tri->points[0].y;
    float x1 = tri->points[1].x, y1 = tri->points[1].y;
    float x2 = tri->points[2].x, y2 = tri->points[2].y;

    float min_x = x0;
    float max_x = x0;
    float min_y = y0;
    float max_y = y0;

    if (x1 < min_x) min_x = x1;
    if (x1 > max_x) max_x = x1;
    if (x2 < min_x) min_x = x2;
    if (x2 > max_x) max_x = x2;

    if (y1 < min_y) min_y = y1;
    if (y1 > max_y) max_y = y1;
    if (y2 < min_y) min_y = y2;
    if (y2 > max_y) max_y = y2;

    return (min_x >= 0.0f && max_x < (float)WINDOW_WIDTH &&
            min_y >= 0.0f && max_y < (float)WINDOW_HEIGHT);
}


shz_vec3_t get_triangle_face_normal(shz_vec4_t vertices[3]){
    shz_vec3_t v1 = shz_vec3_sub(vec3_from_vec4(vertices[1]),vec3_from_vec4(vertices[0]));
    shz_vec3_t v2 = shz_vec3_sub(vec3_from_vec4(vertices[2]),vec3_from_vec4(vertices[0]));
    shz_vec3_t tri_normal = shz_vec3_cross(v1, v2);

    return tri_normal;
}
