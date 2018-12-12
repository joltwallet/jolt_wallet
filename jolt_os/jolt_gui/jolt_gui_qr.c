#include "jolt_gui.h"

static const uint8_t color_header[] = {
    0x04, 0x02, 0x04, 0xff,     /*Color of index 0*/
    0xff, 0xff, 0xff, 0xff,     /*Color of index 1*/
};

/* Convert a QRCode object into a lv_img_dsc_t object.
 * Will malloc space for image data and descriptor.
 * QRCode doesn't need to exist after this call for the image to work.*/
static lv_img_dsc_t *jolt_gui_qr_to_img_dsc(QRCode *qrcode) {
    uint8_t byte_width = (qrcode->size + 7) / 8; // Number of bytes to store a row
	uint16_t data_size = (qrcode->size)*byte_width + sizeof(color_header);
    uint8_t *data = calloc(data_size, sizeof(uint8_t));
    if( NULL == data ) {
        return NULL;
    }
    lv_img_dsc_t *qrcode_img = lv_mem_alloc(sizeof(lv_img_dsc_t));
    if( NULL == qrcode_img ) {
        free(data);
        return NULL;
    }

    memcpy(data, color_header, sizeof(color_header));
    uint8_t *img = data + sizeof(color_header);

    for(uint8_t y=0; y < qrcode->size; y++){ // iterate through rows
        for(uint8_t x=0; x < qrcode->size; x++){ // iterate through cols
            if(!qrcode_getModule(qrcode, x, y)){
                uint16_t pos = y*byte_width + x/8;
                img[pos] |= (1 << (7 - (x & 0x07)) );
            }
        }
    }

    qrcode_img->header.always_zero = 0;
    qrcode_img->header.w = qrcode->size;
    qrcode_img->header.h = qrcode->size;
    qrcode_img->data_size = data_size;
    qrcode_img->header.cf = LV_IMG_CF_INDEXED_1BIT;
    qrcode_img->data = data;

    return qrcode_img;
}

/* Callback for back button */
static lv_action_t delete_screen(lv_obj_t *btn) {
    /* free memory use by img */
    lv_obj_t *img = lv_obj_get_parent(btn);
    lv_img_ext_t *ext = lv_obj_get_ext_attr(img);
    uint8_t *data = ((lv_img_dsc_t *)(ext->src))->data;

    lv_img_set_src( img, NULL );
    free(data);

    jolt_gui_scr_del();
    return 0;
}

static lv_obj_t *jolt_gui_qr_fullscreen_create( const char *title, 
        lv_img_dsc_t *qrcode_img) {
    lv_obj_t *parent = jolt_gui_parent_create();
    if( NULL == parent ) goto exit;


    lv_obj_t *cont = lv_cont_create(parent, NULL);
    if( NULL == cont ) goto exit;
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES-CONFIG_JOLT_GUI_STATUSBAR_H);
    lv_obj_align(cont, jolt_gui_store.statusbar.container, 
            LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    lv_obj_t *img = lv_img_create(cont, NULL);
    if( NULL == img ) goto exit;
    lv_img_set_src( img, qrcode_img );
    lv_img_set_auto_size(img, true);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_group_add_obj(jolt_gui_store.group.main, parent);
    lv_group_focus_obj(parent);

    /* Set img as parent of back action so that we can easily get img
     * attributes while deleting */
    if( NULL == jolt_gui_scr_set_back_action(img, delete_screen) ) goto exit;
    if( NULL == jolt_gui_scr_set_enter_action(parent, NULL) ) goto exit;

    if( NULL == jolt_gui_obj_title_create(parent, title) ) goto exit;

    return parent;
exit:
    if( parent ) {
        lv_obj_del(parent);
    }
    return NULL;
}

/* Creates a screen display the qr code for the data.
 * If you are passing in a string, strlen(string) is good enough for data_len,
 * no need to compensate for the null pointer.
 * Returns the parent dummy object on success.
 * Returns NULL on failure. */
lv_obj_t *jolt_gui_scr_qr_create(const char *title, const char *data,
        uint16_t data_len) {
    // compute largest version that will fit on the screen
    QRCode qrcode;
    uint8_t qr_buf[JOLT_GUI_QR_BUF_SIZE];

    /* Compute the QR Code */
    if( qrcode_initBytes(&qrcode, qr_buf, JOLT_GUI_QR_VERSION, ECC_LOW, 
                data, data_len) ) {
        // too much data
        return NULL;
    }
    lv_img_dsc_t *img = jolt_gui_qr_to_img_dsc(&qrcode);
    if( NULL == img ) {
        return NULL;
    }
	lv_obj_t *scr = jolt_gui_qr_fullscreen_create(title, img);
    if( NULL == scr ) {
    }
    return scr;
}
