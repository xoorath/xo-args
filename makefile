# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c99
CPPFLAGS = -Wall -Wextra -pedantic -std=c++98

# Targets
all: example1 example2 example3 xo-args-tests

xo-args.o:
	mkdir -p obj/common/
	$(CC) $(CFLAGS) -c examples/common/xo-args.c -o obj/common/xo-args.o


example1.o:
	mkdir -p obj/example1/
	$(CC) $(CFLAGS) -c examples/example1/example1.c -o obj/example1/example1.o

example2.o:
	mkdir -p obj/example2/
	$(CC) $(CFLAGS) -c examples/example2/example2.c -o obj/example2/example2.o

example3.o:
	mkdir -p obj/example3/
	$(CC) $(CPPFLAGS) -c examples/example3/example3.cpp -o obj/example3/example3.o

example3-xo-args.o:
	mkdir -p obj/example3/
	$(CC) $(CPPFLAGS) -c examples/example3/xo-args.cpp -o obj/example3/xo-args.o

xo-args-tests.o:
	mkdir -p obj/tests/
	$(CC) $(CFLAGS) -c tests/xo-args-tests.c -o obj/tests/xo-args-tests.o

example1: example1.o xo-args.o
	mkdir -p bin/
	$(CC) obj/example1/example1.o obj/common/xo-args.o -o bin/example1

example2: example2.o xo-args.o
	mkdir -p bin/
	$(CC) obj/example2/example2.o obj/common/xo-args.o -o bin/example2

example3: example3.o example3-xo-args.o
	mkdir -p bin/
	$(CC) obj/example3/example3.o obj/example3/xo-args.o -o bin/example3

xo-args-tests: xo-args-tests.o
	mkdir -p bin/
	$(CC) obj/tests/xo-args-tests.o -o bin/xo-args-tests

tests: xo-args-tests
	clear
	bin/xo-args-tests

# Clean target
clean:
	rm -rf obj/
	rm -rf bin/
