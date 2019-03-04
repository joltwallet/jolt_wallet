/**
 * @file bluetooth_cfg.h
 * @brief BLE configuration constants
 * @author Brian Pugh
 */

#ifndef JOLT_HAL_BLUETOOTH_CFG_H__
#define JOLT_HAL_BLUETOOTH_CFG_H__

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED
#include "stdint.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

#define ESP_GATT_UUID_SPP_DATA_NOTIFY       0xFFE2 // jolt->smartphone
#define ESP_GATT_UUID_SPP_COMMAND_RECEIVE   0xABF3 // smartphone->jolt

#define SPP_DATA_MAX_LEN           (512)
#define SPP_CMD_MAX_LEN            (20)
#define SPP_STATUS_MAX_LEN         (20)
#define SPP_SVC_INST_ID            ( 0 ) // todo; refine this?

#define SAMPLE_DEVICE_NAME          "Jolt"

extern uint16_t spp_mtu_size;

/**
 * @brief RAW Advertising Data
 */
extern const uint8_t spp_adv_data[23];
/**
 * @brief Advertising pair parameters
 */
extern const esp_ble_adv_params_t spp_adv_pair_params;
/**
 * @brief Advertising whitelist parameters
 */
extern const esp_ble_adv_params_t spp_adv_wht_params;

/**
 * @brief Attribute indices into the Characteristic Table
 *
 * For most types of data in the GATT hierarchy, it is important to differentiate between their definition (the whole group of attributes that make it up) and the declaration. The declaration is a single attribute that is always placed first (in increasing handle order) within the definition and that introduces most of the metadata about the data that follows. All declarations have read-only permissions with no security required, because they cannot contain sensitive data. They
 * are only structural attributes that allow the client to find out and discover the layout and nature of the attributes on the server.
 *
 * Source: https://www.oreilly.com/library/view/getting-started-with/9781491900550/ch04.html*/
enum{
    SPP_IDX_SVC = 0,              /**< Serial Port Profile Service Index */

    SPP_IDX_SPP_DATA_NOTIFY_DECL, /**< SPP Data Notify Declaration Index*/
    SPP_IDX_SPP_DATA_NOTIFY_VAL,  /**< SPP Data Notify Chatacteristic Value Index */
    SPP_IDX_SPP_DATA_NOTIFY_CFG,  /**< SPP Data Notify Characteristic Config Index */

    SPP_IDX_SPP_COMMAND_DECL,     /**< SPP Command Declaration Value Index */
    SPP_IDX_SPP_COMMAND_VAL,      /**< SPP Command Characteristic Value Index */

    SPP_IDX_NB,                   /**< Number of Table Elements */
};

/**
 * @brief Application Profiles Identifiers
 *
 *  Currently there is just one called "A" (for lack of a better name).
 *  Might be renamed later if more specific profiles are required in the future.
 */
enum {
    SPP_PROFILE_A_APP_ID, /**< Application Profile ID */

    SPP_PROFILE_NUM,      /**< Number of profiles */
};

/**
 * @brief Profile A Attribute Table
 */
extern const esp_gatts_attr_db_t spp_gatt_db[SPP_IDX_NB];

#endif

#endif
