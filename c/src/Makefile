CSRC		= $(wildcard *.c)
OBJS		=  $(CSRC:%.c=$(COBJPATH)/%.o)
EARLY_DIRS = protocol
LATE_DIRS = plugin_system shell plugins
LIST = $(EARLY_DIRS) $(LATE_DIRS)



#.SILENT: 
.PHONY: clean main $(LIST) 

all		: $(CEXE)

debug: CFLAGS += -D DEBUG -g
debug: all


$(CEXE)		: main $(LIST)
	$(CC) -o $@ $(COBJPATH)/*.o $(LDFLAGS) #-rdynamic #-Wl,-dynamic-list,plugin_system.list


main: $(OBJS)


$(LIST): force_look
	for d in $@; do (echo "entering $$d..."; $(MAKE) $(MFLAGS) -C $$d); done


$(COBJPATH)/%.o 	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean		:
	rm -f $(CEXE) $(OBJS)
	for d in $(LIST); do (echo "entering $$d..."; $(MAKE) clean -C $$d); done

force_look:
	true

