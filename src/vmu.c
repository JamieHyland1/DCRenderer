#include "../include/vmu.h"
#include <string.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
// VMU LCD Icon Drawing Functions
// I basically vibe coded these bored one night, I havent even tried them yet
// Will come back and fix them later
///////////////////////////////////////////////////////////////////////////////

maple_device_t* vmu_find_lcd(void) {
    for (int port = 0; port < 4; port++) {
        for (int unit = 0; unit < 6; unit++) {
            maple_device_t *dev = maple_enum_dev(port, unit);
            if (dev && (dev->info.functions & MAPLE_FUNC_LCD)) {
                return dev;
            }
        }
    }
    return NULL;
}

void vmu_convert_icon_to_lcd(const uint8_t *img_4bpp, uint8_t *out_lcd) {
    memset(out_lcd, 0, VMU_LCD_BYTES);

    for (int y = 0; y < VMU_LCD_HEIGHT; y++) {
        for (int x = 0; x < VMU_LCD_WIDTH; x++) {
            int pixel_index = y * VMU_LCD_WIDTH + x;
            int byte_index = pixel_index / 2;
            uint8_t byte = img_4bpp[byte_index];

            uint8_t pixel;
            if (pixel_index % 2 == 0)
                pixel = (byte >> 4) & 0xF;
            else
                pixel = byte & 0xF;

            int bit = (pixel >= 8) ? 1 : 0;

            int lcd_byte_index = y * (VMU_LCD_WIDTH / 8) + (x / 8);
            int lcd_bit = 7 - (x % 8);
            out_lcd[lcd_byte_index] |= (bit << lcd_bit);
        }
    }
}

int vmu_draw_icon(maple_device_t *vmu, const uint8_t *img_4bpp) {
    if (!vmu) {
        return -1;
    }

    uint8_t lcd_buffer[VMU_LCD_BYTES];
    vmu_convert_icon_to_lcd(img_4bpp, lcd_buffer);
    return vmu_draw_lcd(vmu, lcd_buffer);
}
