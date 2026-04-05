# ------------------------------------------------------------
# Project settings
# ------------------------------------------------------------
PROJECT      = renderer
TARGET       = $(PROJECT).elf
TEST_TARGET  = tests.elf

SRCDIR       = src
INCDIR       = include
TESTDIR      = tests
UNITYDIR     = third_party/unity

FLYCAST ?= /c/flycast/build/flycast.exe

# ------------------------------------------------------------
# Source discovery
# ------------------------------------------------------------
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:.c=.o) romdisk.o

# Unity test sources
TEST_SRCS := \
	$(TESTDIR)/test_main.c \
	$(TESTDIR)/test_vec3.c \
	$(UNITYDIR)/unity.c

# ------------------------------------------------------------
# IMPORTANT:
# Add ONLY the application source files you want to test here.
# Do NOT include the file that contains your normal main().
#
# Example:
# TEST_APP_SRCS := src/vec3.c src/mat4.c src/clipping.c
# ------------------------------------------------------------
TEST_APP_SRCS := 

TEST_OBJS := $(TEST_SRCS:.c=.o) $(TEST_APP_SRCS:.c=.o)

# ------------------------------------------------------------
# ROM disk setup
# ------------------------------------------------------------
KOS_ROMDISK_DIR = romdisk

# ------------------------------------------------------------
# SH4ZAM paths
# ------------------------------------------------------------
SH4ZAM_INC = /opt/toolchains/dc/kos-ports/sh4zam/build/sh4zam-1.0.0/include/sh4zam
SH4ZAM_LIB = /opt/toolchains/dc/kos-ports/sh4zam/build/sh4zam-1.0.0/build

# ------------------------------------------------------------
# Compiler / linker flags
# ------------------------------------------------------------
CFLAGS += -std=gnu2x -I$(INCDIR) -I$(SH4ZAM_INC) -DNDEBUG

# These extra include paths are harmless for the normal build and
# make the test objects compile cleanly using the default KOS rules.
CFLAGS += -I$(TESTDIR) -I$(UNITYDIR)

LIBS = -lpng -lkosutils -lz -lstb_image -lsh4zam -fbuiltin -ffast-math -ffp-contract=fast

# ------------------------------------------------------------
# Default target
# ------------------------------------------------------------
all: $(TARGET)

# ------------------------------------------------------------
# Include KOS build rules
# ------------------------------------------------------------
include /opt/toolchains/dc/kos/Makefile.rules

# ------------------------------------------------------------
# Main build
# ------------------------------------------------------------
$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) $(LIBS) -I$(SH4ZAM_INC) -L$(SH4ZAM_LIB) -L$(KOS_BASE)/lib/dreamcast

# ------------------------------------------------------------
# Unity test build
# ------------------------------------------------------------
$(TEST_TARGET): $(TEST_OBJS)
	kos-cc -o $(TEST_TARGET) $(TEST_OBJS) $(LIBS) -I$(SH4ZAM_INC) -L$(SH4ZAM_LIB) -L$(KOS_BASE)/lib/dreamcast

# ------------------------------------------------------------
# Run targets
# ------------------------------------------------------------
run: $(TARGET)
	"$(FLYCAST)" "$(TARGET)"

test: $(TEST_TARGET)
	"$(FLYCAST)" "$(TEST_TARGET)"

# Optional: build tests, then run tests, then build app
check: $(TEST_TARGET)
	"$(FLYCAST)" "$(TEST_TARGET)"
	$(MAKE) $(TARGET)

# ------------------------------------------------------------
# Dist / deployment helpers
# ------------------------------------------------------------
dist: $(TARGET)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)

dreamcast:
	/opt/dreamsdk/tools/dcload-serial/host-src/dc-tool-serial -t COM3 -x $(TARGET)

# ------------------------------------------------------------
# Assembly output helpers
# ------------------------------------------------------------
asm:
	$(foreach src, $(SRCS), \
		kos-cc -S -o $(basename $(src)).s $(src) &&) true

vasm:
	@mkdir -p asm
	$(foreach src, $(SRCS), \
		kos-cc -S -fverbose-asm -O2 -I$(INCDIR) $(src) -o asm/$(notdir $(basename $(src))).s &&) true

# ------------------------------------------------------------
# Debug build
# ------------------------------------------------------------
debug: CFLAGS += -DDEBUG_ENABLED
debug: clean all

# ------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------
rm-elf:
	-rm -f $(TARGET) $(TEST_TARGET)

clean: rm-elf
	-rm -f $(OBJS) $(TEST_OBJS) *.img *.elf src/*.s asm/*.s tests/*.o third_party/unity/src/*.o

# ------------------------------------------------------------
# Phony targets
# ------------------------------------------------------------
.PHONY: all run test check dist dreamcast asm vasm debug clean rm-elf