SUBDIRS = BF_interpreter_opt/. JIT_Dynasm/. JIT_Opcode/.

all: $(SUBDIRS) 

.PHONY: $(SUBDIRS) clean

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
	

clean: $(SUBDIRS) 
	