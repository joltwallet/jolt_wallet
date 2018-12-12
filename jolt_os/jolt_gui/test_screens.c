#if PC_SIMULATOR
#include "jolt_gui.h"
#include "test_stubs.h"
#include "test_screens.h"

static lv_task_t *loading;

static lv_action_t loading_action();


lv_res_t jolt_gui_test_text_create(lv_obj_t * list_btn) {
    /* Dummy Text Page for Testing */
    jolt_gui_text_create("Test", 
            "Would you like to send 1337 Nano to "
            "xrb_1nanode8ngaakzbck8smq6ru9bethqwyehomf79sae1k7xd47dkidjqzffeg "
            "more text; should investigate changing the word wrapping for "
            "addresses.");
    return LV_RES_OK;
}

lv_res_t jolt_gui_test_alphabet_create(lv_obj_t * list_btn) {
    /* Dummy Text Page for Testing */
    jolt_gui_text_create("Alphabet", 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
            "abcdefghijklmnopqrstuvwxyz "
            "1234567890 "
            "!@#$%^&*()-+_="
            "{}[];':\",.<>?/\\"
            );
    return LV_RES_OK;
}


lv_res_t jolt_gui_test_submenu_create(lv_obj_t * list_btn) {
    /*Create the list*/
    lv_obj_t *settings_list = jolt_gui_menu_create("Settings",
            NULL, "WiFi", list_release_action);
    lv_list_add(settings_list, NULL, "Bluetooth", list_release_action);
    lv_list_add(settings_list, NULL, "Long Option Name Scrolls",
            list_release_action);
    lv_list_add(settings_list, NULL, "Factory Reset", list_release_action);
    return LV_RES_OK;
}

static lv_action_t pin_entry_cb(lv_obj_t roller) {
    uint8_t pin_array[CONFIG_JOLT_GUI_PIN_LEN] = { 0 };
    // Parse and zero out the pin array
    printf("Entered PIN: ");
    for(uint8_t i=0; i < sizeof(pin_array); i++) {
        pin_array[i] = 9 - (lv_roller_get_selected(jolt_gui_store.digit.rollers[i]) % 10);
        //todo: securely zero out the rollers
        //pin_array[i] = lv_roller_get_selected(jolt_gui_store.digit.rollers[i]);
        printf("%d ", pin_array[i]);
    }
    printf("\n");

    jolt_gui_delete_current_screen();

    // todo: call the caller callback

    return 0;
}

lv_res_t jolt_gui_test_pin_create( void ) {
     jolt_gui_numeric_create( CONFIG_JOLT_GUI_PIN_LEN, JOLT_GUI_NO_DECIMAL,
             "PIN (1/10)", pin_entry_cb); 
     return LV_RES_OK;
}

lv_res_t jolt_gui_test_numeric_end_dp_create( void ) {
     jolt_gui_numeric_create( 5, 0, "Decimal End", NULL); 
     return LV_RES_OK;
}
lv_res_t jolt_gui_test_numeric_begin_dp_create( void ) {
     jolt_gui_numeric_create( 4, 4, "Decimal Begin", NULL); 
     return LV_RES_OK;
}
lv_res_t jolt_gui_test_numeric_mid_dp_create( void ) {
     jolt_gui_numeric_create( 6, 2, "Decimal 2", NULL); 
     return LV_RES_OK;
}

lv_res_t jolt_gui_test_loading_create(lv_obj_t *list_btn) {
    jolt_gui_loading_create("Loading Test");
    loading = lv_task_create(loading_action, 500, LV_TASK_PRIO_HIGH, NULL);
    return LV_RES_OK;
}

static lv_action_t loading_action() {
    static uint8_t i = 0;
    switch(i) {
        case 10:
            jolt_gui_loading_update(i, "Contacting Server");
            break;
        case 30:
            jolt_gui_loading_update(i, "Checking Account");
            break;
        case 40:
            jolt_gui_loading_update(i, "Crafting Transaction");
            break;
        case 50:
            jolt_gui_loading_update(i, "Signing");
            break;
        case 60:
            jolt_gui_loading_update(i, "Broadcasting");
            break;
        case 100:
            jolt_gui_loading_update(i, "Complete!");
            break;
    }
    i += 10;
    if( 110 == i ) {
        lv_task_del(loading);
    }
}

#endif
