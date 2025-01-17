CURRENTDIR	= .
SRCDIR		= $(CURRENTDIR)/src
OUTPUTDIR	= $(CURRENTDIR)/output

INCLUDEDIR	= $(CURRENTDIR)/include
COMPILER_DIR	= /usr

# Linker script 
BASE_ADDR	?= 0x00000000
BOOT_LAYOUT_IN	= $(SRCDIR)/magiboot.ld.in
BOOT_LAYOUT_OUT	= $(OUTPUTDIR)/magiboot.ld


# Output ELF image
MAGIBOOT_ELF	= $(OUTPUTDIR)/magiboot

# Output binary image
MAGIBOOT_BIN	= $(OUTPUTDIR)/magiboot.bin

CROSS_COMPILE ?= arm-none-eabi-

AS	= as
CC	= gcc
LD	= ld
CPP	= g++
STRIP	= strip
OBJCOPY	= objcopy
OBJDUMP	= objdump

LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
CFLAGS 	= -Wall -I$(INCLUDEDIR) -I$(COMPILER_DIR)/include -nostdinc -fno-builtin -O -g
LDFLAGS = -static -nostdlib -T $(BOOT_LAYOUT_OUT) -L$(LIBGCCDIR)  -lgcc

CFLAGS += -DSWORD


# Generic code
SRC_OBJS = entry.o serial.o main.o utils.o init.o gpmi.o dm9000x.o net.o


MAGIBOOT_OBJS = $(addprefix $(SRCDIR)/, $(SRC_OBJS))
#		  $(addprefix $(BOARDDIR)/, $(BOARD_OBJS)) \
#		  $(addprefix $(HWDIR)/, $(HW_OBJS))

# Default goal
.PHONY: all
all: build



#
# Define an implicit rule for assembler files
# to run them through C preprocessor
#
%.o: %.S
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Make targets
#
.PHONY: build build_prep clean

build: build_prep $(MAGIBOOT_BIN)

build_prep:
	mkdir -p $(OUTPUTDIR)

clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(MAGIBOOT_OBJS) $(BOOT_LAYOUT_OUT)
	@echo Build output:
	rm -rf $(OUTPUTDIR)

##
## Rules to link and convert magiboot image
## 

$(MAGIBOOT_BIN): $(MAGIBOOT_ELF)
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $< $@

$(MAGIBOOT_ELF): $(MAGIBOOT_OBJS) $(BOOT_LAYOUT_OUT)
	$(LD) -o $@ $(MAGIBOOT_OBJS) $(LDFLAGS)
	@nm -n $@ > $@.map

$(BOOT_LAYOUT_OUT): $(BOOT_LAYOUT_IN)
	$(CPP) -P -DBASE_ADDR=$(BASE_ADDR) -o $@ $<

