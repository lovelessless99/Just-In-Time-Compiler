CC = clang
CFLAGS = -g -Wall

SRC_DIR = ../Part2-BF_compiler

BF_SED = sed_BF
BF_INTERPRETER = BF_interpreter
BF_COMPILER_X64 = BF_compiler_x64
BF_SED_O1 = sed_BF_O1
BF_SED_O2 = sed_BF_O2
BF_SED_O3 = sed_BF_O3

BF_INTERPRETER_opt = BF_interpreter_opt_1 BF_interpreter_opt_2 BF_interpreter_statistic BF_interpreter_opt_3
JIT_DYNASM_FILE = jit_dynasm jit_dynasm_opt1 jit_dynasm_opt2

TEST_PROGRAM = mandelbrot.bf


.PHONY: clean 

all: test 

test: $(BF_INTERPRETER) $(BF_SED)  $(BF_COMPILER_X64) run_compiler $(BF_INTERPRETER_opt) $(JIT_DYNASM_FILE) jit_opcode.o jit_opcode 
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

	$(CC) -S -O1 -o $(BF_SED_O1).s $^
	$(CC) $(CFLAGS) -O1 -o $(BF_SED_O1) $^

	$(CC) -S -O2 -o $(BF_SED_O2).s $^
	$(CC) $(CFLAGS) -O2 -o $(BF_SED_O2) $^

	$(CC) -S -O3 -o $(BF_SED_O3).s $^
	$(CC) $(CFLAGS) -O3 -o $(BF_SED_O3) $^

$(BF_SED).c: $(SRC_DIR)/BF_2_C.sed $(TEST_PROGRAM)
	sed -f $< $(TEST_PROGRAM) > $(BF_SED).c 


BF_interpreter_opt_1: ../Part4-BF_optimization/BF_interpreter_opt/BF_interpreter_opt1.c
	$(CC) $(CFLAGS) $^ -o $@

BF_interpreter_opt_2: ../Part4-BF_optimization/BF_interpreter_opt/BF_interpreter_opt2.c
	$(CC) $(CFLAGS) $^ -o $@

BF_interpreter_opt_3: ../Part4-BF_optimization/BF_interpreter_opt/BF_interpreter_opt3.c
	$(CC) $(CFLAGS) $^ -o $@

BF_interpreter_statistic: ../Part4-BF_optimization/BF_interpreter_opt/BF_interpreter_statistic.cpp
	$(CC)++ -std=c++14 $(CFLAGS) $^ -o $@ -DBFTRACE



jit_dynasm: ../Part4-BF_optimization/JIT_Dynasm/dynasm-driver.c ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm.h
	$(CC) $(CFLAGS) -I../Part4-BF_optimization/JIT_Dynasm/third_party -o $@ ../Part4-BF_optimization/JIT_Dynasm/dynasm-driver.c -DJIT=\"jit_dynasm.h\"

../Part4-BF_optimization/JIT_Dynasm/jit_dynasm.h: ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm.dasc
	lua ../Part4-BF_optimization/JIT_Dynasm/third_party/dynasm/dynasm.lua $^ > $@


jit_dynasm_opt1: ../Part4-BF_optimization/JIT_Dynasm/dynasm-driver.c ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt1.h
	$(CC) $(CFLAGS) -I../Part4-BF_optimization/JIT_Dynasm/third_party -o $@ ../Part4-BF_optimization/JIT_Dynasm/dynasm-driver.c -DJIT=\"jit_dynasm_opt1.h\"

../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt1.h: ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt1.dasc
	lua ../Part4-BF_optimization/JIT_Dynasm/third_party/dynasm/dynasm.lua $^ > $@

jit_dynasm_opt2: ../Part4-BF_optimization/JIT_Dynasm/dynasm-driver.c ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt2.h
	$(CC) $(CFLAGS) -I../Part4-BF_optimization/JIT_Dynasm/third_party -o $@ ../Part4-BF_optimization/JIT_Dynasm/dynasm-driver.c -DJIT=\"jit_dynasm_opt2.h\"

../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt2.h: ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt2.dasc
	lua ../Part4-BF_optimization/JIT_Dynasm/third_party/dynasm/dynasm.lua $^ > $@


jit_opcode: jit_opcode.o
	$(CC) -o $@ $^

jit_opcode.o: ../Part4-BF_optimization/JIT_Opcode/jit_opcode.c
	$(CC) -c $(CFLAGS) $^ -o $@



clean:
	rm -f $(BF_SED).c $(BF_SED) $(BF_SED).s
	rm -f $(BF_SED_O1) $(BF_SED_O1).s
	rm -f $(BF_SED_O2) $(BF_SED_O2).s
	rm -f $(BF_SED_O3) $(BF_SED_O3).s	
	rm -f $(BF_INTERPRETER)
	
	rm -f BF_interpreter_opt_1 BF_interpreter_opt_2 BF_interpreter_statistic BF_interpreter_opt_3
	rm -f jit_dynasm ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm.h jit_dynasm_opt1 ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt1.h jit_dynasm_opt2 ../Part4-BF_optimization/JIT_Dynasm/jit_dynasm_opt2.h
	rm -f jit_opcode.o  jit_opcode

	rm -f $(BF_COMPILER_X64) mandelbrot-compiler.s mandelbrot-compiler summarize_time comparison.png  interpreter.png opt_int_simple_jit.png  faster.png
