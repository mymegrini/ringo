CSRC		= $(wildcard *.c)
OBJS		=  $(CSRC:%.c=$(COBJPATH)%.o)


all		: $(OBJS)

$(COBJPATH)%.o 	: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

.PHONY: clean
clean		:
		rm -f $(OBJS)
