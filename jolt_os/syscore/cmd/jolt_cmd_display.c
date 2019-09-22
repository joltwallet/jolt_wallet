#include "hal/display.h"
#include "syscore/cli.h"

int jolt_cmd_display( int argc, char** argv )
{
    jolt_display_print( NULL );
    jolt_cli_resume();
    return 0;
}
