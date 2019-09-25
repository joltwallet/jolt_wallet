/**
 * @file jolt_gui_indev.h
 * @brief Function to initialize and register the capacitive buttons input device.
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_INDEV_H__
#define JOLT_GUI_INDEV_H__

#include "easy_input.h"

enum {
    JOLT_BTN_UP    = EASY_INPUT_UP,
    JOLT_BTN_DOWN  = EASY_INPUT_DOWN,
    JOLT_BTN_BACK  = EASY_INPUT_BACK,
    JOLT_BTN_ENTER = EASY_INPUT_ENTER,
};
typedef uint8_t jolt_btn_t;

extern volatile uint64_t *jolt_btn_state;

/**
 * @brief Initialize and register the capcitivie buttons input device.
 */
void jolt_gui_indev_init();

#if UNIT_TESTING

    #define JOLT_BTN_DEFAULT_DURATION_MS 100

    #define JOLT_UP                                 \
        do {                                        \
            jolt_btn_press( JOLT_BTN_UP, 0, true ); \
        } while( 0 )

    #define JOLT_DOWN                                 \
        do {                                          \
            jolt_btn_press( JOLT_BTN_DOWN, 0, true ); \
        } while( 0 )

    #define JOLT_BACK                                 \
        do {                                          \
            jolt_btn_press( JOLT_BTN_BACK, 0, true ); \
        } while( 0 )

    #define JOLT_ENTER                                 \
        do {                                           \
            jolt_btn_press( JOLT_BTN_ENTER, 0, true ); \
        } while( 0 )

/**
 * @brief Simulate a button press.
 *
 * Note: Not threadsafe or safe with simulatenous physical input.
 *
 * @param[in] btn button to press
 * @param[in] duration Time in milliseconds to press the button for.
 *            0 for a default short click press.
 * @param[in] blocking Block until the button press is complete. Otherwise
 * return immediately.
 */
void jolt_btn_press( jolt_btn_t btn, uint32_t duration, bool blocking );
#endif

#endif
