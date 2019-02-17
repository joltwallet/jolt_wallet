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
    syscore/cmd

COMPONENT_ADD_INCLUDEDIRS := . \
    hal/storage/aes132_library \
    jelf_loader/include

*.o: jolt_lib.h

jolt_lib.h: $(COMPONENT_PATH)/../elf2jelf/export_list.txt $(COMPONENT_PATH)/../elf2jelf/jolt_lib_template.txt $(COMPONENT_PATH)/../elf2jelf/elf2jelf.py $(COMPONENT_PATH)/../elf2jelf/setup.py
	cd $(COMPONENT_PATH)/../elf2jelf; python3 elf2jelf.py --export_only; python3 setup.py build_ext --inplace;


$(COMPONENT_PATH)/../elf2jelf/export_list.txt: ;

$(COMPONENT_PATH)/../elf2jelf/jolt_lib_template.txt: ;

$(COMPONENT_PATH)/../elf2jelf/elf2jelf.py: ;

$(COMPONENT_PATH)/../elf2jelf/setup.py: ;

