#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

#include "bluetooth.h"
#include "bluetooth_cfg.h"
#include "bluetooth_state.h"
#include "bluetooth_gatts_profile_a.h"
#include "hal/radio/spp_recv_buf.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <esp_timer.h>


static const char TAG[] = "GATTS_A";
static bool is_connected = false;

bool gatts_profile_a_is_connected() {
    return is_connected;
}

static uint8_t find_char_and_desr_index(uint16_t handle) {
    for(int i = 0; i < SPP_IDX_NB ; i++) {
        if( handle == spp_handle_table[i]) {
            return i;
        }
    }
    return 0xff; // error
}

static const char* gatts_evt_to_str(esp_gap_ble_cb_event_t event) {
#define CASE(x) case x: return #x;
    switch(event) {
        CASE(ESP_GATTS_REG_EVT);
        CASE(ESP_GATTS_READ_EVT);
        CASE(ESP_GATTS_WRITE_EVT);
        CASE(ESP_GATTS_EXEC_WRITE_EVT);
        CASE(ESP_GATTS_MTU_EVT);
        CASE(ESP_GATTS_CONF_EVT);
        CASE(ESP_GATTS_UNREG_EVT);
        CASE(ESP_GATTS_CREATE_EVT);
        CASE(ESP_GATTS_ADD_INCL_SRVC_EVT);
        CASE(ESP_GATTS_ADD_CHAR_EVT);
        CASE(ESP_GATTS_ADD_CHAR_DESCR_EVT);
        CASE(ESP_GATTS_DELETE_EVT);
        CASE(ESP_GATTS_START_EVT);
        CASE(ESP_GATTS_STOP_EVT);
        CASE(ESP_GATTS_CONNECT_EVT);
        CASE(ESP_GATTS_DISCONNECT_EVT);
        CASE(ESP_GATTS_OPEN_EVT);
        CASE(ESP_GATTS_CANCEL_OPEN_EVT);
        CASE(ESP_GATTS_CLOSE_EVT);
        CASE(ESP_GATTS_LISTEN_EVT);
        CASE(ESP_GATTS_CONGEST_EVT);
        CASE(ESP_GATTS_RESPONSE_EVT);
        CASE(ESP_GATTS_CREAT_ATTR_TAB_EVT);
        CASE(ESP_GATTS_SET_ATTR_VAL_EVT);
        CASE(ESP_GATTS_SEND_SERVICE_CHANGE_EVT);
        default:
            return "<unknown>";
    }
#undef CASE
}


