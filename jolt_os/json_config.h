/**
 * @file json_config.h
 * @brief cJSON wrappers
 * @author Brian Pugh
 */

#ifndef JOLT_JSON_CONFIG_H__
#define JOLT_JSON_CONFIG_H__

#include "cJSON.h"

/**
 * @brief read in and parse a compressed json file.
 *
 * @param[in] fn Fullpath to JSON file.
 * @return cJSON object representing parsed data
 */
cJSON *jolt_json_read( const char *fn );

/**
 * @brief Read the config file for the currently running app.
 * @return cJSON object representing the application's config json
 */
cJSON *jolt_json_read_app();

/**
 * @brief Writes json to a compressed file.
 * @param[in] fn Fullpath to write JSON to.
 * @param[in] json JSON object to write.
 * @return 0 on success; -1 otherwise.
 */
int jolt_json_write( const char *fn, const cJSON *json );

/**
 * @brief Write the config file for the currently running app.
 * @param[in] json JSON object to write.
 * @return 0 on success; -1 otherwise.
 */
int jolt_json_write_app( const cJSON *json );

/**
 * @brief delete a cJSON object; can be NULL
 * @param[in,out] json cJSON object to delete
 */
static inline void jolt_json_del( cJSON *json ) {
    if( NULL != json ) {
        cJSON_Delete( json );
    }
}


#endif

