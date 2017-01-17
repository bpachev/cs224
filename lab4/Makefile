#
# Author: Rick Kimball
# edited by Benjamin Pachev
# email: rick@kimballsoftware.com
# Version: 1.03 Initial version 10/21/2011

APP= lab4
MCU=msp430g2553

CC=msp430-gcc
AS=msp430-gcc
CXX=msp430-g++
COMMON=-Wall -Os -I. 
CFLAGS   += -mmcu=$(MCU) $(COMMON)
CXXFLAGS += -mmcu=$(MCU) $(COMMON)
ASFLAGS  += -mmcu=$(MCU) $(COMMON)
LDFLAGS   = -Wl,-Map,$(APP).map -nostdlib -nostartfiles

all: $(APP).elf

%.elf: %.o
	$(CC) $(LDFLAGS)  $< -o $@
	msp430-objdump -z -EL -D -W $(APP).elf >$(APP).lss
	msp430-size $(APP).elf
	msp430-objcopy -O ihex $(APP).elf $(APP).hex

run: all
	sudo mspdebug --force-reset rf2500 "prog $(APP).elf" "mw 0x21 1" "mw 0x22 1" "set pc 0xfc00" "run"

cycle_count: all
	naken_util -disasm $(APP).hex > $(APP)_cc.txt


clean:
	rm -f *.o *.elf
