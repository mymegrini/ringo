include Makefile.inc

export COBJPATH = $(shell echo `pwd`)/bin/
export CFLAGS 		= -Wall -O2 -std=gnu99 # #--pedantic -std=c11
export LIBLINKS	= -pthread -lreadline
export CEXE = $(COBJPATH)Cring

all:
	$(MAKE) $(MFLAGS) -C src

debug: CFLAGS += -D DEBUG -g
debug: DFLAG = debug
debug: all

clean:
	$(MAKE) $(MFLAGS) clean -C src
