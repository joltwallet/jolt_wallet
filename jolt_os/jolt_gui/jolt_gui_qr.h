#ifndef __JOLT_GUI_QR_H__
#define __JOLT_GUI_QR_H__

#include "lvgl/lvgl.h"
#include "qrcode.h"

// Maximum size qr code version that will fit at 1 pixel per modules
#define JOLT_GUI_QR_VERSION ((uint8_t)((LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H -17)/4))

// Height/Width of QR Code
#define JOLT_GUI_QR_SIZE (4*JOLT_GUI_QR_VERSION+17)

// Number of bytes require for qrcode internal buffer
#define JOLT_GUI_QR_BUF_SIZE ((JOLT_GUI_QR_SIZE*JOLT_GUI_QR_SIZE+7)/8)

lv_obj_t *jolt_gui_scr_qr_create(const char *title, const char *data,
        uint16_t data_len);

#endif
