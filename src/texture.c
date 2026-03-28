#include <stdio.h>
#include <stdint.h>
#include "../include/renderer.h"


tex2_t new_tex2(float u, float v){
    tex2_t result;
    result.u = u;
    result.v = v;

    return result;
}
static inline int is_pow2(int x) {
    return x > 0 && (x & (x - 1)) == 0;
}

static inline int log2_pow2(int x) {
    int s = 0;
    while ((1 << s) < x) s++;
    return s;
}

bool texture_init(texture_t *t, kos_img_t img,
                  int content_w, int content_h)
{
    t->img = img;

    uint16_t *src = (uint16_t *)img.data;

    int new_width  = img.w;
    int new_height = img.h;

    if (!is_pow2(img.w))
        new_width = 1 << log2_pow2(img.w);

    if (!is_pow2(img.h))
        new_height = 1 << log2_pow2(img.h);

    if (new_width != img.w || new_height != img.h) {

        size_t new_size = new_width * new_height * sizeof(uint16_t);

        fprintf(stderr,
            "Texture not power of two (%d x %d). "
            "Resizing to %d x %d\n",
            img.w, img.h,
            new_width, new_height);

        uint16_t *dst = memalign(32, new_size);
        if (!dst) {
            fprintf(stderr, "Texture allocation failed\n");
            return false;
        }

        memset(dst, 0, new_size);

        for (int y = 0; y < img.h; y++) {
            memcpy(
                &dst[y * new_width],
                &src[y * img.w],
                img.w * sizeof(uint16_t)
            );
        }

        t->img.data = dst;
        t->img.w = new_width;
        t->img.h = new_height;
    }

    t->cw = content_w;
    t->ch = content_h;

    t->w_shift = log2_pow2(t->img.w);
    t->w_mask  = t->img.w - 1;
    t->h_mask  = t->img.h - 1;

    printf("INIT: img.w=%d img.h=%d w_shift=%d w_mask=%d h_mask=%d data=%p\n",
       t->img.w, t->img.h, t->w_shift, t->w_mask, t->h_mask, t->img.data);

    return true;
}
