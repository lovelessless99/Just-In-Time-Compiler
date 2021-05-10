SUBDIRS = Stage-1/. Stage-2/.

all: $(SUBDIRS) 

.PHONY: $(SUBDIRS) 

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
	

clean: $(SUBDIRS) 
	rm -f total_time_summarize comparison.png