#include "message.h"
#include "common.h"
#include "protocol.h"
#include "stdint.h"
#include "listmsg.h"
#include "thread.h"

#include <stdarg.h>
#include <sys/time.h>

////////////////////////////////////////////////////////////////////////////////
// EXTERNAL
////////////////////////////////////////////////////////////////////////////////
extern int parseappmsg(char *message);
static int action_whos(char *content);
static int action_gbye(char *content);
static int action_eybg(char *content);

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
    { "WHOS", action_whos },
    { "APPL", parseappmsg },
    { "GBYE", action_gbye },
    { "EYBG", action_eybg },
    { "", NULL }
};


extern entity ent;
extern int nring;
extern _entity _ent;
int wait_goodbye = 0;

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////


/**
 * Generate a unique message identificator
 *
 * @param content message content to be included in hash
 * @return message idenfitificator, a char* of strlen 8
 */
static char* messageid(char* content) {

    struct timeval time;
    uint64_t h = 5381;
    char* hash = malloc(9*sizeof(char));
    int i;
    uint8_t c;

    /* hash * 33 + c */
    // hashing time
    gettimeofday(&time, NULL);
    h = ((h << 5) + h) + (uint16_t)time.tv_sec;
    h = ((h << 5) + h) + (uint16_t)time.tv_usec;
    // hashing ip and port
    for(i=0; i<16; i++) h = ((h << 5) + h) + ent.ip_self[i];  
    h = ((h << 5) + h) + ent.udp;
    // hashing content
    while((c = *content++)) h = ((h << 5) + h) + c;

    // creating hash using human readable characters
    for(i=0; i<8; i++){
        c = h % 62;
        if (c<10) hash[i] = c+48;      //digits
        else if (c<36) hash[i] = c-10+97; //lowercase letters
        else if (c<62) hash[i] = c-36+65; //uppercase letters
        else hash[i] = 0;
    
        h = h / 62;
    }
    hash[8] = 0;

    debug("messageid", "Created message id : %s.\n", hash);
    return hash;
}


static int action_whos(char *content) {
  
    sendmessage_all("MEMB", "%s %s %d", ent.id, ent.ip_self, ent.udp);
    return 0;
}


static int action_gbye(char *content) {

    if (content[15] != ' ' || content[20] != ' ' || content[36] != ' ') {
        debug("action_gbye", "problem with spaces");
        return 1;
    }
    char ip[16], port_str[5], ip_next[16], port_next[5];
    int r = sscanf(content, "%s %s %s %s", ip, port_str, ip_next, port_next);
    if (r != 4 ||
        !isip(ip) || !isport(port_str) || !isip(ip_next) || !isport(port_next)) {
        // don't retransmit message
        debug("action_gbye", "sscanf test not passed, j = %d.\nip:\"%s\"\n"
                "port:\"%s\"\nip_next:\"%s\"\nport_next:\"%s\"\ncontent:\"%s\"\n", r, ip, port_str,
                ip_next, port_next, content);
        return 1;
    }
    // retransmit message
    int port = atoi(port_str);
    int i;
    verbose("looking for correspondance with current entity...\n");
    for (i = 0; i < nring + 1; ++i)
        if (strcmp(ip, ent.ip_next[i]) == 0 && port == ent.port_next[i]) {
            verbose("Preparing structure for receiver address...\n");
            struct in_addr iaddr;
            char *ipnz = ipnozeros(ip);
#ifdef DEBUG
            if (inet_aton(ipnz, &iaddr) == 0) {
                debug("insertionsrv()", "inet_aton failed with ip \"%s\".", ipnz);
                continue;
            }
#else
            inet_aton(ipnz, &iaddr);
#endif
            free(ipnz);

            int port2 = atoi(port_next);
            struct sockaddr_in entity_leaving = _ent.receiver[i];

            _ent.receiver[i].sin_family = AF_INET;
            _ent.receiver[i].sin_port = htons(port2);
            _ent.receiver[i].sin_addr = iaddr;
            verbose("Structure prepared.\n");
            verbose("Replacing current structure...\n");
            // modifying entity
            verbose("Insertion server: modifying current entity...\n");
            verbose("Insertion server: current entity :\n%s\n", entitytostr(i));
            ent.port_next[i] = port2;
            strcpy(ent.ip_next[i], ip_next);
            verbose("Insertion server: modified entity :\n%s\n", entitytostr(i));
            sendmessage(&entity_leaving, "EYBG", "");
        }
    return 0;
}

static int action_eybg(char *content) {

    if (wait_goodbye) {
        close_tcpserver();
        close_messagemanager();
        printf("Goodbye !\n");
        exit(0);
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

/***
 * Parse a packet and call the appropriate function.
 * 
 * @param the packet message
 * @return returned value of the function called
 * @return -1 if the message has already been seen or is not supported
 */
int parsemsg(char *message) {
    if (message[4] != ' ') {
        fprintf(stderr, "Message not following the protocol.\n");
        return -1;
    }
    message[4]  = 0;
    message[13] = 0;
    char *type = message;
    char *idm  = strdup(message+5);
    char *content = message+14;
    verbose("Parsing message %s of type %s...\n", idm, type);
    if (lookup(idm)) {
        verbose("Message already seen.\n");
        return -1;
    }
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



void sendmessage_all(char *type, char *format, ...) {
    char buff[513];
    char content[499];
    va_list aptr;
    va_start(aptr, format);
    vsnprintf(content, 499, format, aptr);
    va_end(aptr);
    // creating new id and adding it to list of known ids
    char *id = messageid(content);
#ifdef DEBUG
    int r =
#endif
        lookup(id);
#ifdef DEBUG
    if (r==1) debug("sendmessage", "Detected a hash collision: %s\n", id);
#endif
    if (strlen(content))
      snprintf(buff, 513, "%s %s %s", type, id, content);
    else snprintf(buff, 513, "%s %s", type, id);
    
    sendpacket_all(buff);
}


void sendmessage(struct sockaddr_in *receiver, char *type, char *format, ...) {
    char buff[513];
    char content[499];
    va_list aptr;
    va_start(aptr, format);
    vsnprintf(content, 499, format, aptr);
    va_end(aptr);
    // creating new id and adding it to list of known ids
    char *id = messageid(content);
#ifdef DEBUG
    int r =
#endif
        lookup(id);
#ifdef DEBUG
    if (r==1) debug("sendmessage", "Detected a hash collision: %s\n", id);
#endif
    if (strlen(content))
      snprintf(buff, 513, "%s %s %s", type, id, content);
    else snprintf(buff, 513, "%s %s", type, id);
    
    sendpacket(buff, receiver);
}

