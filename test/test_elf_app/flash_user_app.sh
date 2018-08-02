USER_PARTITION_OFFSET=0x1f4000
python2 /home/bpugh/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB ${USER_PARTITION_OFFSET} main.elf
