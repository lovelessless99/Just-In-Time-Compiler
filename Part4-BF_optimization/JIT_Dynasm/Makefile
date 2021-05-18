CC = clang
CFLAGS = -c -g -Wall

JIT_DYNASM_FILE = jit_dynasm jit_dynasm_opt1 jit_dynasm_opt2

.PHONY: clean

all: $(JIT_DYNASM_FILE)


jit_dynasm: dynasm-driver.c jit_dynasm.h
	$(CC) -g -Wall -Ithird_party -o $@ dynasm-driver.c -DJIT=\"jit_dynasm.h\"

jit_dynasm.h: jit_dynasm.dasc
	lua third_party/dynasm/dynasm.lua $^ > $@


jit_dynasm_opt1: dynasm-driver.c jit_dynasm_opt1.h
	$(CC) -g -Wall -Ithird_party -o $@ dynasm-driver.c -DJIT=\"jit_dynasm_opt1.h\"

jit_dynasm_opt1.h: jit_dynasm_opt1.dasc
	lua third_party/dynasm/dynasm.lua $^ > $@

jit_dynasm_opt2: dynasm-driver.c jit_dynasm_opt2.h
	$(CC) -g -Wall -Ithird_party -o $@ dynasm-driver.c -DJIT=\"jit_dynasm_opt2.h\"

jit_dynasm_opt2.h: jit_dynasm_opt2.dasc
	lua third_party/dynasm/dynasm.lua $^ > $@

clean:
	rm -f jit_dynasm jit_dynasm.h jit_dynasm_opt1 jit_dynasm_opt1.h jit_dynasm_opt2 jit_dynasm_opt2.h