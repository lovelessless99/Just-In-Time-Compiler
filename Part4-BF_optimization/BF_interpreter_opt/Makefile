CC = clang
CFLAGS = -c -g -Wall

output_dir = ..

.PHONY: clean

all: BF_interpreter_opt_1 BF_interpreter_opt_2 BF_interpreter_statistic BF_interpreter_opt_3

BF_interpreter_opt_1: BF_interpreter_opt1.c
	$(CC) -g -Wall $^ -o $@

BF_interpreter_opt_2: BF_interpreter_opt2.c
	$(CC) -g -Wall $^ -o $@

BF_interpreter_opt_3: BF_interpreter_opt3.c
	$(CC) -g -Wall $^ -o $@

BF_interpreter_statistic: BF_interpreter_statistic.cpp
	$(CC)++ -std=c++14 -g -Wall $^ -o $@ -DBFTRACE


clean:
	rm -f BF_interpreter_opt_1 BF_interpreter_opt_2 BF_interpreter_statistic BF_interpreter_opt_3
