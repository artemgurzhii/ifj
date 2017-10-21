CC= gcc
CFLAGS= -std=c11 -Wall -Wextra -pedantic -O3
BIN= ifj2017
SRC= main.c
RM= rm -f
# list of modules will be completed during development
MODULES= main.o

all: $(MODULES)
	$(CC) $(CFLAGS) -o $(BIN) $(MODULES)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	$(RM) *.o $(BIN)
