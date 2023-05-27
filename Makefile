CC      = gcc
TARGET  = editor
C_FILES = $(filter-out $(TARGET).c, $(wildcard *.c))
OBJS    = $(patsubst %.c,%.o,$(C_FILES))
CFLAGS  = -g -Wall -Werror -pedantic-errors
LDFLAGS =

.PHONY: all clean
	all: $(TARGET)
$(TARGET): $(OBJS) $(TARGET).c
	    $(CC) $(CFLAGS) $(OBJS) $(TARGET).c -o $(TARGET) $(LDFLAGS)
%.o: %.c %.h
	    $(CC) $(CFLAGS) -c -o $@ $<
clean:
	    rm -f $(OBJS) $(TARGET) $(TARGET).exe
