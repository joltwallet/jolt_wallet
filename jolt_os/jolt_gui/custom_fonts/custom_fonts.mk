CSRCS += jolt_gui_symbols.c
CSRCS += dejavu_40_numeric.c
CSRCS += crox3hb_numeric.c
DEPPATH += --dep-path jolt_gui/custom_fonts
VPATH += :jolt_gui/custom_fonts
CFLAGS += "-I$(LVGL_DIR)/jolt_gui/custom_fonts"
