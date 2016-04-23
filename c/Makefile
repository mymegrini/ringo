export CC	= gcc
export CFLAGS 		= -Wall -O2 -std=gnu99 # #--pedantic -std=c11
export CPATH = $(shell echo `pwd`)
export COBJPATH = $(CPATH)/bin
export CPLUGPATH = $(COBJPATH)/plugins
export LIBLINKS	= -pthread -lreadline -ldl
export CEXE = $(COBJPATH)/Cring

.SILENT: 


all:
	$(MAKE) $(MFLAGS) -C src

debug: CFLAGS += -D DEBUG -g
debug: all 
	#$(MAKE) $(MFLAGS) -C src debug

clean:
	$(MAKE) $(MFLAGS) clean -C src
