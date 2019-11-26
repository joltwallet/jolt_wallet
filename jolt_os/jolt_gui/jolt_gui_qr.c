#include "jolt_gui.h"
#include "jolt_gui_scroll.h"
#include "jolt_helpers.h"

/* QR Screen Structure:
 * * SCREEN
 *   +--LABEL_TITLE
 *   +--CONT_BODY
 *       +--IMG_QR
 */

static const char TAG[] = "scr_qr";

#define CHECK_SCR( x ) \
    if( JOLT_GUI_SCR_ID_SCROLL != jolt_gui_scr_id_get( x ) ) abort();

typedef struct {
    lv_img_ext_t img; /*The ancestor container structure*/
    QRCode qrcode;
    lv_img_dsc_t qrcode_img;
    uint8_t img_data[JOLT_GUI_QR_LVGL_IMG_BUF_SIZE];
} qr_img_ext_t;

jolt_gui_obj_t *jolt_gui_scr_scroll_add_qr( jolt_gui_obj_t *scr, const char *data, uint16_t data_len )
{
    jolt_gui_obj_t *img = NULL;
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR( scr );

        jolt_gui_obj_t *page = BREAK_IF_NULL( jolt_gui_scr_scroll_get_page( scr ) );
        img                  = BREAK_IF_NULL( lv_img_create( page, NULL ) );

        BREAK_IF_NULL( lv_obj_allocate_ext_attr( img, sizeof( qr_img_ext_t ) ) );
        qr_img_ext_t *ext = lv_obj_get_ext_attr( img );

        /* Compute the QR Code */
        uint8_t qr_buf[JOLT_GUI_QR_BUF_SIZE];
        if( qrcode_initBytes( &( ext->qrcode ), qr_buf, JOLT_GUI_QR_VERSION, ECC_LOW, (uint8_t *)data, data_len ) ) {
            // error: too much data
            jolt_gui_obj_del( page );
            break;
        }
        jolt_gui_obj_id_set( img, JOLT_GUI_OBJ_ID_IMG_QR );
        memzero( ext->img_data, sizeof( ext->img_data ) );
        memcpy( ext->img_data, lv_img_color_header, sizeof( lv_img_color_header ) );

        uint8_t *img_data = ext->img_data + sizeof( lv_img_color_header );

        for( uint8_t y = 0; y < JOLT_GUI_QR_SIZE; y++ ) {      // iterate through rows
            for( uint8_t x = 0; x < JOLT_GUI_QR_SIZE; x++ ) {  // iterate through cols
                if( !qrcode_getModule( &( ext->qrcode ), x, y ) ) {
                    uint16_t pos = y * JOLT_GUI_QR_LVGL_IMG_BYTE_WIDTH + x / 8;
                    img_data[pos] |= ( 1 << ( 7 - ( x & 0x07 ) ) );
                }
            }
        }

        ext->qrcode_img.header.always_zero = 0;
        ext->qrcode_img.header.w           = JOLT_GUI_QR_SIZE;
        ext->qrcode_img.header.h           = JOLT_GUI_QR_SIZE;
        ext->qrcode_img.data_size          = JOLT_GUI_QR_LVGL_IMG_BUF_SIZE;
        ext->qrcode_img.header.cf          = LV_IMG_CF_INDEXED_1BIT;
        ext->qrcode_img.data               = ext->img_data;

        lv_img_set_src( img, &( ext->qrcode_img ) );
        lv_img_set_auto_size( img, true );
    }
    return img;
}

jolt_gui_obj_t *jolt_gui_scr_qr_create( const char *title, const char *data, uint16_t data_len )
{
    jolt_gui_obj_t *parent = NULL;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *label;
        parent = BREAK_IF_NULL( jolt_gui_scr_scroll_create( title ) );
        label  = jolt_gui_scr_scroll_add_qr( parent, data, data_len );
        if( NULL == label ) JOLT_GUI_OBJ_DEL_SAFE( parent );
    }
    return parent;
}
