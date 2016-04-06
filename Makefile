#Ringo networking project




all	: C Java

C	:
	make -C c/src

Java	:
	make -C java/src

debug	:
	make -C c/src -f Makedebug

help	:
	@echo "Available make options :\n\tmake C\n\tmake Java"
	@echo "\tmake debug\n\tmake help\n\tmake clean"

clean	:
	make -C c/src clean
	make -C java/src clean
