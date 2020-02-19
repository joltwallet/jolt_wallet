# To be included in a Jolt App's Makefile

DEFAULT_BIP32_KEY = ed25519 seed

# Check all required variables are set
ifndef PROJECT_NAME
$(error PROJECT_NAME is not set)
endif
ifndef COIN_PATH
$(error COIN_PATH is not set. It should look like 44'/0')
endif
ifndef BIP32_KEY
$(warning BIP32_KEY not set. Using default $(DEFAULT_BIP32_KEY))
endif
ifndef APP_VERSION_MAJOR
$(error APP_VERSION_MAJOR is not set)
endif
ifndef APP_VERSION_MINOR
$(error APP_VERSION_MINOR is not set)
endif
ifndef APP_VERSION_PATCH
$(error APP_VERSION_PATCH is not set)
endif


# Parse output binary paths
ELF_BIN_NAME             = $(PROJECT_PATH)/$(PROJECT_NAME).elf
JELF_BIN_NAME            = $(PROJECT_PATH)/$(PROJECT_NAME).jelf

# Defaults; Applications should explicitly specify BIP32_KEY
BIP32_KEY    ?= $(DEFAULT_BIP32_KEY)
ELF2JELF     ?= $(PROJECT_PATH)/jolt_wallet/elf2jelf/elf2jelf.py
PYTHONBIN    ?= python3
DEVICE_PORT  ?= /dev/ttyUSB0

# Add all the Jolt Wallet paths
export JOLT_WALLET_PATH := $(PWD)/jolt_wallet

EXTRA_COMPONENT_DIRS = \
		$(IDF_PATH)/tools/unit-test-app/components/ \
        $(PROJECT_PATH) \
        $(PROJECT_PATH)/src \
        $(JOLT_WALLET_PATH)/jolt_os/lvgl \
        $(JOLT_WALLET_PATH)/jolt_os \
        $(JOLT_WALLET_PATH)/components

CFLAGS += \
        -Werror \
		-DCONFIG_APP_COIN_PATH="\"$(COIN_PATH)\"" \
		-DCONFIG_APP_BIP32_KEY="\"$(BIP32_KEY)\"" \
		-DCONFIG_APP_VERSION_MAJOR=$(APP_VERSION_MAJOR) \
		-DCONFIG_APP_VERSION_MINOR=$(APP_VERSION_MINOR) \
		-DCONFIG_APP_VERSION_PATCH=$(APP_VERSION_PATCH) \
        -DJOLT_APP


include $(IDF_PATH)/make/project.mk
include $(JOLT_WALLET_PATH)/make/jolt_lib.mk

# Internally link all components directly included in the Jolt App's Project.
APP_COMPONENTS := $(dir $(wildcard $(PROJECT_PATH)/components/*/component.mk))
APP_COMPONENTS := $(sort $(foreach comp,$(APP_COMPONENTS),$(lastword $(subst /, ,$(comp)))))
APP_COMPONENTS := main $(APP_COMPONENTS)

# Path to archive files to link
APP_LIBRARIES := $(foreach n, $(APP_COMPONENTS), build/$(n)/lib$(n).a)

# Component build dependencies
APP_COMPONENTS_TARGETS := $(foreach n, $(APP_COMPONENTS), component-$(n)-build)

.PHONY: sdkconfig_defaults tests lint clean-jolt jflash japp merge-menuconfig print-%

# Make sure jolt_lib.h is created before building components
$(APP_COMPONENTS_TARGETS): $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h

# Build ELF file
$(ELF_BIN_NAME): sdkconfig.defaults $(APP_COMPONENTS_TARGETS)
	$(CC) -Wl,-static -nostartfiles -nodefaultlibs -nostdlib -Os \
    		-ffunction-sections -fdata-sections -Wl,--gc-sections \
			-Wl,--start-group \
    		-s -o $(ELF_BIN_NAME) \
    		-Wl,-r \
    		-Wl,-ejapp_main \
    		-Wl,--warn-unresolved-symbols \
			$(APP_LIBRARIES)
	# Trim unnecessary sections
	$(OBJCOPY) --remove-section=.comment \
	         --remove-section=.xtensa.info \
	         --remove-section=.xt.prop \
	         --remove-section=.rela.xt.prop \
	         --remove-section=.xt.lit \
	         --remove-section=.rela.xt.lit \
	         --remove-section=.text \
	         $(ELF_BIN_NAME)

# Convert ELF file to JELF file
$(JELF_BIN_NAME): $(ELF_BIN_NAME)
ifndef JAPP_SIGNING_KEY
	$(eval JAPP_SIGNING_KEY = 000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F)
	$(warning Jolt App Signing Key not set, using default debugging signing key $(JAPP_SIGNING_KEY).)
endif

	$(PYTHONBIN) $(ELF2JELF)  $(ELF_BIN_NAME) \
		    --app_major $(APP_VERSION_MAJOR) \
		    --app_minor $(APP_VERSION_MINOR) \
		    --app_patch $(APP_VERSION_PATCH) \
    		--output $(JELF_BIN_NAME) \
    		--coin "$(COIN_PATH)" \
    		--bip32key "$(BIP32_KEY)" \
			--signing_key "$(JAPP_SIGNING_KEY)"

# Build Jolt App
japp: $(JELF_BIN_NAME) ;


# Upload the application's jelf file to device over USB.
jflash: $(JELF_BIN_NAME)
	$(PYTHONBIN) jolt_wallet/pyutils/usb_upload.py --port $(DEVICE_PORT) $(JELF_BIN_NAME)

lint:
	find main/ \
		\( \
			-iname '*.h' \
			-o -iname '*.c' \
			-o -iname '*.cpp' \
			-o -iname '*.hpp' \
		\) \
		| xargs clang-format -style=file -i -fallback-style=google

tests: sdkconfig_defaults $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h
	CFLAGS='-DUNIT_TESTING=1' \
		$(MAKE) \
		TEST_COMPONENTS='main' \
		flash monitor;

decode:
	# Only works on the tests target
	# usage: make decode 0x40...:\0x3ff 0x40...
	echo "\n"
	xtensa-esp32-elf-addr2line -pfiaC -e build/$(PROJECT_NAME).elf $(filter-out $@,$(MAKECMDGOALS))

%:
	@:

clean-jolt:
	rm -rf build/jolt_os
	rm -rf build/jolt_wallet

# Combine the Jolt sdkconfig.defaults and the app's sdkconfig.japp
sdkconfig_defaults: 
	# Create a new sdkconfig.default file that combines the japp-specific defaults and jolt-os defaults
	$(PYTHONBIN) jolt_wallet/pyutils/merge-menuconfig.py sdkconfig.japp jolt_wallet/sdkconfig.defaults

# Make the generation of sdkconfig.defaults a pre-req to menuconfig
prepare_kconfig_files: sdkconfig_defaults

# Alias merge-menuconfig
merge-menuconfig: sdkconfig_defaults ;

clean-japp: component-main-clean clean-jolt ;
	
# Prints a Makefile variable; for debugging purposes
print-%  : ; @echo $* = $($*)
