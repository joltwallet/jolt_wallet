# USAGE:
# board selection:
#     env var TARGET_BOARD
#     

PROJECT_NAME := jolt_os

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))

export JOLT_WALLET_PATH := $(dir $(MKFILE_PATH))
export IDF_TOOLS_PATH := $(JOLT_WALLET_PATH)/xtensa-esp32-elf
export IDF_PATH := $(JOLT_WALLET_PATH)/esp-idf

# Have to add current path to EXTRA_COMPONENT_DIRS so unit tester will work.
# Unfortunately, JoltOS gets built twice.
EXTRA_COMPONENT_DIRS := \
	$(abspath .) \
	$(abspath jolt_os) \
	$(IDF_PATH)/tools/unit-test-app/components/

CFLAGS += \
		-Werror \
		-DJOLT_OS

# cfg.mk first to override sdkconfig.defaults
# set targetname via `TARGET_BOARD=target`
include $(JOLT_WALLET_PATH)/make/cfg.mk
include $(IDF_PATH)/make/project.mk
include $(JOLT_WALLET_PATH)/make/common.mk

# Executables that generate/apply binary patches
HDIFFZ=components/esp_hdiffz/HDiffPatch/hdiffz
HPATCHZ=components/esp_hdiffz/HDiffPatch/hpatchz

ifeq ("$(shell uname)", "Darwin")
    # Mac OS X platform        
	INSTALL_SYSTEM_DEPENDENCIES = \
		printf "No system-dependencies rules for MacOS\n" \
		&& exit 1
else ifeq ("$(shell expr substr $$(uname -s) 1 5)", "Linux")
    # GNU/Linux platform
	OS=$(shell lsb_release -si)
	ifeq ($(OS), Ubuntu)
		INSTALL_SYSTEM_DEPENDENCIES = \
			sudo apt-get update && sudo apt-get install -y \
				bison \
				ccache \
				cmake \
				flex \
				git \
				gperf \
				libffi-dev \
				libssl-dev \
				libusb-1.0 \
				libz-dev \
				ninja-build \
				protobuf-compiler \
				python3 \
				python3-pip \
				python3-setuptools \
				wget
	else
		INSTALL_SYSTEM_DEPENDENCIES = \
			printf "No rules for your linux distribution: \"$(OS)\"\n" \
			&& exit 1
	endif
else 
	INSTALL_SYSTEM_DEPENDENCIES = \
		printf "No system-dependencies rules for your operating system\n" \
		&& exit 1
endif



.PHONY: install tests test-menu lint compilation_db cppcheck decode \
	system-dependencies

all: $(PB_GENS)

system-dependencies:
	$(INSTALL_SYSTEM_DEPENDENCIES)

$(HDIFFZ): system-dependencies
	# Build the hdiffpatch generator binary
	cd components/esp_hdiffz/HDiffPatch \
		&& make

hdiffz: $(HDIFFZ) ;

install: system-dependencies
	mkdir -p $(IDF_TOOLS_PATH)
	pip3 install -r requirements.txt
	pip3 install -r $(IDF_PATH)/requirements.txt
	$(IDF_PATH)/tools/idf_tools.py install

tests: $(PB_GENS)
	CFLAGS='-DUNIT_TESTING=1' \
		$(MAKE) \
		TEST_COMPONENTS='jolt_os' \
		flash monitor;

test-menu: $(PB_GENS)
	CFLAGS='-DJOLT_GUI_TEST_MENU=1' $(MAKE)

lint:
	find jolt_os/ \
		\( \
			-iname '*.h' \
			-o -iname '*.c' \
			-o -iname '*.cpp' \
			-o -iname '*.hpp' \
		\) \
		! -iname 'jolt_lib.c' \
		! -path 'jolt_os/hal/lv_drivers/*' \
		! -path 'jolt_os/hal/storage/cryptoauthlib/*' \
		! -path 'jolt_os/jolt_gui/custom_fonts/*' \
		| xargs clang-format -style=file -i -fallback-style=google

compile_commands.json:
	rm -rf build
	bear $(MAKE) -j

compilation_db: compile_commands.json

cppcheck: compile_commands.json
	cppcheck \
		--project=compile_commands.json \
		--enable=all \
		--suppressions-list=.cfg/cppcheck-suppressions.txt \
		--inline-suppr \
		–template=gcc \
		--force \
		-ijolt_os/hal/lv_drivers/ \
		jolt_os/ 2>cppcheck.log
	echo ""
	echo "--------------------------"
	echo "---- cppcheck results ----"
	echo "--------------------------"
	cat cppcheck.log
	echo ""

clean-jolt:
	rm -rf build/jolt_os
	rm -rf build/jolt_wallet
	rm -f $(PB_GENS)

clean: clean-jolt

compress: all
	# Compress firmware build/jolt.bin -> build/jolt.bin.gz
	python3 pyutils/ota_compress.py
	
decode:
	# usage: make decode 0x40...:\0x3ff 0x40...
	echo "\n"
	xtensa-esp32-elf-addr2line -pfiaC -e build/$(PROJECT_NAME).elf $(filter-out $@,$(MAKECMDGOALS))

%:
	@:

print-%  : ; @echo $* = $($*)
