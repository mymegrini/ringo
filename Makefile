#Ringo networking project




all	: C Java

C	:
	make -C c/src

Java	:
	make -C java/src

clean	:
	make -C c/src clean
	make -C java/src clean
