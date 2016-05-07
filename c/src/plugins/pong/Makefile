PONG_PATH	= -D PONG_PATH="\"$(shell echo `pwd`)/\""

CSRC		= pong.c gui.c render.c netcode.c engine.c
DOBJS		= $(CPLUGPATH)/pong.so
OBJS		= $(CSRC:%.c=$(CPLUGPATH)/%.o)
HSDL		= `pkg-config --cflags sdl2`
LIBSDL		= `pkg-config --libs sdl2`
PLUGIN          = ../../plugin_system/plugin_interface.h
PROTOCOL        = ../../plugin_system/protocol_interface.h

.PHONY: all clean

all		: $(DOBJS)

$(DOBJS)	: $(OBJS)
	$(CC) $(PLUGIN_CFLAGS) -o $@ $^ -shared $(LIBSDL)

$(CPLUGPATH)/%.o: %.c
	$(CC) $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS) $(PONG_PATH) $(HSDL) -c -o $@ $<

$(CPLUGPATH)/pong.o: pong.h gui.h render.h $(PLUGIN)
$(CPLUGPATH)/gui.o: gui.h render.h netcode.h engine.h
$(CPLUGPATH)/render.o: render.h netcode.h engine.h
$(CPLUGPATH)/netcode.o: netcode.h pong.h engine.h $(PROTOCOL)
$(CPLUGPATH)/engine.o: engine.h pong.h $(PROTOCOL)

clean		:
	rm -f $(DOBJS)
	rm -f $(OBJS)
