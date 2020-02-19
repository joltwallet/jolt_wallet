###########
# Primary #
###########

COMPONENT_NAME := jolt_os

COMPONENT_SRCDIRS := . \
	app_cmd/ \
	crypto/ \
	crypto/algorithms \
    hal \
    hal/lv_drivers \
    hal/lv_drivers/display \
    hal/radio \
    hal/storage \
    hal/storage/aes132_library \
    hal/storage/atca \
	hash/ \
	hash/algorithms \
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
	crypto \
    hal/storage/aes132_library \
	hal/storage/atca \
	hash \
    jelf_loader/include

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/jolt_lib.h


##############################
# Additional Build Artifacts #
##############################

jelf_loader/src/loader.o: $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h
main.o: $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h
syscore/cmd/jolt_cmd_about.o: $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h
syscore/cmd/jolt_cmd_lv_obj.o: $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h
syscore/launcher.o: $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h
test/app_cmd/test_contacts.o: $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h
test/syscore/test_launcher.o: $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h

include $(COMPONENT_PATH)/../make/jolt_lib.mk
