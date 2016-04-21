CSRC		= $(wildcard *.c)
OBJS		=  $(CSRC:%.c=$(COBJPATH)%.o)
EARLY_DIRS = protocol
LATE_DIRS = shell plugin_system
LIST = $(LATE_DIRS) $(EARLY_DIRS)



.SILENT: 
.PHONY: clean main $(LIST) makepath

all		: makepath $(CEXE)

debug: CFLAGS += -D DEBUG -g
debug: all

path: $(MAKE) $(MKFLAGS) -C .. path


$(CEXE)		: main $(LIST)
	$(CC) -o $@ $(COBJPATH)*.o $(LIBLINKS)


main: $(OBJS)


$(LIST): force_look
	for d in $@; do ($(MAKE) $(MFLAGS) -C $$d); done


$(COBJPATH)%.o 	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean		:
	@echo "rm -f $(CEXE) $(OBJS)"
	for d in $(LIST); do echo "$(MAKE) $(MFLAGS) -C $$d clean"; done
	rm -f $(CEXE) $(OBJS) 
	for d in $(LIST); do $(MAKE) $(MFLAGS) -C $$d clean; done

force_look:
	true

