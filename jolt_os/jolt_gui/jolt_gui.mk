
include ./jolt_gui/custom_fonts/custom_fonts.mk

CSRCS += jolt_gui_entry.c
CSRCS += jolt_gui_statusbar.c
CSRCS += jolt_gui_loading.c
CSRCS += jolt_gui.c
CSRCS += lv_theme_jolt.c
CSRCS += test_stubs.c
CSRCS += test_screens.c
DEPPATH += --dep-path jolt_gui
VPATH += :jolt_gui
CFLAGS += "-I$(LVGL_DIR)/jolt_gui"


# Custom Fonts
#CSRCS += custom_fonts/unscii.c
#DEPPATH += --dep-path jolt_gui/custom_fonts
#VPATH += :jolt_gui/custom_fonts
#CFLAGS += "-I$(LVGL_DIR)/jolt_gui/custom_fonts"

# Graphics
#CSRCS += graphics/battery_3.c
#DEPPATH += --dep-path jolt_gui/graphics
#VPATH += :jolt_gui/graphics
#CFLAGS += "-I$(LVGL_DIR)/jolt_gui/graphics"
