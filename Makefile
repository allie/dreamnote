CC=clang
CFLAGS=-std=c99 -g -c -Wall -Werror -D_POSIX_C_SOURCE=200112L -DUNOFFICIAL_MODE -Iinclude -I/usr/local/include -Ilib
LDFLAGS=-lSDL2 -lSDL2_image -lsoundio -lsndfile -lsamplerate -lm
SOURCES=$(shell find src -name "*.c" -not -name "*.partial.c")
SOURCES+= lib/tictoc/src/tictoc.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dreamnote
OS=$(shell gcc -dumpmachine)

ifneq (, $(findstring mingw, $(OS)))
	LDFLAGS := -lmingw32 -lSDL2main $(LDFLAGS)
	CC=gcc
endif

all: $(SOURCES) $(EXECUTABLE)

debug: CFLAGS += -DDEBUG_MODE
debug: all

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE)
	find . -name "*.o" -print0 | xargs -0 rm -rf
