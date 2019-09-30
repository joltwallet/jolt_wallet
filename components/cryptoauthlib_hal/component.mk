CRYPTOAUTHLIB_DIR := ../cryptoauthlib/lib

COMPONENT_SRCDIRS := $(CRYPTOAUTHLIB_DIR)/hal

COMPONENT_OBJS := $(CRYPTOAUTHLIB_DIR)/hal/atca_hal.o $(CRYPTOAUTHLIB_DIR)/hal/hal_freertos.o $(CRYPTOAUTHLIB_DIR)/hal/hal_esp32_i2c.o $(CRYPTOAUTHLIB_DIR)/hal/hal_esp32_timer.o 

COMPONENT_ADD_INCLUDEDIRS := $(CRYPTOAUTHLIB_DIR)

CFLAGS+=-DESP32 -DATCA_HAL_I2C -DATCA_PRINTF -DATCA_USE_RTOS_TIMER

cryptoauthlib/lib/hal/hal_freertos.o: CFLAGS+= -I$(IDF_PATH)/components/freertos/include/freertos
