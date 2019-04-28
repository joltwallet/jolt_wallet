/**
 * @file jolt_gui_err.h
 * @brief Convert generic error codes to GUI screens
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_SCR_ERR_H__
#define JOLT_GUI_SCR_ERR_H__

typedef enum {
    JOLT_GUI_ERR_UNKNOWN = 1,
    JOLT_GUI_ERR_NOT_IMPLEMENTED,
    JOLT_GUI_ERR_WIFI_NO_DRIVERS,
    JOLT_GUI_ERR_WIFI_OFF,
    JOLT_GUI_ERR_BLUETOOTH_NO_DRIVERS,
    JOLT_GUI_ERR_BLUETOOTH_OFF,
    JOLT_GUI_ERR_OOM,
} jolt_gui_err_t;


/**
 * @brief convert a Jolt GUI error code into a NULL-terminated string
 * @param[in] err GUI error code
 * @return NULL-terminated error string
 */
char *jolt_gui_err_to_str( jolt_gui_err_t err );

/**
 * @brief Create an error screen for a given error code
 * @param[in] err GUI error code
 * @return error screen
 */
jolt_gui_obj_t *jolt_gui_scr_err_create( jolt_gui_err_t err );

#endif
