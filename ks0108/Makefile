# Name: Makefile
# Project: GLCD2USB
# Author: Christian Starkjohann, modified for GLCD2USB by Till Harbaum
# Creation Date: 2005-03-20
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile.avrusb,v 1.1 2007/01/14 12:12:27 harbaum Exp $

# DEFINES += -DBWCT_COMPAT 
# DEFINES += -DDEBUG_LEVEL=1 -DDEBUG
# change the following line to atmega32 to use that cpu
MCU ?= atmega8
#F_CPU ?= 12800000
F_CPU ?= 12000000

CFLAGS = -fno-strict-aliasing -ffunction-sections -fdata-sections \
	 -Iusbdrv -I. -DDEBUG_LEVEL=0 -DTUNE_OSCCAL=0 -DCALIBRATE_OSCCAL=0
LDFLAGS = -Wl,--relax -Wl,--gc-sections

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o ks0108.o glcd.o rprintf.o osccal.o
COMPILE = avr-gcc -Wall -O2 $(CFLAGS) -mmcu=$(MCU) -DF_CPU=$(F_CPU)

# symbolic targets:
all:	firmware.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

# ATmega8 fuses: (HFUSE = 0x81 if no bootloader)
HFUSE = 0x81
# Fuse high byte:
# 1 0 0 0   0 0 0 0 ←─ BOOTRST	(Boot reset vector at byte address 0x1800)
# ↑ ↑ ↑ ↑   ↑ └─┴───── BOOTSZ	(Boot-area size: 2 KByte (0x800), maximum)
# │ │ │ │   └───────── EESAVE	(Preserve EEPROM over chip erase: YES)
# │ │ │ └───────────── CKOPT	(Clock option: full output swing)
# │ │ └─────────────── SPIEN	(Serial Programming enable: YES)
# │ └───────────────── WDTON	(Watchdog Timer always on: YES)
# └─────────────────── RSTDISBL	(External Reset disable: NO)

# Fuse low byte:
LFUSE = 0x9f
# 1 0 0 1   1 1 1 1
# ↑ ↑ └┬┘   └──┬──┘
# │ │  │       └────── CKSEL	(Oscillator selection: crystal > 8 MHz)
# │ │  └────────────── SUT	(Start-up time: )
# │ └───────────────── BODEN	(Brown-out detector enable: YES)
# └─────────────────── BODLEVEL (Brown-out detection level: 2.7V)

flash:	all
	avrdude -c avrisp -P /dev/ttyUSB0 -b 19200 -p $(MCU) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U flash:w:firmware.hex

fuse:
	$(UISP) --wr_fuse_h=$(HFUSE) --wr_fuse_l=$(LFUSE)


clean:
	rm -f *~ firmware.lst firmware.obj firmware.cof firmware.list firmware.map firmware.bin *.o usbdrv/*.o firmware.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
firmware.elf:	$(OBJECTS)
	$(COMPILE) $(LDFLAGS) -o firmware.elf $(OBJECTS)

firmware.hex:	firmware.elf
	rm -f firmware.hex
	avr-objcopy -j .text -j .data -O ihex firmware.elf firmware.hex
	./checksize firmware.elf 8096 960
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

disasm:	firmware.elf
	avr-objdump -d firmware.elf

cpp:
	$(COMPILE) -E main.c
