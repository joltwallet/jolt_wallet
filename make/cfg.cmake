# change sdkconfig defaults based on env var TARGET_BOARD

if(DEFINED ENV{TARGET_BOARD})
    set(TARGET_BOARD $ENV{TARGET_BOARD})
else()
    set(TARGET_BOARD joltv1)
endif()

# sdkconfig files that MUST exist
set(SDKCONFIG_DEFAULTS 
    "${JOLT_WALLET_PATH}/sdkconfigs/sdkconfig.defaults"
    "${JOLT_WALLET_PATH}/sdkconfigs/sdkconfig.defaults.${TARGET_BOARD}"
    )
        
# sdkconfig default files that may not exist
# files lower in the list overwrite files higher in the list
# The files that exist get appended to SDKCONFIG_DEFAULTS
set(SDKCONFIG_DEFAULTS_OPTIONAL 
        "${JOLT_WALLET_PATH}/sdkconfigs/sdkconfig.personal"
        "sdkconfig.japp"
        "sdkconfig.personal"
)
# sdkconfig.japp and sdkconfig.personal are intended for jolt apps

foreach(SDKCONFIG_FILE ${SDKCONFIG_DEFAULTS_OPTIONAL})
    if(EXISTS ${SDKCONFIG_FILE})
        list(APPEND SDKCONFIG_DEFAULTS ${SDKCONFIG_FILE})
    else()
        #message("\"${SDKCONFIG_FILE}\" doesn't exist")
    endif()
endforeach()

# Set hardware identifier
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTARGET_BOARD=\\\"${TARGET_BOARD}\\\"")
