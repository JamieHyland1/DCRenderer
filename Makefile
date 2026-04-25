.DEFAULT_GOAL := all

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
DC_TOOL ?= dc-tool-ser
DC_PORT ?= COM3
DC_BAUD ?=

# ------------------------------------------------------------
# Source discovery
# ------------------------------------------------------------
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:.c=.o) romdisk.o

# Unity test sources
TEST_SRCS := \
    $(SRCDIR)/display.c \
    $(SRCDIR)/triangle.c \
    $(SRCDIR)/texture.c \
    $(SRCDIR)/vector.c \
    $(SRCDIR)/utils.c \
    $(SRCDIR)/render_target.c \
    $(SRCDIR)/skybox.c \
    $(SRCDIR)/pipeline.c \
    $(SRCDIR)/object.c \
    $(SRCDIR)/mesh.c \
    $(SRCDIR)/matrix.c \
    $(SRCDIR)/camera.c \
    $(SRCDIR)/clipping.c \
    $(SRCDIR)/light.c \
    $(SRCDIR)/array.c \
    $(SRCDIR)/debug.c \
    $(SRCDIR)/vmu.c \
    $(TESTDIR)/test_main.c \
    $(TESTDIR)/test_vec3.c \
    $(TESTDIR)/test_utils.c \
    $(TESTDIR)/test_fixtures.c \
    $(TESTDIR)/test_memory.c \
    $(TESTDIR)/test_drawing.c \
    $(TESTDIR)/test_pipeline.c \
    $(UNITYDIR)/unity.c

# ------------------------------------------------------------
# IMPORTANT:
# Add ONLY the application source files you want to test here.
# Do NOT include the file that contains your normal main().
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
CFLAGS += -std=gnu2x -I$(INCDIR) -I$(SH4ZAM_INC)
CFLAGS += -I$(TESTDIR) -I$(UNITYDIR)
CFLAGS += -fbuiltin -ffast-math -ffp-contract=fast
CFLAGS += -DNDEBUG

LIBS = -lpng -lkosutils -lz -lstb_image -lsh4zam

# ------------------------------------------------------------
# Per-file optimization overrides
# ------------------------------------------------------------
TRIANGLE_SRC    := $(SRCDIR)/triangle.c
TRIANGLE_OBJ    := $(SRCDIR)/triangle.o
TRIANGLE_CFLAGS := -O3

CLIPPING_SRC    := $(SRCDIR)/clipping.c
CLIPPING_OBJ    := $(SRCDIR)/clipping.o
CLIPPING_CFLAGS := -O3

$(TRIANGLE_OBJ): CFLAGS += $(TRIANGLE_CFLAGS)
$(CLIPPING_OBJ): CFLAGS += $(CLIPPING_CFLAGS)

# ------------------------------------------------------------
# Debug / release builds
# ------------------------------------------------------------
debug: CFLAGS := $(filter-out -DNDEBUG,$(CFLAGS))
debug: CFLAGS += -DDEBUG -DDEBUG_ENABLED
debug: clean all

release: clean all

# Optional baud flag for dc-tool-ser
DC_BAUD_FLAG := $(if $(strip $(DC_BAUD)),-b $(DC_BAUD),)

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
$(TEST_TARGET): $(TEST_OBJS) romdisk.o
	kos-cc -o $(TEST_TARGET) $(TEST_OBJS) romdisk.o $(LIBS) -I$(SH4ZAM_INC) -L$(SH4ZAM_LIB) -L$(KOS_BASE)/lib/dreamcast

# ------------------------------------------------------------
# Run targets
# ------------------------------------------------------------
run: $(TARGET)
	"$(FLYCAST)" "$(TARGET)"

tests: $(TEST_TARGET)

test-run: $(TEST_TARGET)
	"$(FLYCAST)" "$(TEST_TARGET)"

test: test-run

# Optional: build tests, then run tests, then build app
check: $(TEST_TARGET)
	"$(FLYCAST)" "$(TEST_TARGET)"
	$(MAKE) $(TARGET)

# ------------------------------------------------------------
# Dreamcast run targets
# ------------------------------------------------------------
dreamcast: $(TARGET)
	$(DC_TOOL) -t $(DC_PORT) $(DC_BAUD_FLAG) -x $(TARGET)

dreamcast-test: $(TEST_TARGET)
	$(DC_TOOL) -t $(DC_PORT) $(DC_BAUD_FLAG) -x $(TEST_TARGET)

# ------------------------------------------------------------
# Dist / deployment helpers
# ------------------------------------------------------------
dist: $(TARGET)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)

# ------------------------------------------------------------
# Assembly output helpers
# ------------------------------------------------------------
ASM_DIR := asm

asm:
	@mkdir -p $(ASM_DIR)
	@for src in $(SRCS); do \
		out="$(ASM_DIR)/$$(basename $${src%.c}).s"; \
		extra=""; \
		if [ "$$src" = "$(TRIANGLE_SRC)" ]; then extra="$(TRIANGLE_CFLAGS)"; fi; \
		if [ "$$src" = "$(CLIPPING_SRC)" ]; then extra="$$extra $(CLIPPING_CFLAGS)"; fi; \
		echo "  ASMSRC  $$src -> $$out"; \
		kos-cc $(CFLAGS) $$extra -S "$$src" -o "$$out" || exit $$?; \
	done

vasm:
	@mkdir -p $(ASM_DIR)
	@for src in $(SRCS); do \
		out="$(ASM_DIR)/$$(basename $${src%.c}).s"; \
		extra=""; \
		if [ "$$src" = "$(TRIANGLE_SRC)" ]; then extra="$(TRIANGLE_CFLAGS)"; fi; \
		if [ "$$src" = "$(CLIPPING_SRC)" ]; then extra="$$extra $(CLIPPING_CFLAGS)"; fi; \
		echo "  VASM    $$src -> $$out"; \
		kos-cc $(CFLAGS) $$extra -S -fverbose-asm "$$src" -o "$$out" || exit $$?; \
	done

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
.PHONY: all run tests test-run test check dreamcast dreamcast-test dist asm vasm debug release clean rm-elf