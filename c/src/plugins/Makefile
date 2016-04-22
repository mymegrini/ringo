CSRC		= $(wildcard *.c)
CPLUGPATH = $(COBJPATH)plugins/
#DOBJS		= $($(CPLUGPATH):%.o=$(CPLUGPATH)%so)
#OBJS		=  $(CSRC:%.c=$(CPLUGPATH)%.o)
DOBJS		=  $(CSRC:%.c=$(CPLUGPATH)%.so)
OBJS		=  $(CSRC:%.c=$(CPLUGPATH)%.o)
STATIC  =  $(COBJPATH)plugin_system.o $(COBJPATH)list.o


all		: $(DOBJS)

$(CPLUGPATH)%.so	: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(STATIC) -shared

$(CPLUGPATH)%.o 	: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

.PHONY: clean
clean		:
	rm -f $(DOBJS)
	rm -f $(OBJS)
