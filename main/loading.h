#ifndef __NANORAY_LOADING_H__
#define __NANORAY_LOADING_H__

void loading_init(menu8g2_t *menu);
void loading_disable();
void loading_task(void *menu_in);
void loading_text(char *text);

#endif
