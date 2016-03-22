#Ringo networking project




all	:
	make -C c/src
	make -C java/src

clean	:
	make -C c/src clean
	make -C java/src clean
