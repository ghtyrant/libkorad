CC := gcc
LD := $(CC)

CFLAGS  := -fvisibility=hidden -fPIC -std=c11 -Wall -g -DKORAD_DLL -DKORAD_DLL_EXPORTS
LDFLAGS := -lserialport

# Stolen from git's Makefile
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ifneq (,$(findstring MINGW,$(uname_S)))
	CFLAGS += -I/usr/include/
	LDFLAGS += -L/usr/lib/
	LIB_EXT = dll
endif
ifneq (,$(findstring MSYS_NT,$(uname_S)))
	LIB_EXT = dll
endif
ifeq ($(uname_S),Linux)
	LIB_EXT = so
endif
ifeq ($(uname_S),Darwin)
	LIB_EXT = dylib
endif

TARGET  := libkorad.$(LIB_EXT)
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
	make -C example/ clean

install:
	install -p $(TARGET) /usr/local/lib/
	install -p src/korad.h /usr/local/include/

.PHONY: example
