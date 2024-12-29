SUBDIRS = BF_interpreter_opt/. JIT_Dynasm/. JIT_Opcode/. JIT_asmjit/.

all: $(SUBDIRS) 

.PHONY: $(SUBDIRS) clean

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
	

clean: $(SUBDIRS) 
	