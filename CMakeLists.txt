cmake_minimum_required(VERSION 3.10)

#    "."  # TODO: only needed for unity, need to figure out unity in cmake
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
include(${JOLT_WALLET_PATH}/make/common.cmake)

# Executables that generate/apply binary patches
set(HDIFFZ components/esp_hdiffz/HDiffPatch/hdiffz)
set(HPATCHZ components/esp_hdiffz/HDiffPatch/hpatchz)

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
    else()
        message(FATAL_ERROR "Unable to determine linux distro")
    endif()
elseif (MSVC)
    message(FATAL_ERROR "Dependency installation not yet implemented for MSVC")
else()
    message(FATAL_ERROR "Unable to determine operating system")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -DJOLT_OS")


# Dictates name of produced .elf/.bin
# Called at the end because this executes all the ESP-IDF CMake stuff
project(JoltOS)
