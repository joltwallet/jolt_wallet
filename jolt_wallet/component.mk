# Add coin directories and subdirectoreis below
COMPONENT_SRCDIRS := . \
	hal \
	hal/lv_drivers \
	hal/lv_drivers/display \
	hal/storage \
	hal/storage/aes132_library \
	jolt_gui \
	jolt_gui/custom_fonts \
	jolt_gui/menus \
	radio \
	syscore

COMPONENT_ADD_INCLUDEDIRS := . \
	hal/storage/aes132_library
