#include "string.h"
#include "jolt_gui/jolt_gui.h"

lv_obj_t *jolt_gui_scr_bignum_create(const char *title, const char *subtitle, uint32_t n) {
    JOLT_GUI_SCR_CTX(title){
        /* Create text for above the big number */
        if( NULL != subtitle ){
            lv_obj_t *header_label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
            lv_label_set_text(header_label, subtitle);
            lv_obj_align(header_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 6);
        }

        /* Create text for big number */
        char number_str[11] = { 0 }; 
        itoa( n, number_str, 10 );
        lv_obj_t *number_label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
        lv_label_set_text(number_label, number_str);

        /* Set a Big Font Style for number*/
        static lv_style_t number_style;
        lv_style_t *old_style = lv_label_get_style(number_label);
        lv_style_copy(&number_style, old_style);
        number_style.text.font = &lv_font_dejavu_40_numeric;
        lv_label_set_style(number_label, &number_style);

        /* Align Big Number to Center bottom of screen */
        lv_obj_align(number_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }

    return parent;
}

