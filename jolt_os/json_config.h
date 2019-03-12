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
 * @param[in] fn Path to JSON file (probably starts with "/spiffs/").
 * @return cJSON object representing parsed data
 */
cJSON *jolt_json_read( const char *fn );

/**
 * @brief Writes json to a compressed file (probably starts with "/spiffs/").
 * @param[in] fn Path to write JSON to.
 * @return 0 on success; -1 otherwise.
 */
int jolt_json_write( const char *fn, const cJSON *json );

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

