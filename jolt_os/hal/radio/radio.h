/**
 * @file radio.h
 * @brief High level operations for controlling the generic radio
 *
 * Primary purpose is to control radios to mitigate Screaming Channels.
 *
 * @author Brian Pugh
 * @bugs
 *     * Not implemented
 */

#ifndef JOLT_HAL_RADIO_H__
#define JOLT_HAL_RADIO_H__

#include "mp.h"

/**
 * @brief Turn off the radio amplifier
 */
void jolt_radio_off();

/**
 * @brief Turn on the radio amplifier
 */
void jolt_radio_on();

/**
 * @brief Context that turns of the radio at the beginning, and turns it back on at the end.
 */
#define JOLT_RADIO_OFF_CTX                                                        \
    MPP_BEFORE( 1, jolt_radio_off() )                                             \
    MPP_DO_WHILE( 2, false )                                                      \
    MPP_BREAK_HANDLER( 3, ESP_LOGE( TAG, "JOLT_RADIO_OFF break L%d", __LINE__ ) ) \
    MPP_FINALLY( 4, jolt_radio_on() )

#endif
