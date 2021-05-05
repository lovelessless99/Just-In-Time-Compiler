CC = clang
CFLAGS = -g -Wall

BF_SED = sed_BF
BF_INTERPRETER = BF_interpreter
BF_COMPILER_X64 = BF_compiler_x64

.PHONY: clean

all: $(BF_SED) $(BF_INTERPRETER) $(BF_COMPILER_X64)


$(BF_COMPILER_X64): $(BF_COMPILER_X64).c
	$(CC) $(CFLAGS) -o $@ $^

run_compiler: $(BF_COMPILER_X64)
	./$< hello.bf > hello.s
	$(CC) -o hello-x64 hello.s

$(BF_INTERPRETER): $(BF_INTERPRETER).c
	$(CC) $(CFLAGS) -o $@ $^
	
$(BF_SED): $(BF_SED).c
	$(CC) $(CFLAGS) -o $@ $^

$(BF_SED).c: BF_2_C.sed hello.bf 
	sed -f $< hello.bf > $(BF_SED).c 

clean:
	rm -f $(BF_SED).c $(BF_SED) $(BF_INTERPRETER) $(BF_COMPILER_X64) hello.s hello-x64