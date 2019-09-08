PROJECT_NAME := jolt

EXTRA_COMPONENT_DIRS := \
	$(abspath .) \
	$(abspath jolt_os) \
	$(IDF_PATH)/tools/unit-test-app/components/

GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)

CFLAGS += \
		-Werror \
		-DJOLT_OS \
		-DJOLT_OS_COMMIT_STR=\"$(GIT_VERSION)\"

include $(IDF_PATH)/make/project.mk

print-%  : ; @echo $* = $($*)

.PHONY: tests test-menu

tests:
	CFLAGS='-DUNIT_TESTING=1' \
		$(MAKE) \
		TEST_COMPONENTS='jolt_os' \
		flash monitor;

test-menu:
	CFLAGS='-DJOLT_GUI_TEST_MENU=1' \
		$(MAKE) \
		flash monitor;
