#if JOLT_GUI_TEST_MENU
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/test_screens.h"
#include "jolt_helpers.h"
#include <driver/adc.h>

#include "json_config.h"
#include "cJSON.h"

#include "syscore/https.h"

static const char TAG[] = "test_screens";

lv_res_t jolt_gui_test_json_create(lv_obj_t *btn) {
#define EXIT_IF_NULL(x) if( NULL == x ) goto exit;
    const char fn[] = "/spiffs/test.json";
    cJSON *json;

    json = jolt_json_read( fn );

    if( NULL == json ) {
        ESP_LOGI(TAG, "No json file found; creating.");
        json = cJSON_CreateObject();
        EXIT_IF_NULL( cJSON_AddStringToObject(json, "testStringKey", "testStringVal") );
        EXIT_IF_NULL( cJSON_AddNumberToObject(json, "testIntKey", 123456789) );
        jolt_json_write( fn, json );
    }

    /* Print stuff to stdout*/
    {
        cJSON *obj;
        obj = cJSON_GetObjectItemCaseSensitive(json, "testStringKey");
        printf("testStringKey: %s\n", cJSON_GetStringValue(obj) );
    }

exit:
    jolt_json_del(json);
    return LV_RES_OK;
#undef EXIT_IF_NULL
}

static lv_res_t jolt_gui_test_number_enter_cb(lv_obj_t *parent){
    double d_val = jolt_gui_scr_digit_entry_get_double(parent);
    uint32_t i_val = jolt_gui_scr_digit_entry_get_int(parent);
    ESP_LOGI(TAG, "Entry as Double: %f", d_val);
    ESP_LOGI(TAG, "Entry as Integer: %d", i_val);
    return jolt_gui_scr_del( parent );
}

lv_res_t jolt_gui_test_number_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_digit_entry_create( "Number Test", 7, 2); 
    /* Set the digit left of the dp */
    jolt_gui_scr_digit_entry_set_pos(scr, 2);
    jolt_gui_scr_set_enter_action(scr, jolt_gui_test_number_enter_cb);
    return LV_RES_OK;
}

lv_res_t jolt_gui_test_qrcode_create(lv_obj_t *btn) {
    const char data[] = "Meow";
    lv_obj_t *scr = jolt_gui_scr_qr_create("QR Test", "Meow", sizeof(data));
	return LV_RES_OK;
}

void test_loading_task(void *param) {
    lv_obj_t *scr = (lv_obj_t *)param;
    for(uint8_t i=0;i < 101; vTaskDelay(pdMS_TO_TICKS(1000)), i+=10){
        if(i==50){
            jolt_gui_scr_loadingbar_update(scr, "Almost Done", "woof", i);
        }
        else if(i>50){
            jolt_gui_scr_loadingbar_update(scr, NULL, "bark", i);
        }
        else{
            jolt_gui_scr_loadingbar_update(scr, NULL, "meow", i);
        }
    }
    lv_obj_del(scr);
    vTaskDelete(NULL);
}

lv_res_t jolt_gui_test_loading_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_loadingbar_create("Loading Test");
    if(NULL == scr){
        ESP_LOGE(TAG, "NULL Loading Screen");
        return 1;
    }
    xTaskCreate(test_loading_task,
                "TestLoading", 4096,
                (void *) scr, 10, NULL);
    return LV_RES_OK;
}


/* Screen that gives info on the battery */
lv_task_t *test_battery_task_h = NULL;
lv_obj_t *test_battery_scr = NULL;

lv_res_t jolt_gui_test_battery_del(lv_obj_t *btn) {
    lv_task_del(test_battery_task_h);
    lv_obj_del(test_battery_scr);
    return LV_RES_INV;
}
void jolt_gui_test_battery_task(void *param) {
    if(NULL != test_battery_scr) {
        lv_obj_del(test_battery_scr);
    }
    int val = adc1_get_raw(JOLT_ADC1_VBATT);
    char buf[40];
    snprintf(buf, sizeof(buf), "Raw Value: %d\nPercentage: %d", val,
            statusbar_indicators[JOLT_HW_MONITOR_INDEX_BATTERY].val);
    test_battery_scr = jolt_gui_scr_text_create("Battery", buf);
    jolt_gui_scr_set_back_action(test_battery_scr, jolt_gui_test_battery_del);
}

lv_res_t jolt_gui_test_battery_create(lv_obj_t *btn) {
    test_battery_task_h = lv_task_create(jolt_gui_test_battery_task, 300, LV_TASK_PRIO_LOW, NULL);
    return LV_RES_OK;
}

lv_res_t jolt_gui_test_alphabet_create(lv_obj_t * list_btn) {
    /* Dummy Text Page for Testing */
    lv_obj_t *scr = jolt_gui_scr_text_create("Alphabet", 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
            "abcdefghijklmnopqrstuvwxyz "
            "1234567890 "
            "!@#$%^&*()-+_="
            "{}[];':\",.<>?/\\"
            " ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"
            "The Quick Brown Fox, Jumped Over The Lazy Dog.\n"
            "Testing proper wrap around of very long words and address. "
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ."
            );
    jolt_gui_scr_scroll_add_monospace_text(scr,
            "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
    return LV_RES_OK;
}

static void https_cb(int16_t status_code, char *post_response, void *params, lv_obj_t *scr) {
    if( 200 == status_code ){
        printf("Response:\n");
        printf(post_response);
        printf("\n");
    }
    if( NULL != post_response ) {
        free(post_response);
    }
    jolt_gui_obj_del(scr);
}

lv_res_t jolt_gui_test_https_create( lv_obj_t *btn ) {
    lv_obj_t *scr = jolt_gui_scr_preloading_create("Test", "HTTPS Test");
    const char post_data[]  = "{ \"action\" : \"block_count\"}";
    jolt_network_post( post_data, https_cb, NULL, scr );
    return LV_RES_OK;
}

#endif
