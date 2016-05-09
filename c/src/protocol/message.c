#include "message.h"
#include "common.h"
#include "protocol.h"
#include "stdint.h"
#include "listmsg.h"
#include "thread.h"
#include "network.h"

#include <stdarg.h>
#include <sys/time.h>

////////////////////////////////////////////////////////////////////////////////
// EXTERNAL
////////////////////////////////////////////////////////////////////////////////
extern int parseappmsg(const char *message, const char *content, int lookup_flag);
static int action_whos(const char *message, const char *content, int lookup_flag);
static int action_memb(const char *message, const char *content, int lookup_flag);
static int action_test(const char *message, const char *content, int lookup_flag);

extern int action_gbye(const char *message, const char *content, int lookup_flag);
extern int action_eybg(const char *message, const char *content, int lookup_flag);
////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

typedef struct protocol_msg {
    char type[5];
    int (*action)(const char *, const char *, int lookup_flag);
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


/* extern entity *ent; */
extern volatile int nring;
extern _entity *_ent;
extern volatile int ring_check[NRING];
extern short volatile *rc;
extern volatile struct test_data* test_data;

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
    h = h * 33 + (uint16_t)time.tv_sec;
    h = h * 33 + (uint16_t)time.tv_usec;
    // hashing ip and port
    for(i=0; i<16; i++) h = h * 33 + ent->ip_self[i];  
    h = h * 33 + ent->udp;
    // hashing content
    // while((c = *content++)) h = h * 33 + c;

    // creating hash using alphanumerical characters
    for(i=0; i<8; i++){
        c = h % 62;
        if (c<10) hash[i] = c+48;      //digits
        else if (c<36) hash[i] = c-10+97; //lowercase letters
        else if (c<62) hash[i] = c-36+65; //uppercase letters
        else hash[i] = 0;
    
        h = h / 62;
    }
    hash[8] = 0;

    /*debug("messageid", "Created message id : %s.\n", hash);*/
}


static int action_whos(const char *message, const char *content, int lookup_flag) {
    // message already seen
    if (lookup_flag) {
      verbose(UNDERLINED "WHOS:" RESET " message already seen, nothing to do.\n");
        return 0;
    }
    if (strlen(message) > 13) {
        debug("action_whos", "content should be empty: \"%s\" = %d",
              content, *content);
        return 1;
    } else {
        sendpacket_all(message);
        sendmessage_all("MEMB", "%s %s %d", ent->id, ent->ip_self, ent->udp);
    }
    return 0;
}


static int action_memb(const char* message, const char* content, int lookup_flag) {
    // message already seen
    if (lookup_flag)
        return 0;
    sendpacket_all(message);
    return 0;
}

static int action_test(const char *message, const char *content, int lookup_flag) {
    debug("action_test", RED "entering function...");
    if (content[15] != ' ') {
        debug("action_test", RED "content not following the protocol."\
                "content: \"%s\"", content);
        return 1;
    }
    if (lookup_flag) {
        char mdiff_port[5];
        for (int i = 0; i < test_data->nring; ++i) {
            itoa4(mdiff_port, ent->mdiff_port[i]);
            // find ring associated with message and actualize the checking
            if (strncmp(content, ent->mdiff_ip[i], 15) == 0 &&
                    strncmp(&content[16], mdiff_port, 4) == 0) {
                test_data->ring_check[i] = 1;
                if (--test_data->count == 0)
                    pthread_cond_signal(&mutex->test.c);
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
        fprintf(stderr, RED "Message not following the protocol.\n");
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
    int lookup_flag = lookup(idm);
    debug("parsemsg", "message %s, lookup: %d", type, lookup_flag);
    // search action to do
    for (int i = 0; pmsg[i].type[0] != 0; i++)
        if (strcmp(type, pmsg[i].type) == 0) {
          verbose("Executing action %s.\n", type);
          return (pmsg[i].action(message, content, lookup_flag));
        }
    // message not supported
    verbose("Message of type %s not supported.\n", type);
    if (!lookup_flag)
      sendpacket_all(message);
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


void sendmessage_all(const char *type, const char *format, ...) {
    char buff[513];
    va_list aptr;

    va_start(aptr, format);
    makemessage(buff, type, format, aptr);
    va_end(aptr);

    sendpacket_all(buff);
}


void sendmessage(int ring, const char *type, const char *format, ...) {
    char buff[513];
    va_list aptr;

    va_start(aptr, format);
    makemessage(buff, type, format, aptr);
    va_end(aptr);
    
    /*sendpacket(buff, ring);*/
    sendpacket_sockaddr(buff, &_ent->receiver[ring]);
    debug("sendmessag(int ring, type, format, ...)", BLUE "message sent->");
}

void sendmessage_sockaddr(const struct sockaddr_in *receiver, const char *type, const char *format, ...) {
    char buff[513];
    va_list aptr;

    va_start(aptr, format);
    makemessage(buff, type, format, aptr);
    va_end(aptr);
    
    sendpacket_sockaddr(buff, receiver);
}
