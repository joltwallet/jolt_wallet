/**
 * @file jolt_gui_qr.h
 * @brief Display QR codes
 * @author Brian Pugh
 */

#ifndef __JOLT_GUI_QR_H__
#define __JOLT_GUI_QR_H__

#include "lvgl/lvgl.h"
#include "qrcode.h"

static const uint8_t lv_img_color_header[] = {
    0x04, 0x02, 0x04, 0xff,     /*Color of index 0*/
    0xff, 0xff, 0xff, 0xff,     /*Color of index 1*/
};

/**
 * @brief Maximum QR Version that will fit at 1 pixel per module.
 */
#define JOLT_GUI_QR_VERSION ((uint8_t)((LV_VER_RES_MAX - CONFIG_JOLT_GUI_STATUSBAR_H -17)/4))

/**
 * @brief Height/Width of maximum QR Code
 */
#define JOLT_GUI_QR_SIZE (4*JOLT_GUI_QR_VERSION+17)

/**
 * @brief Number of bytes require for qrcode internal buffer
 */
#define JOLT_GUI_QR_BUF_SIZE ((JOLT_GUI_QR_SIZE*JOLT_GUI_QR_SIZE+7)/8)

#define JOLT_GUI_QR_LVGL_IMG_BYTE_WIDTH  ((JOLT_GUI_QR_SIZE+7)/8)
#define JOLT_GUI_QR_LVGL_IMG_BUF_SIZE (JOLT_GUI_QR_SIZE*JOLT_GUI_QR_LVGL_IMG_BYTE_WIDTH+sizeof(lv_img_color_header))

/**
 * @brief Add a QR code to a scroll screen
 * @param[in,out] scr scrollable screen to add the QR code to
 * @param[in] data data to be encoded into a QR code
 * @param[in] data_len length of data
 * @return image object representing the QR code
 */
lv_obj_t *jolt_gui_scr_scroll_add_qr(lv_obj_t *scr, const char *data, uint16_t data_len);

/**
 * @brief Convenience function to create scroll screen with a QR code
 * @param[in] title title-bar string
 * @param[in] data data to be encoded into a QR code
 * @param[in] data_len length of data
 * @return scroll screen
 */
lv_obj_t *jolt_gui_scr_qr_create(const char *title, const char *data,
        uint16_t data_len);

#endif
