#include "application.h"
#include "common.h"


////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

/***
 * list of supported applications and there actions
 */
typedef struct application {
    char id[9];
    void (*app)(char *); // action to do, take the message as argument
} application;

application app[] = {
    { "", NULL}
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
int parseappmsg(char *message) {
    if (message[4] != ' ') {
        fprintf(stderr, "APPL message not following the protocol.\n");
        return -1;
    }
    message[4] = 0;
    char *idapp = message, *content = &message[5];
    // search app and execute
    for (int i = 0; app[i].id[0] != 0; i++) 
        if (strcmp(app[i].id, idapp) == 0) {
            app[i].app(message);
            return 1;
        }
    // app not found
    return 0;
}




