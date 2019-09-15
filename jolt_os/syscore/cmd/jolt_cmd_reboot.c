#include "esp_system.h"

int jolt_cmd_reboot( int argc, char** argv )
{
    esp_restart();
    return 0;
}
