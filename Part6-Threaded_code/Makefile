CC = gcc
CFLAGS = -g
TEST_PROGRAM = mandelbrot.bf


.PHONY: clean 

all: test

test: BF_interpreter thread_code_interpreter
		sh test.sh

BF_interpreter: BF_interpreter.c
		$(CC) $(CFLAGS) $^ -o $@

thread_code_interpreter: thread_code_interpreter.c
		$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f BF_interpreter
	rm -f thread_code_interpreter