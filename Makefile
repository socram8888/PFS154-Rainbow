
ARCH = pdk14
MCU = pfs154
CFLAGS = -Iinclude -m$(ARCH) -p$(MCU) -DPFS154

all: beacon.ihx

beacon.ihx: beacon.c
	sdcc $(CFLAGS) beacon.c

clean:
	$(RM) *.o *.asm *.cdb *.ihx *.lk *.lst *.map *.rel *.rst *.sym
