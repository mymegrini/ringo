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
extern int parseappmsg(char *message, char *content, ...);
static int action_whos(char *message, char *content, ...);
static int action_gbye(char *message, char *content, ...);
static int action_eybg(char *message, char *content, ...);
static int action_memb(char *message, char *content, ...);
static int action_test(char *message, char *content, ...);

////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

typedef struct protocol_msg {
    char type[5];
    int (*action)(char *, char *, ...);
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
    { "MEMB", action_memb },
    { "TEST", action_test },
    { "", NULL }
};


extern entity ent;
extern int nring;
extern _entity _ent;
int wait_goodbye = 0;
extern int ring_check[NRING];

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////


/**
 * Generate a unique message identificator
 *
 * @param content message content to be included in hash
 * @param hash will contain message hash id
 * @return message idenfitificator, a char* of strlen 8
 */
static void messageid(char* hash) {

    struct timeval time;
    uint64_t h = 5381;
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
    // while((c = *content++)) h = ((h << 5) + h) + c;

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
}


static int action_whos(char *message, char *content, ...) {

    if (strlen(message) > 13) {
        debug("action_whos", "content should be empty: \"%s\" = %d",
              content, *content);
        return 1;
    } else {
        sendpacket_all(message);
        sendmessage_all("MEMB", "%s %s %d", ent.id, ent.ip_self, ent.udp);
    }
    return 0;
}


static int action_gbye(char *message, char *content, ...) {

    // don't retransmit message
    if (content[15] != ' ' || content[20] != ' ' || content[36] != ' ') {
        return 1;
    }
    char ip[16], port_str[5], ip_next[16], port_next[5];
    int r = sscanf(content, "%s %s %s %s", ip, port_str, ip_next, port_next);
    if (r != 4 ||
        !isip(ip) || !isport(port_str) || !isip(ip_next) || !isport(port_next)) {
        debug("action_gbye", "GBYE not following the protocol, content: \"%s\"", content);
        return 1;
    }
    // retransmit message
    sendpacket_all(message);
    int port = atoi(port_str);
    int i;
    verbose("looking for correspondance with current entity...\n");
    for (i = 0; i < nring + 1; ++i)
        if (strcmp(ip, ent.ip_next[i]) == 0 && port == ent.port_next[i]) {
            verbose("Preparing structure for receiver address...\n");
            struct in_addr iaddr;
            char ipnz[16];
            ipnozeros(ipnz, ip);
#ifdef DEBUG
            if (inet_aton(ipnz, &iaddr) == 0) {
                debug("insertionsrv()", "inet_aton failed with ip \"%s\".", ipnz);
                continue;
            }
#else
            inet_aton(ipnz, &iaddr);
#endif

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

static int action_eybg(char *message, char *content, ...) {

    if (wait_goodbye) {
        close_tcpserver();
        close_messagemanager();
        printf("Goodbye !\n");
        exit(0);
    }
    else {
        debug("action_eybg", "entity received EYBG whereas not waiting for it.");
        sendpacket_all(message);
    }
    return 0;
}

static int action_memb(char* message, char* content, ...) {

    sendpacket_all(message);
    return 0;
}

static int action_test(char *message, char *content, ...) {
    if (content[15] != ' ' || content[20] != 0) {
        debug("action_test", "content not following the protocol."\
                "content: \"%s\"", content);
        return 1;
    }
    va_list args;
    va_start(args, content);
    char *lookup_flag = va_arg(args, char*);
    va_end(args);

    if (lookup_flag) {
        char mdiff_port[5];
        for (int i = 0; i < nring + 1; ++i) {
            itoa4(mdiff_port, ent.mdiff_port[i]);
            // find ring associated with message and actualize the checking
            if (strncmp(content, ent.mdiff_ip[i], 15) == 0 &&
                    strncmp(&content[16], mdiff_port, 4) == 0) {
                ring_check[i] = 1;
                return 0;
            }
        }
    }
    else {
        sendpacket_all(message);
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
    //message[4]  = 0;
    //message[13] = 0;
    char type[5];
    strncpy(type, message, 4);
    type[4] = 0;
    char idm[9];
    strncpy(idm, message+5, 8);
    idm[8] = 0;
    char *content = message+14;

    verbose("Parsing message %s of type %s...\n", idm, type);
    if (lookup(idm)) {
        verbose("Message already seen.\n");
        if (strcmp(type, "TEST") == 0)
            return action_test(message, content);
        else
            return 0;
    }
    // search action to do
    for (int i = 0; pmsg[i].type[0] != 0; i++)
        if (strcmp(type, pmsg[i].type) == 0) {
            return (pmsg[i].action(message, content));
        }
    // message not supported
    verbose("Message of type %s not supported.\n", type);
    // remove message from the list because not supported
    //lookup(idm);
    return -1;
}

static void makemessage(char* buff, const char* type,
        const char* format, va_list aptr) {

    char content[499];

    // creating message content
    vsnprintf(content, 499, format, aptr);
    
    // creating new id
    char id[9];
    messageid(id);

    // adding id to list of known ids, and checking for collisions
#ifdef DEBUG
    int r =
#endif
        lookup(id);
#ifdef DEBUG
    if (r==1) debug("makemessage", "Detected a hash collision: %s\n", id);
#endif

    // creating message
    if (strlen(content))
      snprintf(buff, 513, "%s %s %s", type, id, content);
    else snprintf(buff, 513, "%s %s", type, id);

    return;
}


void sendmessage_all(char *type, char *format, ...) {
    char buff[513];
    va_list aptr;

    va_start(aptr, format);
    makemessage(buff, type, format, aptr);
    va_end(aptr);

    sendpacket_all(buff);
}


void sendmessage(struct sockaddr_in *receiver, char *type, char *format, ...) {
    char buff[513];
    va_list aptr;

    va_start(aptr, format);
    makemessage(buff, type, format, aptr);
    va_end(aptr);
    
    sendpacket(buff, receiver);
}

