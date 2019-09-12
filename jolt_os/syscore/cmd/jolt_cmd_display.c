#include "hal/display.h"
#include "syscore/cli.h"

int jolt_cmd_display(int argc, char** argv) {
    print_display_buf();
    return 0;
}
