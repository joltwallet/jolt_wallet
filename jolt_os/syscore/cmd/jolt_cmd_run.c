#include "stdlib.h"
#include "syscore/launcher.h"

int jolt_cmd_run(int argc, char** argv) {
    /* Takes in 1 argument (elf_fn, )
     * the elf suffix will be added to elf_fn.
     * if entry_point is not provided, defaults to app_main
     */
    int return_code;

    int app_argc = argc - 2;
    char **app_argv = NULL;
    if( app_argc <= 0 ) {
        app_argc = 0;
    }
    else{
        app_argv = argv + 2;
    }

    return_code = launch_file(argv[1], app_argc, app_argv);

    return return_code;
}


