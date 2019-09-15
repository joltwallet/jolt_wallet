#include "jolt_gui/menus/settings/submenus.h"
#include "syscore/cli.h"

int jolt_cmd_factory_reset( int argc, char** argv )
{
    menu_factory_reset_create( NULL, jolt_gui_event.short_clicked );

    return 0;
}
