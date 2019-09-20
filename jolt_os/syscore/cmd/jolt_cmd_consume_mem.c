//#define LOG_LOCAL_LEVEL 4

#include "sdkconfig.h"

#if JOLT_GUI_TEST_MENU

    #include "cJSON.h"
    #include "esp_log.h"
    #include "jolt_helpers.h"
    #include "stdio.h"

// static const char TAG[] = "jolt_cmd_consume_mem";

int jolt_cmd_consume_mem( int argc, char **argv )
{
    static void *consumed = NULL;
    if( consumed ) {
        /* free */
        if( 1 != argc ) return -1;
        jolt_consume_mem_free( consumed );
        consumed = NULL;
    }
    else {
        /* consume */
        if( 2 != argc ) return -1;
        int remain = atoi( argv[1] );
        if( remain == 0 ) return -1;
        consumed = jolt_consume_mem( remain, 128 );
        if( NULL == consumed ) return -1;
    }
    return 0;
}

#endif
