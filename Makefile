# Project settings
PROJECT = renderer
TARGET = $(PROJECT).elf
OBJS = renderer.o vector.o display.o mesh.o triangle.o romdisk.o

# ROM Disk setup (if used)
KOS_ROMDISK_DIR = romdisk

# Add any libraries you use here. Include `-lkosimg` if you're using kos/img.h
LIBS = -lpng -lkosutils -lz

# Build target
all: rm-elf $(TARGET)

# Include KOS Makefile helpers
include /opt/toolchains/dc/kos/Makefile.rules

# Clean up object files and output binaries
clean: rm-elf
	-rm -f $(OBJS)

rm-elf:
	-rm -f $(TARGET)

# Final build link command
$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) $(LIBS) -L$(KOS_BASE)/lib/dreamcast

# Run via Flycast emulator (optional)
run:
	flycast $(TARGET)

# Create a stripped binary (optional for CD builds)
dist: $(TARGET)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)
