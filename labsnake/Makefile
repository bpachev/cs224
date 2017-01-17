#
# Author: Rick Kimball
# edited by Benjamin Pachev
# email: rick@kimballsoftware.com
# Version: 1.03 Initial version 10/21/2011

APP= lab10
OBJS= snake.o snake_events.o snake_interrupts.o snake_images.o  RBX430-1.o RBX430_lcd.o  
MCU=msp430f2274

CC=msp430-gcc
AS=msp430-gcc
CXX=msp430-g++
COMMON=-Wall -Wno-comment -Os -I. 
CFLAGS   += -mmcu=$(MCU) $(COMMON)
CXXFLAGS += -mmcu=$(MCU) $(COMMON)
ASFLAGS  += -mmcu=$(MCU) $(COMMON)
LDFLAGS   = -Wl,-Map,$(APP).map -mmcu=$(MCU)

all: $(APP).elf

$(APP).elf: $(OBJS)
	$(CC) $(LDFLAGS)  $(OBJS) -o $(APP).elf
	msp430-objdump -z -EL -D -W $(APP).elf >$(APP).lss
	msp430-size $(APP).elf
	msp430-objcopy -O ihex $(APP).elf $(APP).hex

run: all
	sudo mspdebug --force-reset rf2500 "prog $(APP).elf"  "run"

cycle_count: all
	naken_util -disasm $(APP).hex > $(APP)_cc.txt


clean:
	rm -f *.o *.elf
