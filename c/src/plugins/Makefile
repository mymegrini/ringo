.PHONY	: all clean

PLUGINS = pong chat download zyva


all	:
	@$(foreach plug,$(PLUGINS),make -C $(plug);)
	# foreach plug,$(PLUGINS),make -C $$plug
	# make -C chat
	# make -C zyva
	# make -C pong

clean	:
	@$(foreach plug,$(PLUGINS),make -C $(plug) clean;)
	# @$(foreach plug,$(PLUGINS),make -C $$plug clean)
	# make -C chat clean
	# make -C zyva clean
	# make -C pong clean

