#
# Component Makefile
#

COMPONENT_SRCDIRS := . \
    lvgl/src/lv_core \
    lvgl/src/lv_draw \
    lvgl/src/lv_font \
    lvgl/src/lv_hal \
    lvgl/src/lv_misc \
    lvgl/src/lv_objx \
    lvgl/src/lv_themes
    
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_SRCDIRS) .
