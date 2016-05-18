#ifndef PLUGIN_PROGRAMMER_INTERFACE_H
#define PLUGIN_PROGRAMMER_INTERFACE_H


#ifndef NRING
#define NRING 2
#endif

/* #include "plugin_system.h" */
#include <stdarg.h>
#include <inttypes.h>
#include <stdlib.h>
#include <signal.h>
#include "../list.h"




////////////////////////////////////////////////////////////////////////////////
// PLUGIN DECLARATION
////////////////////////////////////////////////////////////////////////////////

typedef struct _PluginManager PluginManager;

extern PluginManager plugin_manager;



typedef int (*PluginAction)(const char *, const char *, int); // action to do, take the message, content and lookup_flag
typedef int (*PluginCommand)(int, char **); // plugin command
typedef int (*PluginInitFunc)(PluginManager*);
typedef void (*PluginCloseFunc)();



typedef struct PluginAction_t {
  char         name[9];
  char         desc[512];
  PluginAction action;
} PluginAction_t;



typedef struct PluginCommand_t {
  char          name[30];
  char          desc[512];
  PluginCommand command;
} PluginCommand_t;



typedef struct Plugin {
  int             size_command;
  PluginCommand_t *command;
  int             size_action;
  PluginAction_t  *action;
  PluginCloseFunc close_plugin;
} Plugin;



extern int plugin_register(PluginManager *plug_manager, const char *name, Plugin *plug);

//// END OF PLUGIN DECLARATION



////////////////////////////////////////////////////////////////////////////////
// PROTOCOL COMMUNICATION
////////////////////////////////////////////////////////////////////////////////

void retransmit(const char *message);
void send_message(const char *idapp, const char *format, ...);
const char *get_message();

//// END OF PROTOCOL COMMUNICATION



////////////////////////////////////////////////////////////////////////////////
// ACCESS TO RING DATA
////////////////////////////////////////////////////////////////////////////////

typedef struct info_t {
    const char     id[9];
    const char     ip_self[16];
    const uint16_t udp;
    const uint16_t tcp;
    const char     ip_next[NRING][16];
    const uint16_t port_next[NRING];
    const char     mdiff_ip[NRING][16];
    const uint16_t mdiff_port[NRING];
} info_t;

extern info_t * const info;
extern volatile const int * const ring_number;

//// END OF ACCESS TO RING DATA


////////////////////////////////////////////////////////////////////////////////
// EXTERNAL TOOLS
////////////////////////////////////////////////////////////////////////////////
typedef struct xterm* xterm;

extern int init_xterm_communication(xterm *x);
extern int xterm_printf(xterm x, char *format, ...);
extern int xterm_read(xterm x, char *buff, size_t count);
extern ssize_t xterm_getline(xterm x, char **line, size_t *n);
extern pid_t xterm_getpid(xterm x);
extern int xterm_getoutput(xterm x);
extern void xterm_close(xterm *x);

//// END OF EXTERNAL TOOLS


#endif /* C_PLUGIN_PROGRAMMER_INTERFACE_H */
