AS = nasm
CC = gcc
LD = ld

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -Iinclude
LDFLAGS = -m elf_i386 -T src/linker.ld

OBJS = build/boot.o build/kernel.o build/idt.o build/idt_asm.o \
       build/isr.o build/pic.o build/keyboard.o build/timer.o \
       build/string.o build/memory.o build/command.o build/commands.o \
       build/lineedit.o

all: build/kernel.bin argon.iso

build/boot.o: src/boot.asm
	@mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/idt_asm.o: src/idt_asm.asm
	@mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/kernel.o: src/kernel.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: src/idt.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/isr.o: src/isr.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/pic.o: src/pic.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: src/keyboard.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/timer.o: src/timer.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/string.o: src/string.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/memory.o: src/memory.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/command.o: src/command.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/commands.o: src/commands.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/lineedit.o: src/lineedit.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

argon.iso: build/kernel.bin
	@echo "Creating ISO..."
	@rm -rf isodir
	@mkdir -p isodir/boot/grub
	@cp build/kernel.bin isodir/boot/kernel.bin
	@cp boot/grub/grub.cfg isodir/boot/grub/grub.cfg
	@echo "Files copied, running grub-mkrescue..."
	grub-mkrescue -o argon.iso isodir 2>&1 | grep -v "xorriso"
	@echo "ISO created: argon.iso"

run: build/kernel.bin
	qemu-system-i386 -kernel build/kernel.bin

run-iso: argon.iso
	qemu-system-i386 -cdrom argon.iso

clean:
	rm -rf build isodir argon.iso

.PHONY: all run run-iso clean