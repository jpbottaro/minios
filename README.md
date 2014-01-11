# Minios 0.01

Simple and minimal x86 OS.

## Requirements

### Build

* nasm
* binutils
* mtools

### Test

* bochs-sdl

## Compilation - Use

Everything is done in the bin folder:

* make - create kernel.bin y and diskette image to boot bochs
* make clean - clean all
* make test - clean, build everything and start bochs with the new image

The 'fsimage' file in a MinisFs v2 image, and it is used by the kernel to boot.
To edit its contents in linux, issue:

    mount -o loop -t minix fsimage FOLDER

To create a new one:

    dd if=/dev/zero of=NAME bs=1k count=BLOCK_NR
    mkfs.minix -v NAME BLOCK_NR

Currently only MinixFS v2 is supported (that is what the -v flags creates)

## Folders

* apps: some applications to use with the system
* bin: build system (makefile et al)
* doc: docs (mainly in spanish)
* fs: file system source
* include: system-wide include files (eg. the list interface)
* kernel: basic kernel, including mm, interrupt handles, scheduler...
* lib: various generic functions (eg mystrncpy, mystrncmp)

## Applications

Every app is in the apps/ folder. These are already compiled and placed in a
bin/ folder inside the 'fsimage'. In case you want to change or add any:

* Modify apps/ and call make.
* Add the .pso (these are the excecutables of minios) to the bin/ folder in 'fsimage'
