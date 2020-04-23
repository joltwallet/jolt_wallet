idf_component_register(
        SRCS
            "linenoise/linenoise.c"
            "argtable3/argtable3.c"
            "./split_argv.c"
            "./commands.c"
        INCLUDE_DIRS
            "."
        REQUIRES
            "vfs"
)

