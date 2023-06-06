CC      = gcc
TARGET  = text_editor
C_FILES = $(filter-out $(TARGET).c, $(wildcard *.c))
OBJS    = $(patsubst %.c,%.o,$(C_FILES))
CFLAGS  = `pkg-config --cflags gtk4`
LDFLAGS = `pkg-config --libs gtk4`

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJS) $(TARGET).c
	    $(CC) $(CFLAGS) $(OBJS) $(TARGET).c -o $(TARGET) $(LDFLAGS)
%.o: %.c %.h
	    $(CC) $(CFLAGS) -c -o $@ $<
clean:
	    rm -f $(OBJS) $(TARGET) $(TARGET).exe
