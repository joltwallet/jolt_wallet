#ifndef JOLT_HAL_RADIO_H__
#define JOLT_HAL_RADIO_H__

#include "mp.h"

/* Used for short calls to mitigate Screaming Channels */
void jolt_radio_off();
void jolt_radio_on();

#define JOLT_RADIO_OFF_CTX \
    MPP_BEFORE(1, jolt_radio_off() ) \
    MPP_DO_WHILE(2, false) \
    MPP_BREAK_HANDLER(3, ESP_LOGE(TAG, "JOLT_RADIO_OFF break L%d", __LINE__)) \
    MPP_FINALLY(4, jolt_radio_on() )


#endif
