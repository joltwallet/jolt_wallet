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

ifndef JAPP_SIGNING_KEY
JAPP_SIGNING_KEY = 000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F
$(warning Jolt App Signing Key not set, using default debugging signing key $(JAPP_SIGNING_KEY).)
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
EXTRA_COMPONENT_DIRS = \
		$(IDF_PATH)/tools/unit-test-app/components/ \
        $(PROJECT_PATH)/src \
        $(PROJECT_PATH)/jolt_wallet/jolt_os/lvgl \
        $(PROJECT_PATH)/jolt_wallet/jolt_os \
        $(PROJECT_PATH)/jolt_wallet/components

CFLAGS += \
        -Werror \
        -DJOLT_APP

include $(IDF_PATH)/make/project.mk

# Internally link all components directly included in the Jolt App's Project.
APP_COMPONENTS := $(dir $(wildcard $(PROJECT_PATH)/components/*/component.mk))
APP_COMPONENTS := $(sort $(foreach comp,$(APP_COMPONENTS),$(lastword $(subst /, ,$(comp)))))
APP_COMPONENTS := main $(APP_COMPONENTS)

# Path to archive files to link
APP_LIBRARIES := $(foreach n, $(APP_COMPONENTS), build/$(n)/lib$(n).a)

# Component build dependencies
APP_COMPONENTS_TARGETS := $(foreach n, $(APP_COMPONENTS), component-$(n)-build)

.PHONY: jflash japp print-%

# Build ELF file
$(ELF_BIN_NAME): $(APP_COMPONENTS_TARGETS)
	$(CC) -Wl,-static -nostartfiles -nodefaultlibs -nostdlib -Os \
    		-ffunction-sections -fdata-sections -Wl,--gc-sections \
			-Wl,--start-group \
    		-s -o $(ELF_BIN_NAME) \
    		-Wl,-r \
    		-Wl,-eapp_main \
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
	$(PYTHONBIN) $(ELF2JELF)  "$(ELF_BIN_NAME)" \
    		--output "$(JELF_BIN_NAME)" \
    		--coin "$(COIN_PATH)" \
    		--bip32key "$(BIP32_KEY)" \
			--signing_key "$(JAPP_SIGNING_KEY)"

# Build Jolt App
japp: $(JELF_BIN_NAME) ;


# Upload the application's jelf file to device over USB.
jflash: $(JELF_BIN_NAME)
	$(PYTHONBIN) jolt_wallet/pyutils/usb_upload.py --port $(DEVICE_PORT) $(JELF_BIN_NAME)

# Prints a Makefile variable; for debugging purposes
print-%  : ; @echo $* = $($*)
