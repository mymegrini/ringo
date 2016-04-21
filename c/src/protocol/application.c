#include "application.h"
#include "common.h"
#include "network.h"
#include "message.h"

#include <stdarg.h>
#include <stdio.h>

extern void action_chat(char *mess);

application app[] = {
  { "DIFF####", "Chat message.", action_chat },
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
int parseappmsg(char *message, char *content) {
    /*if (message[CTNT_OFFST+4] != ' ') {*/
  debug(MAGENTA "parseappmessage", MAGENTA "content: %s", content);
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
            app[i].app(app_content);
            r = 1;
            break;
        }
    }
    return r;
}




void sendappmessage_all(char *type, char *format, ...)
{
    char content[490];

    va_list aptr;
    va_start(aptr, format);
    vsnprintf(content, 490, format, aptr);
    va_end(aptr);

    sendmessage_all("APPL", "%s %s", type, content);
}
