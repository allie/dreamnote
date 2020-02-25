CC=clang
CFLAGS=-std=c99 -g -Wno-nullability-completeness -Wall -Werror -D_POSIX_C_SOURCE=200112L -DUNOFFICIAL_MODE -O2
INC=-Iinclude -I/usr/local/include
LDFLAGS=-lSDL2 -lSDL2_image -lportaudio -lsndfile -lsamplerate -lm
SOURCES=$(shell find src -name "*.c" -not -name "*.partial.c")
OBJDIR=build
OBJECTS=$(SOURCES:%.c=$(OBJDIR)/%.o)
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

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $(INC) $< -o $@

clean:
	rm -f $(EXECUTABLE)
	rm -rf $(OBJDIR)
