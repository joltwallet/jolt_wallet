cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "jolt_os")

set(EXTRA_COMPONENT_DIRS
        "$ENV{IDF_PATH}/tools/unit-test-app/components"
        "jolt_os"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

#idf_build_get_property(JOLT_WALLET_PATH, PROJECT_DIR)
set(JOLT_WALLET_PATH ${CMAKE_SOURCE_DIR})
#message("Jolt wallet path is \"${JOLT_WALLET_PATH}\"")

# set targetname via `TARGET_BOARD=target`
include(${JOLT_WALLET_PATH}/make/cfg.cmake)

# Executables that generate/apply binary patches
set(HDIFFZ components/esp_hdiffz/HDiffPatch/hdiffz)
set(HPATCHZ components/esp_hdiffz/HDiffPatch/hpatchz)

idf_build_set_property(COMPILE_DEFINITIONS "-DJOLT_OS" APPEND)

#########################################
# Toolchain and Dependency Installation #
#########################################
if (WIN32)
    message(FATAL_ERROR "Dependency installation not yet implemented for WIN32")
elseif (UNIX)
    message(STATUS "Detected linux system")
    find_program(LSB_RELEASE_EXEC lsb_release)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
        OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (${LSB_RELEASE_ID_SHORT} STREQUAL "Ubuntu")
        set(INSTALL_SYSTEM_DEPENDENCIES sudo apt-get update && sudo apt-get install -y
                 bison
                 ccache
                 cmake
                 flex
                 git
                 gperf
                 libffi-dev
                 libssl-dev
                 libusb-1.0
                 libz-dev
                 ninja-build
                 protobuf-compiler
                 python3
                 python3-pip
                 python3-setuptools
                 wget
        )
    else()
        message(FATAL_ERROR "Unable to determine linux distro")
    endif()
elseif (MSVC)
    message(FATAL_ERROR "Dependency installation not yet implemented for MSVC")
else()
    message(FATAL_ERROR "Unable to determine operating system")
endif()

if(NOT DEFINED INSTALL_SYSTEM_DEPENDENCIES)
    set(INSTALL_SYSTEM_DEPENDENCIES printf "No rules for your OS" && exit 1)
endif()



##################
# CUSTOM TARGETS #
##################

add_custom_target("system_dependencies")
add_custom_command(TARGET system_dependencies
     COMMAND ${INSTALL_SYSTEM_DEPENDENCIES}
)


add_custom_target("toolchain")
add_custom_command(TARGET toolchain
    COMMAND mkdir -p $ENV{IDF_TOOLS_PATH}
    COMMAND pip3 install -r ${JOLT_WALLET_PATH}/requirements.txt
    COMMAND pip3 install -r $ENV{IDF_PATH}/requirements.txt
    COMMAND ${IDF_PATH}/tools/idf_tools.py install
    DEPENDS system_dependencies
)


add_custom_target("compress")
add_custom_command(TARGET compress
    COMMAND python3 ${JOLT_WALLET_PATH}/pyutils/ota_compress.py --input ${PROJECT_NAME}.bin
    DEPENDS
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
        ${JOLT_WALLET_PATH}/pyutils/ota_compress.py
)
add_dependencies(compress app)


add_custom_target("build-hdiffz")
add_custom_command(TARGET build-hdiffz
    COMMAND cd ${JOLT_WALLET_PATH}/components/esp_hdiffz/HDiffPatch && make
)


# the UNIT_TESTING variable is set via this project's idf_ext.py when the
# "tests" target is built. This passes it along as a COMPILE_DEFINITIONS
# so the macro is available at build time.
if( ${UNIT_TESTING} )
    idf_build_set_property(COMPILE_DEFINITIONS "-DUNIT_TESTING=1" APPEND)
endif()



project(${PROJECT_NAME})
