export CC             = gcc
export CPATH          = $(shell echo `pwd`)
export COBJPATH       = $(CPATH)/bin
export CPLUGPATH      = $(COBJPATH)/plugins
export CEXE           = $(COBJPATH)/Cring
# export LDFLAGS        = -pthread -lreadline -ldl -lcurses -lm -Wl,-dynamic-list,plugin_system.list
export HISTORY_FILE   = $(COBJPATH)/.history
# export MACROS         = PLUG_DIR="\"$(CPLUGPATH)\""  _GNU_SOURCE HISTORY_FILE=\"$(HISTORY_FILE)\"
# export PLUGIN_CFLAGS  = $(CFLAGS)
# export PLUGIN_LDFLAGS = -fPIC
# export MACROFLAGS     = $(foreach m, $(MACROS), -D $m)
# export CFLAGS         = -Wall -O2 -std=gnu99 $(MACROFLAGS)#-D PLUG_DIR="\"$(CPLUGPATH)\"" -D _GNU_SOURCE #--pedantic -std = c11
export LIBREADLINE_PATH    = $(CPATH)/libreadline
export LIBREADLINE_AR      = $(foreach ar, libhistory.a libreadline.a, $(LIBREADLINE_PATH)/lib/$(ar))

.SILENT: 

.PHONY: readline_clean_lib readline_clean_src readline_clean_all

all: readline
	$(MAKE) $(MFLAGS) -C src

debug: CFLAGS += -D DEBUG -g
debug: all 
	$(MAKE) $(MFLAGS) -C src debug

# history: $(HISTORY_FILE)
# 	touch $(HISTORY_FILE)

readline: readline_lib
readline_lib: readline-6.3 $(LIBREADLINE_AR)

readline-6.3:
	tar -xvf ./readline-6.3.tar.gz


$(LIBREADLINE_AR): readline-6.3/Makefile
	$(MAKE) -C readline-6.3
	$(MAKE) -C readline-6.3 install-static
	$(MAKE) -C readline-6.3 clean
	echo $(LIBREADLINE_AR)

readline_clean_lib:
	rm -Rf $(LIBREADLINE_PATH)

readline_clean_src:
	rm -Rf readline-6.3
	rm -Rf config.log

readline_clean_all: readline_clean_lib readline_clean_src

readline-6.3/Makefile:
	cd readline-6.3; ./configure --prefix=$(LIBREADLINE_PATH)

clean:
	$(MAKE) $(MFLAGS) clean -C src
