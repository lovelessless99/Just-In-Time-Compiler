CC = clang
CFLAG = -c -g -Wall

SRCS  = $(wildcard *.c)
OBJS  = $(patsubst %.c,%.o,$(SRCS))
PROGS = $(patsubst %.c,%,$(SRCS))

JIT_DYNASM_FILE = jit_dynasm


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


$(TEST_TARGETS): test_%: %
	@./$< 56
	@echo $?

clean:
	rm -f $(OBJS) $(PROGS) $(JIT_DYNASM_FILE) $(JIT_DYNASM_FILE).h
