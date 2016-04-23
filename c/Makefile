export CC             = gcc
export CFLAGS         = -Wall -O2 -std=gnu99 #--pedantic -std = c11
export CPATH          = $(shell echo `pwd`)
export COBJPATH       = $(CPATH)/bin
export CPLUGPATH      = $(COBJPATH)/plugins
export CEXE           = $(COBJPATH)/Cring
export LDFLAGS        = -pthread -lreadline -ldl -Wl,-dynamic-list,plugin_system.list
export PLUGIN_CFLAGS  = $(CFLAGS)
export PLUGIN_LDFLAGS = -fPIC

# .SILENT: 


all:
	$(MAKE) $(MFLAGS) -C src

debug: CFLAGS += -D DEBUG -g
debug: all 
	#$(MAKE) $(MFLAGS) -C src debug

clean:
	$(MAKE) $(MFLAGS) clean -C src
