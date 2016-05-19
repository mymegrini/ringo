CSRC         = $(filter-out PluginSystem.c,$(wildcard *.c))
OBJS         = $(CSRC:%.c=$(COBJPATH)/%.o)
SYMTABLE     = -Wl,-dynamic-list,plugin_system.list
PLUGSYSFLAGS = $(CFLAGS) $(SYMTABLE)

all		: $(OBJS)

$(COBJPATH)/%.o 	: %.c
	$(CC) $(PLUGSYSFLAGS) -c -o $@ $<

.PHONY: clean
clean		:
		rm -f $(OBJS)
