CSRC		= $(wildcard *.c)
DOBJS		=  $(CSRC:%.c=$(CPLUGPATH)/%.so)
OBJS		=  $(CSRC:%.c=$(CPLUGPATH)/%.o)

all		: $(DOBJS)

$(CPLUGPATH)/%.so	: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< -shared

$(CPLUGPATH)/%.o 	: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

.PHONY: clean
clean		:
	rm -f $(DOBJS)
	rm -f $(OBJS)
