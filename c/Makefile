export CC             = gcc
export CPATH          = $(shell echo `pwd`)
export COBJPATH       = $(CPATH)/bin
export CPLUGPATH      = $(COBJPATH)/plugins
export CEXE           = $(COBJPATH)/Cring
export LDFLAGS        = -pthread -lreadline -ldl -lcurses -lm -Wl,-dynamic-list,plugin_system.list
export HISTORY_FILE   = $(COBJPATH)/.history
export MACROS         = PLUG_DIR="\"$(CPLUGPATH)\""  _GNU_SOURCE HISTORY_FILE=\"$(HISTORY_FILE)\"
export PLUGIN_CFLAGS  = $(CFLAGS)
export PLUGIN_LDFLAGS = -fPIC
export MACROFLAGS     = $(foreach m, $(MACROS), -D $m)
export CFLAGS         = -Wall -O2 -std=gnu99 $(MACROFLAGS)#-D PLUG_DIR="\"$(CPLUGPATH)\"" -D _GNU_SOURCE #--pedantic -std = c11

.SILENT: 


all:
	$(MAKE) $(MFLAGS) -C src

debug: CFLAGS += -D DEBUG -g
debug: all 
	#$(MAKE) $(MFLAGS) -C src debug

history: $(HISTORY_FILE)
	touch $(HISTORY_FILE)

clean:
	$(MAKE) $(MFLAGS) clean -C src
