#include <stdio.h>
#include <stdint.h>
#include "../include/light.h"


static light_t main_light;

void init_light(shz_vec3_t direction){
    main_light.direction = direction;
}

shz_vec3_t get_light_direction(){
    return main_light.direction;
}

////////////////////////////////////////////////
// apply color change based off light intensity
////////////////////////////////////////////////

uint16_t light_apply_intensity(uint16_t color, float percentage_factor) {
    // Clamp percentage_factor to [0.0, 1.0] for dimming (adjust range as needed)
    percentage_factor = SHZ_CLAMP(percentage_factor, 0.0f, 1.0f);

    // Extract components in RGB565 format
    uint16_t r = (color & 0xF800) >> 11;   // Red: 5 bits (0xF800, bits 11-15)
    uint16_t g = (color & 0x07E0) >> 5;    // Green: 6 bits (0x07E0, bits 5-10)
    uint16_t b = (color & 0x001F);       // Blue: 5 bits (0x001F, bits 0-4)

    // Apply intensity factor (scale and clamp to 5-bit for r,b; 6-bit for g)
    r = (uint16_t)(r * percentage_factor + 0.5f) & 0x1F; // Clamp to 5 bits (0-31)
    g = (uint16_t)(g * percentage_factor + 0.5f) & 0x3F; // Clamp to 6 bits (0-63)
    b = (uint16_t)(b * percentage_factor + 0.5f) & 0x1F; // Clamp to 5 bits (0-31)

    // Combine components back into RGB565 format
    uint16_t new_color = (r << 11) | (g << 5) | b;

    return new_color;
}

