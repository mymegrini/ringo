CSRC		= pong.c gui.c render.c
DOBJS		= $(CPLUGPATH)/pong.so
OBJS		= $(CSRC:%.c=$(CPLUGPATH)/%.o)
HSDL		= `pkg-config --cflags sdl2`
LIBSDL		= `pkg-config --libs sdl2 SDL2_gfx`

.PHONY: all clean

all		: $(DOBJS)

$(DOBJS)	: $(OBJS)
	$(CC) $(PLUGIN_CFLAGS) -o $@ $^ -shared $(LIBSDL)

$(CPLUGPATH)/%.o: %.c
	$(CC) $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS) $(HSDL) -c -o $@ $<

$(CPLUGPATH)/pong.o: pong.h gui.h render.h ../../plugin_system/plugin_interface.h
$(CPLUGPATH)/gui.o: gui.h render.h
$(CPLUGPATH)/render.o: render.h netcode.h
$(CPLUGPATH)/netcode.o: netcode.h


clean		:
	rm -f $(DOBJS)
	rm -f $(OBJS)
