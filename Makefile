#Ringo networking project




all	: C Java

C	:
	$(MAKE) $(MFLAGS) -C c

Java	:
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
