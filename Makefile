PROJECT_NAME := jolt

EXTRA_COMPONENT_DIRS := \
	$(abspath .) \
	$(abspath jolt_os) \
	$(IDF_PATH)/tools/unit-test-app/components/

CFLAGS += \
		-Werror \
		-DJOLT_OS

include $(IDF_PATH)/make/project.mk

print-%  : ; @echo $* = $($*)

.PHONY: tests test-menu lint compilation_db cppcheck

tests:
	CFLAGS='-DUNIT_TESTING=1' \
		$(MAKE) \
		TEST_COMPONENTS='jolt_os' \
		flash monitor;

test-menu:
	CFLAGS='-DJOLT_GUI_TEST_MENU=1' \
		$(MAKE) \
		flash monitor;

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

