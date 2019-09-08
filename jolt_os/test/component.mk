COMPONENT_ADD_LDFLAGS = -Wl,--whole-archive -l$(COMPONENT_NAME) -Wl,--no-whole-archive
#include $(PROJECT_PATH)/jolt_wallet/component.mk
