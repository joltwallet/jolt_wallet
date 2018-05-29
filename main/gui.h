#ifndef __JOLT_GUI_H__
#define __JOLT_GUI_H__

void setup_screen(u8g2_t *u8g2);
void gui_task();

#define FULLSCREEN_ENTER(menu) \
    (menu)->post_draw = NULL; \
    bool statusbar_draw_original = statusbar_draw_enable; \
    statusbar_draw_enable = false;

#define FULLSCREEN_EXIT(menu) \
    statusbar_draw_enable = statusbar_draw_original;

#define SCREEN_SAVE \
    size_t disp_buffer_size = 8 * u8g2_GetBufferTileHeight(&u8g2) * \
            u8g2_GetBufferTileWidth(&u8g2);\
    uint8_t *old_disp_buffer = malloc(disp_buffer_size);\
    memcpy(old_disp_buffer, u8g2_GetBufferPtr(&u8g2), disp_buffer_size);

#define SCREEN_RESTORE \
    memcpy(u8g2_GetBufferPtr(&u8g2), old_disp_buffer, disp_buffer_size); \
    free(old_disp_buffer);

#endif
