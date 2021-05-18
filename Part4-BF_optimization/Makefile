CC = clang
CFLAGS = -c -g -Wall


SRCS  = opt_1.c
OBJS  = opt_1.o
PROGS = opt_1

JIT_DYNASM_FILE = opt_2 opt_3 opt_4

.PHONY: clean

all: $(PROGS) $(JIT_DYNASM_FILE) BF_interpreter_opt_1 BF_interpreter_opt_2 BF_interpreter_statistic

# $(JIT_DYNASM_FILE): dynasm-driver.c $(JIT_DYNASM_FILE).h
# 	$(CC) -g -Wall -Ithird_party -o $@ dynasm-driver.c -DJIT=\"$(JIT_DYNASM_FILE).h\"

# $(JIT_DYNASM_FILE).h: $(JIT_DYNASM_FILE).dasc
# 	lua third_party/dynasm/dynasm.lua $^ > $@

opt_2: dynasm-driver.c opt_2.h
	$(CC) -g -Wall -Ithird_party -o $@ dynasm-driver.c -DJIT=\"opt_2.h\"

opt_2.h: opt_2.dasc
	lua third_party/dynasm/dynasm.lua $^ > $@


opt_3: dynasm-driver.c opt_3.h
	$(CC) -g -Wall -Ithird_party -o $@ dynasm-driver.c -DJIT=\"opt_3.h\"

opt_3.h: opt_3.dasc
	lua third_party/dynasm/dynasm.lua $^ > $@

opt_4: dynasm-driver.c opt_4.h
	$(CC) -g -Wall -Ithird_party -o $@ dynasm-driver.c -DJIT=\"opt_4.h\"

opt_4.h: opt_4.dasc
	lua third_party/dynasm/dynasm.lua $^ > $@


BF_interpreter_opt_1: BF_interpreter_opt1.c
	$(CC) -g -Wall $^ -o $@

BF_interpreter_opt_2: BF_interpreter_opt2.c
	$(CC) -g -Wall $^ -o $@

BF_interpreter_statistic: BF_interpreter_statistic.cpp
	$(CC)++ -std=c++14 -g -Wall $^ -o $@ -DBFTRACE




$(PROGS): $(OBJS)
	$(CC) -o $@ $^

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(PROGS) opt_2.h opt_2 opt_3 opt_3.h opt_4 opt_4.h BF_interpreter_opt_1 BF_interpreter_opt_2 BF_interpreter_statistic