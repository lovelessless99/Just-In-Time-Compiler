CC = clang
CFLAGS = -c -g -Wall


SRCS  = jit_opcode.c
OBJS  = jit_opcode.o
PROGS = jit_opcode

.PHONY: clean

all: $(PROGS) $(JIT_DYNASM_FILE)

$(PROGS): $(OBJS)
	$(CC) -o $@ $^

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(PROGS)