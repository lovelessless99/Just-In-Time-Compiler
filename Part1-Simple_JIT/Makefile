CC = clang
CFLAG = -c -g -Wall

SRCS  = jit_toy.c  obj_jit_toy.c
OBJS  = $(patsubst %.c,%.o,$(SRCS))
PROGS = $(patsubst %.c,%,$(SRCS))

JIT_DYNASM_FILE = jit_dynasm
OBJ_JIT_DYNASM_FILE = obj_jit_dynasm


TEST_TARGETS = $(patsubst %.c,test_%,$(SRCS)) test_jit_dynasm

.PHONY: clean $(TEST_TARGETS)

all: $(PROGS) 

$(PROGS): %: %.o
	$(CC) -o $@ $<

%.o: %.c
	$(CC) $(CFLAG) -o $@ $<

$(JIT_DYNASM_FILE): dynasm-driver.c $(JIT_DYNASM_FILE).h
	$(CC) -g -Wall -Ithird_party -o $(JIT_DYNASM_FILE) dynasm-driver.c -DJIT=\"$(JIT_DYNASM_FILE).h\"

$(JIT_DYNASM_FILE).h: $(JIT_DYNASM_FILE).dasc
	lua third_party/dynasm/dynasm.lua $(JIT_DYNASM_FILE).dasc > $(JIT_DYNASM_FILE).h

$(OBJ_JIT_DYNASM_FILE): dynasm-driver.c $(OBJ_JIT_DYNASM_FILE).h
	$(CC) -g -Wall -Ithird_party -o $(OBJ_JIT_DYNASM_FILE) dynasm-driver.c -DJIT=\"$(OBJ_JIT_DYNASM_FILE).h\"

$(OBJ_JIT_DYNASM_FILE).h: $(OBJ_JIT_DYNASM_FILE).dasc mul.o
	lua third_party/dynasm/dynasm.lua $(OBJ_JIT_DYNASM_FILE).dasc > $(OBJ_JIT_DYNASM_FILE).h

mul.o: mul.c
	$(CC) $(CFLAG) -o $@ $<

$(TEST_TARGETS): test_%: %
	@./$< 56
	@echo $?


clean:
	rm -f $(OBJS) $(PROGS) $(JIT_DYNASM_FILE) $(JIT_DYNASM_FILE).h $(OBJ_JIT_DYNASM_FILE) $(OBJ_JIT_DYNASM_FILE).h mul.o
