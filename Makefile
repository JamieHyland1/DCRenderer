
TARGET = renderer.elf
OBJS = renderer.o vector.o display.o mesh.o triangle.o

all: rm-elf $(TARGET)

include /opt/toolchains/dc/kos/Makefile.rules

clean: rm-elf
	-rm -f $(OBJS)

rm-elf:
	-rm -f $(TARGET)

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS)

run:
	flycast.exe $(TARGET)

dist: $(TARGET)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)
