# Generate jolt_lib header via python script
$(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h: $(JOLT_WALLET_PATH)/elf2jelf/export_list.txt $(JOLT_WALLET_PATH)/elf2jelf/jolt_lib_template.h $(JOLT_WALLET_PATH)/elf2jelf/elf2jelf.py $(JOLT_WALLET_PATH)/elf2jelf/common_structs.py
	echo "Generating jolt_lib.h"
	python3 $(JOLT_WALLET_PATH)/elf2jelf/elf2jelf.py --export_only --jolt_lib $(JOLT_WALLET_PATH)/jolt_os/jolt_lib.h;
