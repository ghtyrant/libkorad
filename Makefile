CC := gcc
LD := $(CC)

CFLAGS  := -fvisibility=hidden -fPIC -std=c11 -Wall -g -DKORAD_DLL -DKORAD_DLL_EXPORTS
LDFLAGS := -lserialport

TARGET  := libkorad.so
SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

example:
	make -C example/

clean:
	rm -rf $(TARGET) $(OBJECTS)

.PHONY: example