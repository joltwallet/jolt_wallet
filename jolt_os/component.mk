COMPONENT_NAME := jolt_os

COMPONENT_SRCDIRS := . \
	hal \
	hal/lv_drivers \
	hal/lv_drivers/display \
	hal/radio \
	hal/storage \
	hal/storage/aes132_library \
	jelf_loader/src \
	jolt_gui \
	jolt_gui/custom_fonts \
	jolt_gui/menus \
	syscore

COMPONENT_ADD_INCLUDEDIRS := . \
	hal/storage/aes132_library \
	jelf_loader/include
