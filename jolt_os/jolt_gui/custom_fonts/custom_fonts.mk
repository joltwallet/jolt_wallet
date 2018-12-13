CSRCS += f_6x10.c
CSRCS += synchronizer7.c
CSRCS += jolt_gui_symbols.c
CSRCS += crox3hb.c
CSRCS += unifont.c
DEPPATH += --dep-path jolt_gui/custom_fonts
VPATH += :jolt_gui/custom_fonts
CFLAGS += "-I$(LVGL_DIR)/jolt_gui/custom_fonts"
