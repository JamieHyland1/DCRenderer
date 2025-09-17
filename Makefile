# Project settings
PROJECT = renderer
TARGET = $(PROJECT).elf
SRCDIR = src
INCDIR = include
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:.c=.o) romdisk.o

# ROM Disk setup (if used)
KOS_ROMDISK_DIR = romdisk

# SH4ZAM paths
SH4ZAM_INC = /opt/toolchains/dc/kos/sh4zam/include/sh4zam
SH4ZAM_LIB = /opt/toolchains/dc/kos/sh4zam/build

# Compiler include flags
CFLAGS += -std=c2x -std=gnu2x -I$(INCDIR) -I$(SH4ZAM_INC)

# Add any libraries you use here
LIBS = -lpng -lkosutils -lz -lstb_image -lsh4zam -fbuiltin -ffast-math -ffp-contract=fast

# Build target
all: rm-elf $(TARGET)

# Include KOS Makefile helpers
include /opt/toolchains/dc/kos/Makefile.rules

# Clean up object files and output binaries
clean: rm-elf
	-rm -f $(OBJS) *.img src/*.s asm/*.s

rm-elf:
	-rm -f $(TARGET)

# Final build link command
$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) $(LIBS) -I$(SH4ZAM_INC) -L$(SH4ZAM_LIB) -L$(KOS_BASE)/lib/dreamcast
 

# Run via Flycast emulator (optional)
run:
	flycast $(TARGET)

# Create a stripped binary (optional for CD builds)
dist: $(TARGET)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)

dreamcast:
	/opt/dreamsdk/tools/dcload-serial/host-src/dc-tool-serial -t COM3 -x renderer.elf

asm:
	$(foreach src, $(SRCS), \
		kos-cc -S -o $(basename $(src)).s $(src) &&) true

vasm:
	@mkdir -p asm
	$(foreach src, $(SRCS), \
		kos-cc -S -fverbose-asm -O2 -I$(INCDIR) $(src) -o asm/$(notdir $(basename $(src))).s &&) true


# Fix debug target to define DEBUG_ENABLED
debug: CFLAGS += -DEBUG_ENABLED
debug: LDFLAGS += 
debug: clean all
