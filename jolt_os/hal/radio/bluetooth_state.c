#include "bluetooth_state.h"
#include "bluetooth_cfg.h"
#include "bluetooth_gatts_profile_a.h"

#if CONFIG_BT_ENABLED
/* Array to store each application profile; will store the gatts_if returned by ESP_GATTS_REG_EVT */
gatts_profile_inst_t spp_profile_tab[SPP_PROFILE_NUM] = {
        [SPP_PROFILE_A_APP_ID] =
                {
                        .gatts_cb = gatts_profile_a_event_handler,
                        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
                },
};

uint16_t spp_handle_table[SPP_IDX_NB];

xQueueHandle ble_in_queue = NULL;

    #if CONFIG_JOLT_BT_DEBUG_ALWAYS_ADV
/* always in pairing mode for debugging */
bool jolt_bluetooth_pair_mode = true;
    #else
bool jolt_bluetooth_pair_mode = false;
    #endif

#endif
