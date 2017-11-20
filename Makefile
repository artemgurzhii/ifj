SRC=$(wildcard src/*.c)
OBJ=${SRC:.c=.o}

CC=clang
PREFIX=/usr/local
CFLAGS=-std=c99 -g -O0 -Wno-parentheses -Wno-switch-enum -Wno-unused-value
CFLAGS+=-Wno-switch
CFLAGS+=-I deps
LDFLAGS+=-lm

# MinGW gcc support
# TODO: improve

clang=$(shell which clang 2> /dev/null)
ifeq (, $(clang))
	CC = gcc
endif

# deps

CFLAGS+=-I deps/linenoise
OBJ+=deps/linenoise/linenoise.o
OBJ+=deps/linenoise/utf8.o

# test
TEST_SRC=$(shell find src/*.c test/*.c | sed '/ifj17/d')
TEST_OBJ=${TEST_SRC:.c=.o}

CFLAGS+=-I src

# output

OUT=ifj17
ifdef SystemRoot
	OUT = ifj17.exe
endif

$(OUT): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@
	@printf "\e[36mCC\e[90m %s\e[0m\n" $@

test: test_runner
	@./$<

test_runner: $(TEST_OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

install: ifj17
	install ifj17 $(PREFIX)/bin

uninstall:
	rm $(PREFIX)/bin/ifj17

clean:
	rm -f ifj17 test_runner $(OBJ) $(TEST_OBJ)

.PHONY: clean test install uninstall
