# Add coin directories and subdirectoreis below
COMPONENT_SRCDIRS := . \
	gui \
	gui/entry \
	gui/menus \
	hal \
	hal/aes132_library \
	jolt_gui \
	jolt_gui/custom_fonts \
	radio \
	syscore \
	hal/lv_drivers \
	hal/lv_drivers/display \
	hal/storage
#	hal/storage/aes132_library \

COMPONENT_ADD_INCLUDEDIRS := .\
	hal/storage/aes132_library
