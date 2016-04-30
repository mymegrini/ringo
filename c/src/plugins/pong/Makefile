CSRC		= $(wildcard *.c)
DOBJS		= $(CPLUGPATH)/pong.so
OBJS		= $(CSRC:%.c=$(CPLUGPATH)/%.o)
HSDL		= `pkg-config --cflags sdl2`
LIBSDL		= `pkg-config --libs sdl2 SDL2_gfx`

all		: $(DOBJS)

$(CPLUGPATH)/%.so	: $(OBJS)
	$(CC) $(PLUGIN_CFLAGS) -o $@ $^ -shared $(LIBSDL) -static

$(CPLUGPATH)/%.o 	: %.c
	$(CC) $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS) $(HSDL) -c -o $@ $<

.PHONY: clean
clean		:
	rm -f $(DOBJS)
	rm -f $(OBJS)
