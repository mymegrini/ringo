CSRC		= $(wildcard *.c)
DOBJS		=  $(CSRC:%.c=$(CPLUGPATH)/%.so)
OBJS		=  $(CSRC:%.c=$(CPLUGPATH)/%.o)

all		: $(DOBJS)

$(CPLUGPATH)/%.so	: $(CPLUGPATH)/%.o
	$(CC) $(PLUGIN_CFLAGS) -o $@ $< -shared

$(CPLUGPATH)/%.o 	: %.c
	$(CC) $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS) -c -o $@ $<

.PHONY: clean
clean		:
	rm -f $(DOBJS)
	rm -f $(OBJS)
