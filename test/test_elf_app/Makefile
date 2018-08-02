#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

all: main.o main.elf

%.o: %.c
	xtensa-esp32-elf-gcc -fno-common -mlongcalls -Wall -Os -o $@ -c $<

%.elf: %.o
	xtensa-esp32-elf-gcc -Wl,-r -nostartfiles -nodefaultlibs -nostdlib -Os -Wl,-Tesp32.ld -o $@ $< 

clean:
	rm -f *.o *.elf

flash:
	./flash_user_app.sh
