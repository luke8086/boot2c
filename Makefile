AS = as
CC = gcc
LD = ld
OBJCOPY = objcopy

# On Mac, install i386-elf-gcc and i386-elf-binutils packages from MacPorts
#
# AS = i386-elf-as
# CC = i386-elf-gcc
# LD = i386-elf-ld
# OBJCOPY = i386-elf-objcopy

ASFLAGS = -march=i386 --32
CFLAGS = -ffreestanding -m16 -march=i386 -Wall -Wextra -pedantic
LDFLAGS = -Tlinker.ld -nostdlib -m elf_i386

OBJDIR = build

COMMON_OBJS = $(addprefix $(OBJDIR)/, intr.o util.o)
BINS = $(addprefix $(OBJDIR)/,boot.bin snake.bin)
DISK = $(OBJDIR)/disk.img

.PHONY: clean disk ensure-app

# don't remove intermediate files
.SECONDARY:

all: $(OBJDIR) $(BINS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.bin: $(OBJDIR)/%.elf
	$(OBJCOPY) -O binary $< $@

$(OBJDIR)/boot.elf: boot.s
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/%.elf: $(OBJDIR)/%.o $(COMMON_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

disk: all ensure-app
	dd if=/dev/zero of=$(DISK) bs=512 count=2880
	dd if=$(OBJDIR)/boot.bin of=$(DISK) bs=512 conv=notrunc
	dd if=$(OBJDIR)/boot.bin of=$(DISK) bs=512 conv=notrunc seek=1
	dd if=$(OBJDIR)/$(APP).bin of=$(DISK) bs=512 conv=notrunc seek=2

qemu: all ensure-app disk
	qemu-system-i386 -drive format=raw,file=$(DISK)

bochs: all ensure-app disk
	bochs -f bochsrc.txt

clean:
	rm -rf build
