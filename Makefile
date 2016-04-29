#Ringo networking project




all	: c java

.PHONY	: c java

c	:
	$(MAKE) $(MFLAGS) -C c

java	:
	$(MAKE) $(MFLAGS) -C java/src

debug	:
	$(MAKE) $(MFLAGS) -C c debug

help	:
	@echo -e "Available $(MAKE) options :\n\tmake C\n\tmake Java"
	@echo -e "\t$(MAKE) debug\n\tmake help\n\tmake clean"

clean	:
	$(MAKE) $(MFLAGS) -C c clean
	$(MAKE) $(MFLAGS) -C java/src clean

.PHONY: clean help
