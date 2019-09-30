###########
# Primary #
###########
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
    jolt_gui/menus/settings \
	lang \
    syscore \
    syscore/cmd \
	syscore/encoding \
	syscore/ymodem

COMPONENT_ADD_INCLUDEDIRS := . \
    hal/storage/aes132_library \
    jelf_loader/include

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/jolt_lib.c \
	$(COMPONENT_PATH)/jolt_commit.c


##############################
# Additional Build Artifacts #
##############################

# Generate the C source files before building JoltOS
$(COMPONENT_LIBRARY): $(COMPONENT_PATH)/jolt_lib.c $(COMPONENT_PATH)/jolt_commit.c

# Generate C source files via python script
$(COMPONENT_PATH)/jolt_lib.c $(COMPONENT_PATH)/jolt_commit.c: $(COMPONENT_PATH)/../elf2jelf/export_list.txt $(COMPONENT_PATH)/../elf2jelf/jolt_lib_template.c $(COMPONENT_PATH)/../elf2jelf/elf2jelf.py $(COMPONENT_PATH)/../elf2jelf/common_structs.py
	cd $(COMPONENT_PATH)/../elf2jelf; python3 elf2jelf.py --export_only;