/* Only used to handle events for SPP_PROFILE_A_APP_ID */
void IRAM_ATTR gatts_profile_a_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t *) param;
    uint8_t res = 0xff;

    ESP_LOGI(TAG, "GATTS event %d: %s", event, gatts_evt_to_str(event));
    switch (event) {
    	case ESP_GATTS_REG_EVT:
            /* Triggers when an application is registered */
        	esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
            esp_ble_gap_config_local_privacy(true);
        	esp_ble_gap_config_adv_data_raw((uint8_t *)spp_adv_data,
                    sizeof(spp_adv_data));
        	esp_ble_gatts_create_attr_tab(spp_gatt_db, gatts_if, 
                    SPP_IDX_NB, SPP_SVC_INST_ID);
       	    break;
    	case ESP_GATTS_READ_EVT:
            /* gatt client request read operation */
            res = find_char_and_desr_index(p_data->read.handle);
            ESP_LOGI(TAG, "ESP_GATTS_READ_EVT : handle = %d\n", res);
       	    break;
    	case ESP_GATTS_WRITE_EVT: {
            /*  gatt client request write operation */
    	    res = find_char_and_desr_index(p_data->write.handle);
            if(p_data->write.is_prep == false){
                ESP_LOGI(TAG, "ESP_GATTS_WRITE_EVT : handle = %d\n", res);
                if(res == SPP_IDX_SPP_COMMAND_VAL){
                    /* Forward packet to the ble_in_queue */
                    ble_packet_t packet = { 0 };
#if CONFIG_JOLT_BT_PROFILING
                    packet.t_receive = esp_timer_get_time();
#endif
                    ESP_LOGI(TAG, "SPP_IDX_SPP_COMMAND_VAL;"
                            " Allocating %d bytes.", p_data->write.len);
                    // May need to revert this to allocate mtu
                    packet.data = (uint8_t *)malloc(p_data->write.len);
                    if(packet.data == NULL){
                        ESP_LOGE(TAG, "%s malloc failed\n", __func__);
                        break;
                    }
                    packet.len = p_data->write.len;
                    memcpy(packet.data, p_data->write.value, p_data->write.len);
                    if(!xQueueSend(ble_in_queue, &packet, 10/portTICK_PERIOD_MS)){
                        ESP_LOGE(TAG, "Timed out trying to put packet onto ble_in_queue");
                        free(packet.data);
                        break;
                    }
                }
                else if(res == SPP_IDX_SPP_DATA_NOTIFY_CFG){
                    ESP_LOGI(TAG, "SPP_IDX_SPP_DATA_NOTIFY_CFG");
                    if( (p_data->write.len == 2)
                            &&(p_data->write.value[0] == 0x01)
                            &&(p_data->write.value[1] == 0x00) ) {
                    }
                    else if( (p_data->write.len == 2)
                            &&(p_data->write.value[0] == 0x00)
                            &&(p_data->write.value[1] == 0x00) ) {
                    }
                }
                else{
                    ESP_LOGI(TAG, "Unknown state machine attribute %d.",
                            res);
                    //TODO:
                }
            }
            else if( (p_data->write.is_prep == true)
                    && (res == SPP_IDX_SPP_COMMAND_VAL) ) {
                ESP_LOGI(TAG, "ESP_GATTS_PREP_WRITE_EVT : handle = %d\n", res);
                store_wr_buffer(p_data);
            }
      	 	break;
    	}
    	case ESP_GATTS_EXEC_WRITE_EVT:{
            /* gatt client request execute write.
             * Performs the full atmoic write that has been queued up
             * by multiple "prepare write". */
    	    ESP_LOGI(TAG, "ESP_GATTS_EXEC_WRITE_EVT\n");
    	    if(p_data->exec_write.exec_write_flag){
    	        print_write_buffer();
    	        free_write_buffer();
    	    }
    	    break;
    	}
    	case ESP_GATTS_MTU_EVT:
            /* set mtu complete */
            ESP_LOGI(TAG, "ESP_GATTS_MTU_EVENT;"
                    "Setting MTU size to %d bytes.", p_data->mtu.mtu);
    	    spp_mtu_size = p_data->mtu.mtu;
    	    break;
    	case ESP_GATTS_CONF_EVT:
    	    break;
    	case ESP_GATTS_UNREG_EVT:
        	break;
    	case ESP_GATTS_DELETE_EVT:
        	break;
    	case ESP_GATTS_START_EVT:
        	break;
    	case ESP_GATTS_STOP_EVT:
        	break;
    	case ESP_GATTS_CONNECT_EVT: {
            /* When gatt client connects */
    	    is_connected = true;
            if(jolt_bluetooth_pair_mode) {
                /* Allow connections from non-bonded devices */
                esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
            }
            else {
                /* Reject Connection if it's not a bonded addr */
                int dev_num = esp_ble_get_bond_device_num();

                esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
                esp_ble_get_bond_device_list(&dev_num, dev_list);

                bool found = false; 
                for(uint8_t i=0 ; i < dev_num; i++) {
                    if( 0==memcmp( param->connect.remote_bda, dev_list[i].bd_addr, sizeof(esp_bd_addr_t )) ) {
                        found = true;
                        break;
                    }
                }
                free(dev_list);
                if(!found) {
                    /* Disconnect from the device */
                    esp_ble_gap_disconnect(param->connect.remote_bda);
                    ESP_LOGI(TAG, "Unknown device attempted to connect while not in pairing mode.");
                }
                else {
                    esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
                }
            }
        	break;
        }
    	case ESP_GATTS_DISCONNECT_EVT:
            /* When gatt client disconnect */
    	    is_connected = false;
            /* Start looking for whitelisted devices again */
            esp_ble_gap_start_advertising( (esp_ble_adv_params_t *)&spp_adv_wht_params );
    	    break;
    	case ESP_GATTS_OPEN_EVT:
    	    break;
    	case ESP_GATTS_CANCEL_OPEN_EVT:
    	    break;
    	case ESP_GATTS_CLOSE_EVT:
    	    break;
    	case ESP_GATTS_LISTEN_EVT:
    	    break;
    	case ESP_GATTS_CONGEST_EVT:
    	    break;
    	case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
    	    ESP_LOGI( TAG, "The number handle =%x\n",
                    param->add_attr_tab.num_handle );
    	    if ( param->add_attr_tab.status != ESP_GATT_OK ) {
    	        ESP_LOGE(TAG, 
                        "Create attribute table failed, error code=0x%x",
                        param->add_attr_tab.status);
    	    }
    	    else if (param->add_attr_tab.num_handle != SPP_IDX_NB){
                ESP_LOGE(TAG, "Create attribute table abnormally, "
                        "num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
                        param->add_attr_tab.num_handle, SPP_IDX_NB);
    	    }
    	    else {
    	        memcpy(spp_handle_table, param->add_attr_tab.handles,
                        sizeof(spp_handle_table));
    	        esp_ble_gatts_start_service(spp_handle_table[SPP_IDX_SVC]);
    	    }
    	    break;
    	}
    	default:
    	    break;
    }
}
#endif

