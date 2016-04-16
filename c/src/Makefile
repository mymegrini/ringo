ifndef COBJFLAGS
		export COBJFLAGS=../bin
		include ../Makefile.inc
endif
CSRC		= $(wildcard *.c)
OBJS		=  $(CSRC:%.c=$(COBJPATH)%.o)
EARLY_DIRS = protocol
LATE_DIRS = shell
LIST = $(LATE_DIRS) $(EARLY_DIRS)



.PHONY: clean main $(LIST)

all		: $(CEXE)


$(CEXE)		: main $(LIST)
	$(CC) -o $@ $(COBJPATH)*.o $(LIBLINKS)


main: $(OBJS)


$(LIST): force_look
	for d in $@; do ($(MAKE) $(MFLAGS) -C $$d); done


$(COBJPATH)%.o 	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean		:
	rm -f $(CEXE) $(OBJS) 
	for d in $(LIST); do (cd $$d && $(MAKE) clean); done

force_look:
	true

