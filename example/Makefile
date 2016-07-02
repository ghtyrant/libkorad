CC := gcc
LD := $(CC)

CFLAGS  := -Wall -std=c11 -g $(GLIB_CFLAGS) -I../src/
LDFLAGS := -L../ -lkorad

TARGET  := example
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

run: all
	LD_LIBRARY_PATH=../ ./$(TARGET)

gdb: all
	LD_LIBRARY_PATH=../ gdb $(TARGET)

valgrind: all
	LD_LIBRARY_PATH=../ valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,indirect,possible --show-reachable=no --leak-resolution=high --num-callers=20 --trace-children=no --child-silent-after-fork=yes --track-fds=yes --track-origins=yes ./$(TARGET) 2>&1 | tee valgrind.log

callgrind: all
	LD_LIBRARY_PATH=../ valgrind --tool=callgrind ./$(TARGET)

clean:
	rm $(TARGET) $(OBJECTS)