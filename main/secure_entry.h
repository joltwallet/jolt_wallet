#ifndef __NANORAY_SECURE_ENTRY_H__
#define __NANORAY_SECURE_ENTRY_H__

#define MAX_PIN_DIGITS 9

bool pin_entry(menu8g2_t *menu, unsigned char *pin_hash, const char *title);

#endif
