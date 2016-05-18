#include "application.h"
#include "../common.h"
#include "network.h"
#include "message.h"
#include "protocol.h"
#include "../plugin_system/plugin_system.h"

#include <stdio.h>

extern void action_diff(char *mess, char *content, int lookup_flag);

application app[] = {
  { "DIFF####", "Diff message used to send message over the ring.", action_diff },
  { "", "", NULL}
};


////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

/***
 * Call the application function corresponding to an APPL message
 *
 * @param APPL message
 * @return 1 when the app is supported
 * @return 0 when the app is not supported
 * @return -1 when the message doesn't follow the protocol
 */
int parseappmsg(char *message, char *content, int lookup_flag) {
    /*if (message[CTNT_OFFST+4] != ' ') {*/
    if (content[8] != ' ') {
        fprintf(stderr, "APPL message not following the protocol.\n");
        return -1;
    }
    int r = 0;
    char idapp[9];
    strncpy(idapp, content, 8);
    idapp[8] = 0;
    char *app_content = &content[9];
    verbose("Looking for APPL %s.\n", idapp);
    // search app and execute
    for (int i = 0; app[i].id[0] != 0; ++i)  {
        if (strcmp(app[i].id, idapp) == 0) {
            verbose("APPL %s found.\n", idapp);
            app[i].app(message, app_content, lookup_flag);
            r = 1;
            break;
        }
    }
    // look for plugin action
    /* plug_action *pa; */
    /* if (find((void **)&pa, plugin_manager.action, idapp)) { */
    /*   return pa->action(message, app_content, lookup_flag); */
    /* } */
    // application not found, retransmit the message if not seen.
    r =  exec_plugin_action(&plugin_manager, idapp, 
        message, app_content, lookup_flag);
    if (r != -1)
      return r;
    if (!lookup_flag)
      sendpacket_all(message);
    return 0;
}




void makeappmessage(char* idm, char* buff, const char* idapp,
        const char* format, va_list aptr)
{
    char content[490];

    // creating message content
    vsnprintf(content, 490, format, aptr);
    
    // creating new id
    messageid(idm);

    // adding id to list of known ids, and checking for collisions
/* #ifdef DEBUG */
/*     int r = */
/* #endif */
/*         lookup(id); */
/* #ifdef DEBUG */
/*     if (r==1) debug("makemessage", "Detected a hash collision: %s\n", id); */
/* #endif */

    // creating message
    if (strlen(content))
      snprintf(buff, 513, "APPL %s %s %s", idm, idapp, content);
    else snprintf(buff, 513, "APPL %s %s", idm, idapp);

    return;
}

void sendappmessage_all(const char *type, const char *format, ...)
{
    char content[490];

    va_list aptr;
    va_start(aptr, format);
    vsnprintf(content, 490, format, aptr);
    va_end(aptr);

    sendmessage_all("APPL", "%s %s", type, content);
}
