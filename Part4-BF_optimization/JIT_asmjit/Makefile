CC = clang++
CFLAGS = -g -Wall --std=c++11

JIT_asmjit_FILE = jit_asmjit jit_asmjit_opt1 build/libasmjit.a

.PHONY: clean

all: $(JIT_asmjit_FILE)

build/libasmjit.a: third_party/asmjit
	cmake -B build -DASMJIT_STATIC=ON ./third_party/asmjit && cmake --build build

jit_asmjit: jit_asmjit.cpp build/libasmjit.a
	$(CC) $(CFLAGS) -lrt -Ithird_party/asmjit/src $^  -o $@


jit_asmjit_opt1: jit_asmjit_opt1.cpp build/libasmjit.a
	$(CC) $(CFLAGS) -lrt -Ithird_party/asmjit/src $^  -o $@

clean:
	rm -f jit_asmjit jit_asmjit_opt1
