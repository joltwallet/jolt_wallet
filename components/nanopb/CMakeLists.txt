idf_component_register(
        SRCS
            "nanopb/pb_encode.c"
            "nanopb//pb_decode.c"
            "nanopb//pb_common.c"
        INCLUDE_DIRS
            "."
            "nanopb/"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -DPB_ENABLE_MALLOC)
