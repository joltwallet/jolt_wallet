#ifndef JOLT_HAL_BLUETOOTH_CFG_H__
#define JOLT_HAL_BLUETOOTH_CFG_H__
/* bluetooth_cfg holds constants that configure various bluetooth components */

#include "stdint.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

/// Characteristic UUID
#define ESP_GATT_UUID_SPP_DATA_RECEIVE      0xFFE1 // smartphone->jolt
#define ESP_GATT_UUID_SPP_DATA_NOTIFY       0xFFE2
#define ESP_GATT_UUID_SPP_COMMAND_RECEIVE   0xABF3 // smartphone->jolt
#define ESP_GATT_UUID_SPP_COMMAND_NOTIFY    0xABF4

/* todo: make this larger, will have to modify partial headers */
#define SPP_DATA_MAX_LEN           (512)
#define SPP_CMD_MAX_LEN            (20)
#define SPP_STATUS_MAX_LEN         (20)
#define SPP_SVC_INST_ID            ( 0 ) // todo; refine this?

#define SAMPLE_DEVICE_NAME          "Jolt"

extern uint16_t spp_mtu_size;

extern esp_bd_addr_t spp_remote_bda;

extern const uint8_t spp_adv_data[23];
extern const esp_ble_adv_params_t spp_adv_params;

enum{
    SPP_IDX_SVC = 0,              /* Serial Port Profile Service Index */

    SPP_IDX_SPP_DATA_RECV_CHAR,   /* SPP Data Receive Characteristic Index */
    SPP_IDX_SPP_DATA_RECV_VAL,    /* SPP Data Receive Characteristic Value Index */

    SPP_IDX_SPP_DATA_NOTIFY_CHAR, /* SPP Data Notify Characteristic Index*/
    SPP_IDX_SPP_DATA_NOTIFY_VAL,  /* SPP Data Notify Chatacteristic Value Index */
    SPP_IDX_SPP_DATA_NOTIFY_CFG,  /* SPP Data Notify Characteristic Config Index */

    SPP_IDX_SPP_COMMAND_CHAR,     /* SPP Command Characteristic Value Index */
    SPP_IDX_SPP_COMMAND_VAL,      /* */

    SPP_IDX_SPP_STATUS_CHAR,      /* */
    SPP_IDX_SPP_STATUS_VAL,       /* */
    SPP_IDX_SPP_STATUS_CFG,       /* */

    SPP_IDX_NB,                   /* Number of Table Elements */
};

/* Application Profiles 
 *     * currently there is just one called "A" (for lack of a better name).
 *         * May be renamed later if more specific profiles are required in the future
 */
enum {
    SPP_PROFILE_A_APP_ID, /* Application Profile ID */

    SPP_PROFILE_NUM, /* Number of profiles */
};

extern const esp_gatts_attr_db_t spp_gatt_db[SPP_IDX_NB];


#endif
