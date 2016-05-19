.PHONY	: all clean

PLUGINS = chat download #pong


all	:
	@$(foreach plug,$(PLUGINS),make -C $(plug);)

clean	:
	@$(foreach plug,$(PLUGINS),make -C $(plug) clean;)

