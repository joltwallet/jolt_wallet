set(CRYPTOAUTHLIB_DIR "cryptoauthlib/lib")

idf_component_register(
		SRCS
			"${CRYPTOAUTHLIB_DIR}/hal/atca_hal.c"
			"${CRYPTOAUTHLIB_DIR}/hal/hal_freertos.c"
			"${CRYPTOAUTHLIB_DIR}/hal/hal_esp32_i2c.c"
			"${CRYPTOAUTHLIB_DIR}/hal/hal_esp32_timer.c"
	   INCLUDE_DIRS
		   "${CRYPTOAUTHLIB_DIR}"
	   REQUIRES
	   PRIV_REQUIRES
)

target_compile_options(${COMPONENT_LIB}
		PRIVATE
			-DESP32
			-DATCA_HAL_I2C
			-DATCA_PRINTF
			-DATCA_USE_RTOS_TIMER
)

set_source_files_properties("${CRYPTOAUTHLIB_DIR}/hal/hal_freertos.c"
    PROPERTIES COMPILE_FLAGS
	-I${IDF_PATH}/components/freertos/include/freertos
)
