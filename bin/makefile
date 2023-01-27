ARCH = i386
CXX = $(ARCH)-elf-g++
ASM = nasm
NAME = DarkCore

UTILSDIR=../utils
FSGEN=$(UTILSDIR)/lite-fs/bin/lite-fs.elf

CXXFLAGS = -Wall -Wextra -Wshadow \
	-Wpointer-arith -Wcast-align -Wwrite-strings \
	-Wmissing-declarations -Wredundant-decls \
	-Winline -Wno-long-long -Wconversion \
	-std=c++17 -ffreestanding -fno-exceptions \
	-fno-rtti -O0 -I ../library/
LDFLAGS = -ffreestanding -O0 -nostdlib -nodefaultlibs -T link.ld
FSGENPARTFLAGS = -gp -a rs base ../storage lite-fs-image.pbd

CXXSOURCES = $(shell find ../library/ -name "*.cxx") $(shell find ../kernel/ -name "*.cxx")
ASMSOURCES = $(shell find ../library/ -name "*.asm") $(shell find ../kernel/ -name "*.asm")

CXXOBJECTS = $(addsuffix o, $(CXXSOURCES))
ASMOBJECTS = $(addsuffix o, $(ASMSOURCES))

.PHONY:		clean

all:
	$(MAKE) build
	$(MAKE) run-qemu
	$(MAKE) clean

build:
	$(MAKE) $(CXXOBJECTS)
	$(MAKE) $(ASMOBJECTS)
	$(MAKE) kernel
	$(FSGEN) $(FSGENPARTFLAGS)
	$(MAKE) $(NAME)

%.cxxo:		%.cxx
	$(CXX) -c $< $(CXXFLAGS) -o $@

%.asmo:		%.asm
	nasm -felf $< -o $@

kernel:		$(CXXOBJECTS) $(ASMOBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@.bin
	kernelSize=$$(stat -c %s $@.bin);\
	alignedSize=$$(./align.sh $$kernelSize 512);\
	truncate -s $$alignedSize $@.bin

$(NAME):
	$(ASM) -fbin ../boot/main.asm -I../boot/ -o main.bin
	$(ASM) -fbin ../boot/boot.asm -o $(NAME).bin

run-qemu:
	qemu-system-$(ARCH) -monitor stdio -m 2G -cpu max -drive format=raw,file=$(NAME).bin -D dbg.txt -d cpu_reset

run-bochs:
	bochs -q --enable-debugging

clean:
	rm -rf *.bin *.txt $(CXXOBJECTS) $(ASMOBJECTS) *.pbd
	find ../ -type f -name '*.cxx.o' -delete