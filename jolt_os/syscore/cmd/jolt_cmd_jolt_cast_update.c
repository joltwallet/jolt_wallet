#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "stdio.h"
#include "syscore/cli.h"
#include "syscore/cli_helpers.h"

static char *new_uri    = NULL;
static const char TAG[] = "jolt_cmd_jolt_cast_update";

const char prompt_str[] = "Update jolt_cast server domain to:\n%s";

static void jolt_cmd_jolt_cast_cb( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        storage_set_str( new_uri, "user", "jc_uri" );
        esp_restart();
    }
    else if( jolt_gui_event.cancel == event ) {
        new_uri = NULL;
        jolt_cli_return( -1 );
    }
}

int jolt_cmd_jolt_cast_update( int argc, char **argv )
{
    char buf[sizeof( prompt_str ) + 200];

    /* Check if number of inputs is correct */
    if( !console_check_equal_argc( argc, 2 ) ) return -2;
    new_uri = argv[1];

    /* Confirm Inputs */
    snprintf( buf, sizeof( buf ), prompt_str, new_uri );

    bool success = false;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *scr;
        scr = BREAK_IF_NULL( jolt_gui_scr_text_create( "JoltCast Update", buf ) );
        jolt_gui_scr_set_event_cb( scr, jolt_cmd_jolt_cast_cb );
        success = true;
    }
    if( !success ) { return -3; }

    return JOLT_CLI_NON_BLOCKING;
}
