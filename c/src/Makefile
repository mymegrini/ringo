CSRC		= $(wildcard *.c)
OBJS		=  $(CSRC:%.c=$(COBJPATH)/%.o)
EARLY_DIRS = protocol
LATE_DIRS = plugin_system shell plugins
LIST = $(EARLY_DIRS) $(LATE_DIRS)

export CFLAGS         = -Wall -O2 -std=gnu99 $(MACROFLAGS)#-D PLUG_DIR="\"$(CPLUGPATH)\"" -D _GNU_SOURCE #--pedantic -std = c11
export LDFLAGS        = -pthread -ldl -lncurses -lm -Wl,-dynamic-list,plugin_system.list -ltermcap #-lreadline 
export ARFLAGS        = $(LIBREADLINE_AR)
export MACROS         = PLUG_DIR="\"$(CPLUGPATH)\""  _GNU_SOURCE HISTORY_FILE=\"$(HISTORY_FILE)\"
export PLUGIN_CFLAGS  = $(CFLAGS)
export PLUGIN_LDFLAGS = -fPIC
export MACROFLAGS     = $(foreach m, $(MACROS), -D $m)
export C_INCLUDE_PATH      = $(LIBREADLINE_PATH)/include


#.SILENT: 
.PHONY: clean main $(LIST) 

all		: $(CEXE)

debug: CFLAGS += -D DEBUG -g
debug: all


$(CEXE)		: main $(LIST)
	$(CC) -o $@ $(COBJPATH)/*.o $(ARFLAGS) $(LDFLAGS) #-rdynamic #-Wl,-dynamic-list,plugin_system.list


main: $(OBJS)


$(LIST): force_look
	for d in $@; do (echo "entering $$d..."; $(MAKE) $(MFLAGS) -C $$d); done


$(COBJPATH)/%.o 	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean		:
	rm -f $(CEXE) $(OBJS)
	for d in $(LIST); do (echo "entering $$d..."; $(MAKE) clean -C $$d); done

force_look:
	true

