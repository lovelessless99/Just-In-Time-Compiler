CC = clang
CFLAGS = -g -Wall

SRC_DIR = ../../Part2-BF_compiler

BF_SED = sed_BF
BF_SED_O1 = sed_BF_O1
BF_SED_O2 = sed_BF_O2
BF_SED_O3 = sed_BF_O3

BF_INTERPRETER = BF_interpreter
BF_COMPILER_X64 = BF_compiler_x64

TEST_PROGRAM = ../mandelbrot.bf

.PHONY: clean 

all: test 

test: $(BF_SED)  $(BF_COMPILER_X64) run_compiler
	sh test.sh

run_compiler: $(BF_COMPILER_X64)
	./$< $(TEST_PROGRAM) > mandelbrot-compiler.s
	$(CC) -o mandelbrot-compiler mandelbrot-compiler.s

$(BF_COMPILER_X64): $(SRC_DIR)/$(BF_COMPILER_X64).c
	$(CC) $(CFLAGS) -o $@ $^


$(BF_SED): $(BF_SED).c
	$(CC) -S -o $(BF_SED).s $^
	$(CC) $(CFLAGS) -o $@ $^

	$(CC) -S -O1 -o $(BF_SED_O1).s $^
	$(CC) $(CFLAGS) -O1 -o $(BF_SED_O1) $^

	$(CC) -S -O2 -o $(BF_SED_O2).s $^
	$(CC) $(CFLAGS) -O2 -o $(BF_SED_O2) $^

	$(CC) -S -O3 -o $(BF_SED_O3).s $^
	$(CC) $(CFLAGS) -O3 -o $(BF_SED_O3) $^

$(BF_SED).c: $(SRC_DIR)/BF_2_C.sed $(TEST_PROGRAM)
	sed -f $< $(TEST_PROGRAM) > $(BF_SED).c 

clean:
	rm -f $(BF_SED).c $(BF_SED) $(BF_SED).s
	rm -f $(BF_SED_O1) $(BF_SED_O1).s
	rm -f $(BF_SED_O2) $(BF_SED_O2).s
	rm -f $(BF_SED_O3) $(BF_SED_O3).s
	
	rm -f $(BF_COMPILER_X64) mandelbrot-compiler.s mandelbrot-compiler summarize_time comparison.png 