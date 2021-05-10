CC = clang
CFLAGS = -g -Wall

SRC_DIR = ../../Part2-BF_compiler

BF_SED = sed_BF
BF_INTERPRETER = BF_interpreter
BF_COMPILER_X64 = BF_compiler_x64

TEST_PROGRAM = ../mandelbrot.bf


.PHONY: clean 

all: test 

test: $(BF_INTERPRETER) $(BF_SED)  $(BF_COMPILER_X64) run_compiler
	sh test.sh


$(BF_INTERPRETER): $(SRC_DIR)/$(BF_INTERPRETER).c
	$(CC) $(CFLAGS) -o $@ $^

run_compiler: $(BF_COMPILER_X64)
	./$< $(TEST_PROGRAM) > mandelbrot-compiler.s
	$(CC) -o mandelbrot-compiler mandelbrot-compiler.s

$(BF_COMPILER_X64): $(SRC_DIR)/$(BF_COMPILER_X64).c
	$(CC) $(CFLAGS) -o $@ $^

$(BF_SED): $(BF_SED).c
	$(CC) -S -o $(BF_SED).s $^
	$(CC) $(CFLAGS) -o $@ $^

$(BF_SED).c: $(SRC_DIR)/BF_2_C.sed $(TEST_PROGRAM)
	sed -f $< $(TEST_PROGRAM) > $(BF_SED).c 

clean:
	rm -f $(BF_INTERPRETER) $(BF_SED).c $(BF_SED) $(BF_SED).s $(BF_COMPILER_X64) mandelbrot-compiler.s mandelbrot-compiler summarize_time comparison.png 