#ifndef __NANORAY_GUI_H__
#define __NANORAY_GUI_H__

void setup_screen(u8g2_t *u8g2);
void gui_task();

#define FULLSCREEN_ENTER(menu) \
    menu->post_draw = NULL; \
    bool statusbar_draw_original = statusbar_draw_enable; \
    statusbar_draw_enable = false;

#define FULLSCREEN_EXIT(menu) \
    statusbar_draw_enable = statusbar_draw_original;

#endif
