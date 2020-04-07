PROJECT_NAME := jolt

export IDF_TOOLS_PATH := $(PWD)/xtensa-esp32-elf
export IDF_PATH := $(PWD)/esp-idf
export JOLT_WALLET_PATH := $(PWD)

# Have to add current path to EXTRA_COMPONENT_DIRS so unit tester will work.
# Unfortunately, JoltOS gets built twice.
EXTRA_COMPONENT_DIRS := \
	$(abspath .) \
	$(abspath jolt_os) \
	$(IDF_PATH)/tools/unit-test-app/components/

CFLAGS += \
		-Werror \
		-DJOLT_OS

include $(IDF_PATH)/make/project.mk
include $(JOLT_WALLET_PATH)/make/common.mk

.PHONY: install tests test-menu lint compilation_db cppcheck decode

all: $(PB_GENS)

install:
	mkdir -p $(IDF_TOOLS_PATH)
	pip3 install -r requirements.txt
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

decode:
	# usage: make decode 0x40...:\0x3ff 0x40...
	echo "\n"
	xtensa-esp32-elf-addr2line -pfiaC -e build/$(PROJECT_NAME).elf $(filter-out $@,$(MAKECMDGOALS))

%:
	@:

print-%  : ; @echo $* = $($*)
