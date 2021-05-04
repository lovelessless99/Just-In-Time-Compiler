CC = clang
CFLAGS = -g -Wall

SED_BF = sed_BF

.PHONY: clean

all: $(SED_BF)

$(SED_BF): $(SED_BF).c
	$(CC) $(CFLAGS) -o $(SED_BF) $^

$(SED_BF).c: BF_2_C.sed hello.bf 
	sed -f $< hello.bf > $(SED_BF).c 

clean:
	rm $(SED_BF).c $(SED_BF)