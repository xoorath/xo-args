# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Targets
all: example1 example2

xo-args.o:
	mkdir -p obj/common/
	$(CC) $(CFLAGS) -c examples/common/xo-args.c -o obj/common/xo-args.o

example1.o:
	mkdir -p obj/example1/
	$(CC) $(CFLAGS) -c examples/example1/example1.c -o obj/example1/example1.o

example2.o:
	mkdir -p obj/example2/
	$(CC) $(CFLAGS) -c examples/example2/example2.c -o obj/example2/example2.o

example1: example1.o xo-args.o
	mkdir -p bin/
	$(CC) obj/example1/example1.o obj/common/xo-args.o -o bin/example1

example2: example2.o xo-args.o
	mkdir -p bin/
	$(CC) obj/example2/example2.o obj/common/xo-args.o -o bin/example2

# Clean target
clean:
	rm -rf obj/
	rm -rf bin/
