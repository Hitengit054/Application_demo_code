TARGETS=client2

CROSS_TOOL=
CC_C=$(CROSS_TOOL)gcc

all: clean $(TARGETS)

$(TARGETS): 
	$(CC_C) $@.c -o $@
	$(CC_C) server.c create_c.c file_read.c -o server -pthread
	
clean: 
	rm -f $(TARGETS)




	
