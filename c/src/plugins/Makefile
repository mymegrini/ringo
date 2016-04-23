CSRC		= $(wildcard *.c)
#DOBJS		= $($(CPLUGPATH):%.o=$(CPLUGPATH)%so)
#OBJS		=  $(CSRC:%.c=$(CPLUGPATH)%.o)
DOBJS		=  $(CSRC:%.c=$(CPLUGPATH)/%.so)
OBJS		=  $(CSRC:%.c=$(CPLUGPATH)/%.o)
EXTOBJ	= plugin_system.o list.o application.o protocol.o #message.o protocol.o common.o application.o listmsg.o network.o thread.o #protocol_interface.o 
STATIC  =  $(foreach obj, $(EXTOBJ), $(COBJPATH)/$(obj))


all		: $(DOBJS)

$(CPLUGPATH)/%.so	: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(STATIC) -shared

$(CPLUGPATH)/%.o 	: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

.PHONY: clean
clean		:
	rm -f $(DOBJS)
	rm -f $(OBJS)
