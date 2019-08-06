#include "string.h"
#include "jolt_gui/jolt_gui.h"

static bool first = true;
static lv_style_t bignum_style;

#define BUF_LEN 11

jolt_gui_obj_t *jolt_gui_scr_bignum_create(const char *title, const char *subtitle, uint32_t n, int8_t n_digits) {
    if(n_digits > BUF_LEN-1){
        /* Insufficient Buffer */
        return NULL;
    }
    JOLT_GUI_SCR_CTX(title){
        /* Create text for above the big number */
        if( NULL != subtitle ){
            jolt_gui_obj_t *header_label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
            lv_label_set_text(header_label, subtitle);
            lv_obj_align(header_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 6);
        }

        /* Create text for big number */
        char number_str[BUF_LEN] = { 0 };
        if( n_digits > 0 ) {
            sprintf(number_str, "%0*d", n_digits, n);
        }
        else{
            itoa( n, number_str, 10 );
        }
        jolt_gui_obj_t *number_label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
        lv_label_set_text(number_label, number_str);
        jolt_gui_group_add(number_label);

        /* Set a Big Font Style for number*/
        if( first ) {
            first = false;
            lv_style_copy(&bignum_style, lv_label_get_style(number_label, LV_LABEL_STYLE_MAIN));
            bignum_style.text.font = JOLT_GUI_FONT_BIG_NUM;
            bignum_style.text.letter_space = 3;
        }

        lv_label_set_style(number_label, LV_LABEL_STYLE_MAIN, &bignum_style);

        /* Align Big Number to Center bottom of screen */
        lv_obj_align(number_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

        /* Default back action is to delete screen */
        jolt_gui_scr_set_event_cb(parent, jolt_gui_event_del);
    }

    return parent;
}

