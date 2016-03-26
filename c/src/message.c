#include "message.h"
#include "common.h"
#include "protocol.h"

////////////////////////////////////////////////////////////////////////////////
// EXTERNAL
////////////////////////////////////////////////////////////////////////////////
extern int parseappmsg(char *message);


////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

typedef struct protocol_msg {
    char type[5];
    int (*action)(char *);
    // action should return -1 when message doesn't follow the protocol
    // so it's not retransmitted
} protocol_msg;


/***
 * list of supported message and there actions
 */
protocol_msg pmsg[] = {
    { "APPL", &parseappmsg },
    { "", NULL }
};


extern entity ent;

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

/***
 * Generate a unique message identificator
 *
 * @return message idenfitificator, a char* of strlen 8
 */
// TODO
static char *messageid() {
    char *id = (char *) malloc(9);
    snprintf(id, 8, "12345678");
    return id;
}




////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

/***
 * Parse a packet and call the appropriate function.
 * 
 * @param the packet message
 * @return returned value of the function called
 * @return -1 if the message has already been seen are is not supported
 */
int parsemsg(char *message) {
    if (message[4] != ' ' || message[12] != ' ') {
        fprintf(stderr, "Message not following the protocol.\n");
        return -1;
    }
    message[4]  = 0;
    message[12] = 0;
    char *type = message;
    char *idm  = &message[5];
    char *content = &message[13];
    //if (lookup(idm)) {
    //    verbose("Message already seen.\n");
    //    return -1;
    //}
    // search action to do
    for (int i = 0; pmsg[i].type[0] != 0; i++)
        if (strcmp(type, pmsg[i].type) == 0) {
            return (pmsg[i].action(content));
        }
    // message not supported
    verbose("Message of type %s not supported.\n", type);
    // remove message from the list because not supported
    //lookup(idm);
    return -1;
}



void sendmessage(char *type, char *content) {
    char buff[513];
    snprintf(buff, 513, "%s %s", type, content);
    sendpacket(buff);
}

