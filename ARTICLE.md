## Introduction

I was recently wondering what would it take to run a simple C
[program](https://github.com/luke8086/nf) directly on top of the PC BIOS,
without any operating system code. Since BIOS provides an out-of-the-box
[API](http://stanislavs.org/helppc/idx_interrupt.html) for accessing disks,
keyboard, video (both text and graphics), system timer, and more, it should
be easy enough, right?

Well, there are tons of tutorials for writing boot-loaders and simple
operating systems, but usually they're either extremely limited
in functionality, or require a substantial time commitment. Meanwhile,
I wanted to find out how straightforward it can possibly be to write
usable applications with minimal boilerplate.

As a byproduct, after a few days of tinkering, I created
[this repository](https://github.com/luke8086/boot2c) to share my approach.
It shows how to create C programs, which:

- boot directly from a USB drive / SD card
- don't require any operating system code
- don't require writing any custom drivers
- only require small and fixed amount of assembly code
- can access the BIOS API directly from C

Of course they also have some limitations:

- they don't have access to the standard C library
- they only work in the [real-address mode](http://www.logix.cz/michal/doc/i386/chp14-00.htm)
- the final binary is limited to ~64KB
- available RAM is limited to ~640KB
- the boot-loader is not guaranteed to work on every PC

So why bother doing such a thing in 2019? Although bare BIOS is too limited for any
real-world applications, thanks to the backwards compatibility, it's still widely
available. It's also well-documented and well-understood. In the end, I think it
still makes for a fun and approachable way to tinker with your PC over a few
lazy afternoons.

## Code overview

#### Overall structure

The final app consists of two binaries: a boot loader and the actual
program. Both are created by compiling the source code with clang/gcc
and GNU assembler, linking the program with GNU ld, and converting
the resulting ELF files to flat binaries using objcopy.

To ensure that our `main()` is always the first code in the program
binary, we move it to a separate `.start` section using the
[`ENTRY_POINT`](https://github.com/luke8086/boot2c/blob/master/util.h)
macro, and emit it at the top of the file
using a custom
[linker script](https://github.com/luke8086/boot2c/blob/master/linker.ld)

#### 16 vs 32 bit

It is [possible](http://www.logix.cz/michal/doc/i386/chp16-00.htm)
to use 32-bit instructions in the real-address mode, they just need
to be marked with address-size and operand-size prefixes. The
`-m16` option for clang/gcc, and `.code16` directive in GNU assembler, do exactly
that. The resulting code is 32-bit, only marked everywhere with those
prefixes. It's not compatible with actual 16-bit CPUs.

Unfortunately the 32-bit addresses still cannot exceed the boundary
of the [segment](http://www.logix.cz/michal/doc/i386/chp14-01.htm#14-01)
(65535), otherwise they'll trigger an exception. QEMU
doesn't emulate this behaviour, so it's useful to occasionally test
with Bochs.

#### Bootloader

The boot loader
([boot.s](https://github.com/luke8086/boot2c/blob/master/boot.s))
loads the main program from the
startup disk to the memory segment at 0x10000, and jumps to the
starting point at offset 0.
It assumes that BIOS will emulate the USB disk either as
a HDD or a floppy. To make it more likely, it includes a basic MBR
partition table. Just in case, we install it both to the main boot
sector, and the boot sector of the first active partition.

Since USB booting
[is not](https://wiki.osdev.org/Problems_Booting_From_USB_Flash)
a standardized process, it may not work on every PC. I only really tested
on mine, and it still behaved in two different ways for a USB stick
and an SD card.
In case it doesn't work for you, you can try an
[alternative](#installing-the-syslinux-boot-loader) loader.

#### Calling BIOS services

The services provided by BIOS are primarily accessed by saving their
method number and arguments to CPU registers, and triggering a software
interrupt. Some of them store return values back to the registers.

To avoid writing separate assembly code for every service, we define a generic
function
([intr.h](https://github.com/luke8086/boot2c/blob/master/intr.h),
[intr.s](https://github.com/luke8086/boot2c/blob/master/intr.s)),
taking the interrupt
number and a pointer to a struct holding register values.

#### Memory segmentation

Modern compilers don't have a concept of far pointers, so we can't
seamlessly access memory outside of the current code / data segments.
This is the main factor limiting our binary to 64K.

Fortunately, both
[gcc](https://gcc.gnu.org/onlinedocs/gcc-6.1.0/gcc/Named-Address-Spaces.html#index-x86-named-address-spaces-3132)
and [clang](https://clang.llvm.org/docs/LanguageExtensions.html#memory-references-to-specified-segments)
have support for "address spaces" relative to FS and GS.
We include functions
([util.h](https://github.com/luke8086/boot2c/blob/master/util.h),
[util.c](https://github.com/luke8086/boot2c/blob/master/util.c))
to set values
of these registers, for example to access the text-mode video memory
at `b800:0000`
(see [bios.h](https://github.com/luke8086/boot2c/blob/master/bios.h))

#### Standard library

The standard C library depends on the operating system, so we can't
use it in standalone programs (hence the `-ffreestanding` and `-nostdlib`
flags). However, for certain operations, like initialising a struct on
the stack, the compiler may still generate implicit calls to standard
functions, like `memcpy`. In such cases, we just need to provide
our own versions (see
[util.h](https://github.com/luke8086/boot2c/blob/master/util.h) and
[util.c](https://github.com/luke8086/boot2c/blob/master/util.c)).

## Troubleshooting

More likely than not, working on standalone programs will require some
tinkering. Below are some hints:

#### Installing the syslinux boot loader

In case the provided boot loader doesn't work, you may experiment with the
one of syslinux:

```
$ wget https://mirrors.edge.kernel.org/pub/linux/utils/boot/syslinux/6.xx/syslinux-6.03.tar.gz
$ tar zxf syslinux-6.03.tar.gz
$ make disk APP=snake
$ dd if=syslinux-6.03/bios/mbr/mbr.bin of=build/disk.img conv=notrunc
```

#### Disassembling files

By default, objdump will disassemble our binaries with no complaints,
showing a completely incorrect output. Since the code is compiled to run
in 16-bit mode, we need to add `-m i8086`:

```bash
$ objdump -D -m i8086 build/hello.o
$ objdump -D -m i8086 build/hello.elf
$ objdump -D -b binary -m i8086 build/hello.bin
```

#### Debugging with Bochs

Bochs is one of the slowest emulators, but often more accurate than others.
Its debugger seems to handle 16-bit code slightly better than GDB with QEMU.
The `xchg %bx, bx` instruction can be used to set a breakpoint, in C it's
available using `BOCHS_BREAKPOINT` macro. The system clock is completely
inaccurate, so it's not that useful for testing games / animations.

#### Running in VirtualBox

The easiest way to test in VirtualBox is by attaching `disk.img` as a raw
image of a floppy. However, it imposes a limit on the amount of sectors
that can be read (with a single BIOS call) to 0x48. So you'll need to replace
`mov $0x027f, %ax` with `mov $0x0248, %ax` in `boot.s`

#### Writing assembly functions

In case you want to write any custom assembly function, be sure to use
32-bit `ret` (i.e. `retl` in GNU as), otherwise it'll leave the stack
shifted by 2 bytes.

## References

Lists of available BIOS services:

- [HelpPC Reference Library](http://stanislavs.org/helppc/) - A well-organized
  quick reference of interrupts and other relevant topics
- [Ralf Brown's Interrupt List](http://www.ctyme.com/rbrown.htm) - The ultimate list

If you'd like to learn more:

- [OSDev Wiki](https://wiki.osdev.org) - Tons of knowledge about low-level development, primarily focused on writing operating systems
- [Intel 80386 Programmer's Reference Manual](http://www.logix.cz/michal/doc/i386/)
- [80286 and 80287 Programmer's Reference Manual](https://duckduckgo.com/?q="80286+and+80287+Programmers+Reference+Manual")
- [System BIOS for IBM PC/XT/AT computers and compatibles](https://duckduckgo.com/?q="System+BIOS+for+IBM+PC%2FXT%2FAT+computers+and+compatibles") - Another comprehensive reference of BIOS
- [IBM 5150 Technical Reference](https://duckduckgo.com/?q=ibm+5150+technical+reference)
  (includes the source code of the original BIOS)
- [IBM PC BIOS source code reconstruction](https://sites.google.com/site/pcdosretro/ibmpcbios)
