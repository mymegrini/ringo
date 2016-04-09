#include "protocol.h"

#include "common.h"
#include "listmsg.h"
#include "network.h"
#include "message.h"
#include "thread.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>


////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////////
int parseappmsg(char *message);
void *message_manager(void *args);

////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

/**
 * Current entity
 */
entity ent;

_entity _ent;

typedef struct welc_msg {
    char     ip[16];
    uint16_t port;
    char     ip_diff[16];
    uint16_t port_diff;
} welc_msg;

typedef struct newc_msg {
    char ip[16];
    int port;
} newc_msg;
 
typedef welc_msg dupl_msg;


volatile int nring = -1;

char ring_check[NRING];

#define TIMEOUT 10
unsigned timeout = TIMEOUT;



////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////



static welc_msg *parse_welc(const char *w_msg) {
    if (w_msg[4] != ' ' || w_msg[20] != ' ' || w_msg[25] != ' ' ||
            w_msg[41] != ' ' || w_msg[46] != 0) {
        return NULL;
    }
    welc_msg *welc = malloc(sizeof(welc_msg));
    char type[5], port[5], port_mdiff[5];
    int r = sscanf(w_msg, "%s %s %s %s %s", type, welc->ip, port, 
            welc->ip_diff, port_mdiff);
    if (r != 5 || 
            !isip(welc->ip) || !isport(port) || !isip(welc->ip_diff) ||
                !isport(port_mdiff)) {
        free(welc);
        return NULL;
    }
    welc->port = atoi(port);
    welc->port_diff = atoi(port_mdiff);
    return welc;
}


static newc_msg *parse_newc(const char *n_msg) {
    if (n_msg[4] != ' ' || n_msg[20] != ' ' || n_msg[25] != 0)
        return NULL;
    newc_msg *newc = malloc(sizeof(newc_msg));
    char type[5];
    char port[5];
    int r = sscanf(n_msg, "%s %s %s", type, newc->ip, port);
    if (r != 3 || !isip(newc->ip) || !isport(port)) {
        free(newc);
        return NULL;
    }
    newc->port = atoi(port);
    return newc;
}


static dupl_msg *parse_dupl(const char *d_msg) {
    return parse_welc(d_msg);
}


/**
 * Prepare WELC message for insertion protocol
 *
 * @return WELC message
 */
static char *prepare_welc() {
    char *msg = (char *)malloc(50);
    char port_next[5], mdiff_port[5];
    itoa4(port_next, ent.port_next[nring]);
    itoa4(mdiff_port, ent.mdiff_port[nring]);
    sprintf(msg, "WELC %s %s %s %s\n",
            ent.ip_next[nring], port_next,
            ent.mdiff_ip[nring], mdiff_port);
    return msg;
}



/**
 * Prepare NEWC message for insertion protocol
 *
 * @return NEWC message
 */
static char *prepare_newc() {
    char *msg = (char *)malloc(30);
    char udp[5];
    itoa4(udp, ent.udp);
    sprintf(msg, "NEWC %s %s\n", ent.ip_self, udp);
    return msg;
}

/**
 * Prepare WELC message for insertion protocol
 *
 * @return WELC message
 */
static char *prepare_dupl() {
    char *msg = (char *)malloc(50);
    char port[5], mdiff_port[5];
    itoa4(port, ent.udp);
    itoa4(mdiff_port, ent.mdiff_port[nring]);
    sprintf(msg, "DUPL %s %s %s %s\n",
            ent.ip_self, port,
            ent.mdiff_ip[nring], mdiff_port);
    return msg;
}


static void insert(int ring, char *n_msg, int sock2)
{
    verbose("Insertion server: parsing NEWC message...\n");
    newc_msg *newc = parse_newc(n_msg);
    if (newc == NULL) {
        fprintf(stderr, "Protocol error: bad response from client.\nInsertion failed.\n");
        free(newc);
        return;
    }
    verbose("Insertion server: NEWC parsing successful.\n");
    // Actualize udp communication
    verbose("Insertion server: actualizing socket informations for next entity...\n");
    // receiver (next entity) socket
    verbose("Preparing structure for receiver address...\n");
    struct sockaddr_in receiver;
    char ipnz[16];
    ipnozeros(ipnz, newc->ip);
#ifdef DEBUG
    if (!
#endif
            getsockaddr_in(&receiver, ipnz, newc->port, 1)
#ifdef DEBUG
            ) {
        debug("insertionsrv", "Can't retrieve sockaddr_in !");
        return;
    }
#endif
#ifndef DEBUG
    ;
#endif
    verbose("Structure prepared.\n");
    // modifying entity
    verbose("Insertion server: modifying current entity...\n");
    verbose("Insertion server: current entity :\n%s\n", entitytostr(ring));
    strcpy(ent.ip_next[ring], newc->ip);
    verbose("Insertion server: modified entity :\n%s\n", entitytostr(ring));
    free(newc);
    // ACKC confirmation sending
    verbose("Insertion server: sending ACKC confirmation message...\n");
    send(sock2, "ACKC\n", 5, 0);
    ent.port_next[ring] = newc->port;
    verbose("Insertion server: message sent.\n");
    _ent.receiver[ring] = receiver;
    verbose("Actualizing receviver...\n");
    verbose("Current structure replaced.\n");
    // closing connection
    /*close(sock2);*/
    verbose("Insertion server: connection closed.\n");
}

static void dupplicate(char *d_msg, int sock2) {
    verbose("Insertion server: parsing DUPL message...\n");
    dupl_msg *dupl = parse_dupl(d_msg);
    if (dupl == NULL) {
        verbose("Protocol error: bad response from client.\nInsertion failed.\n");
        free(dupl);
        return;
    }
    verbose("Insertion server: DUPL parsing successful.\n");
    verbose("Retreiveing struct for communication udp communication...\n");
    char ip[16];
    ipnozeros(ip, dupl->ip);
    if (!getsockaddr_in(&_ent.receiver[nring+1], ip, dupl->port, 1)) {
        verbose("Can't create communication with %s on port %d.\n"
                "Dupplication failed.\n", ip, dupl->port);
        free(dupl);
        return;
    }
    verbose("Struct retreived.\n");
    _ent.sockmdiff[nring+1] = socket(AF_INET, SOCK_DGRAM, 0);
    ipnozeros(ip, dupl->ip_diff);
    verbose("Subscribing to multicast channel ip %s on port %d.\n", 
            ip, dupl->port_diff);
    if (!multicast_subscribe(_ent.sockmdiff[nring+1], dupl->port_diff, ip)) {
        verbose("Can't subscribe to channel ip %s on port %d.\n"
                "Dupplication failed.\n", ip, dupl->port_diff);
        close(_ent.sockmdiff[nring+1]);
        free(dupl);
        return;
    }
    verbose("Sending confirmation message with listening port...\n");
    char msg[11];
    char port[5];
    itoa4(port, ent.udp);
    sprintf(msg, "ACKC %s\n", port);
    send(sock2, msg, 10, 0);
    verbose("Confirmation message sent.\n");

    verbose("Modifying entity...\n");
    ent.port_next[nring+1] = dupl->port;
    strcpy(ent.ip_next[nring+1], dupl->ip);
    ent.mdiff_port[nring+1] = dupl->port_diff;
    strcpy(ent.mdiff_ip[nring+1], dupl->ip_diff);
    verbose("Entity modified.\n");
    verbose("Actualizing ring number...\n");
    actualize_nring(nring+1);
    verbose("Ring number actualized. Number of rings: %d.\n", nring+1);
    verbose("Dupplication finished.\n");
}
/**
 * Server waiting for new entity insertions
 */
static void insertionsrv() {
    verbose("Starting insertion server...\n");
    // socket preparation
    int sock = socket(PF_INET,SOCK_STREAM, 0);
    struct sockaddr_in addr_sock;
    // server socket filling
    addr_sock.sin_family      = AF_INET;
    addr_sock.sin_port        = htons(ent.tcp);
    addr_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    // port binding
    int res;
    res = bind(sock, (struct sockaddr *)&addr_sock, 
            sizeof(struct sockaddr_in));
    if ( res == -1 ) {
        close(sock);
        perror("Binding error.");
        exit(1);
    }
    // listening sock
    res = listen(sock, 0);
    if ( res == -1 ) {
        close(sock);
        perror("Listen error.");
        exit(1);
    }
    _ent.socktcp = sock;
    while (1) {
        // wait for connection
        verbose("Insertion server: waiting for client...\n");
        struct sockaddr_in caller;
        socklen_t size = sizeof(caller);
        int sock2;
        sock2 = accept(sock, (struct sockaddr *)&caller, &size);
        // error from accept
        if ( sock2 == -1 ) {
            perror("Error accept.");
            continue;
        }
        verbose("Insertion server: connection established.\n");
        // insertion protocol
        // WELC message sending
        verbose("Insertion server: preparing WELC message...\n");
        char *msg = prepare_welc();
        verbose("Insertion server: sending \"%s\"...\n", msg);
        send(sock2, msg, strlen(msg), 0);
        verbose("Insertion server: message sent.\n");
        free(msg);
        // NEWC message reception
        verbose("Insertion server: waiting for NEWC message...\n");
        msg = receptLine(sock2);
        verbose("Insertion server: received : \"%s\".\n", msg);
        if (strncmp(msg, "NEWC", 4) == 0)
            insert(nring, msg, sock2);
        else if (strncmp(msg, "DUPL", 4) == 0)
            dupplicate(msg, sock2);
        else
            verbose("Message not supported: \"%s\".\n", msg);
        close(sock2);
        free(msg);
    }
}



static void *packet_treatment(void *args) {
    char *packet = (char *)args;
    packet[512] = 0;
    parsemsg(packet);
    free(packet);
    return NULL;
}

static void test_ring() {
    // initialize ring_check array
    /*pthread_mutex_lock(&mutexes.nring);*/
    memset(ring_check, -1, NRING);
    char port_diff[5];
    // send test messages in each rings
    int fixed_nring = getnring();
    for (int i = 0; i < fixed_nring + 1; i++) {
        ring_check[i] = 0;
        itoa4(port_diff, ent.mdiff_port[i]);
        sendmessage(i, "TEST", "%s %s", ent.mdiff_ip[i], port_diff);
    }
    debug("test_ring", GREEN "timeout beginning...");
    sleep(timeout);
    debug("test_ring", GREEN "end of timeout.");

    for (int i = 0; i < fixed_nring + 1 && ring_check[i] != -1; i++) {
        if (ring_check[i]) {
            debug("test_ring", GREEN "ring %d: checked.", i);
            continue;
        }
        else {
            debug("test_ring", GREEN "ring %d: checking failed. Ring broken.", i);
            continue;
        }
    }
    /*pthread_mutex_lock(&mutexes.nring);*/
    
}


////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

/**
 * Return a string representing current entity.
 *
 * @return the string representing current entity
 */
char *entitytostr(int ring) {
    char *str = (char *)malloc(350);
    char *id = malloc(50), *udp = malloc(50), *tcp = malloc(50),
         *ip = malloc(50), *np = malloc(50), *mdip = malloc(50), 
         *mdp = malloc(50);
    sprintf(id, "%30s - %s", "\x1b[4mid\x1b[0m", ent.id);
    sprintf(udp, "%30s - %d", "\x1b[4mudp listening port\x1b[0m", ent.udp);
    sprintf(tcp, "%30s - %d", "\x1b[4mtcp listening port\x1b[0m", ent.tcp);
    sprintf(ip, "%30s - %s", "\x1b[4mip of next entity\x1b[0m", ent.ip_next[ring]);
    sprintf(np, "%30s - %d", "\x1b[4mport of next entity\x1b[0m", ent.port_next[ring]);
    sprintf(mdip, "%30s - %s", "\x1b[4mmultidiff ip\x1b[0m", ent.mdiff_ip[ring]);
    sprintf(mdp, "%30s - %d", "\x1b[4mmultidiff port\x1b[0m", ent.mdiff_port[ring]);
    sprintf(str, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n", id, udp, tcp, ip, np, mdip, mdp);
    free(id);free(udp);free(tcp);free(ip);free(np);free(mdip);free(mdp);
    return str;
}

/**
 * Protocol for insertion of current entity into a ring.
 *
 * @param hostname of the entity on the ring
 * @param port of the entity on the ring
 * @return 1 if insertion succed, 0 else
 */
int join(const char *host, const char *tcpport) {
    // preparing the structure
    struct sockaddr_in addr;
    if (!getsockaddr_in(&addr, host, atoi(tcpport), 0)) {
        verbose("Can't get address of %s at port %s.\n", host, tcpport);
        return 0;
    }
    // socket creation
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr *)&addr,
                (socklen_t)sizeof(struct sockaddr_in)) != 0)
    {
        close(sock);
        fprintf(stderr,
                "Can't establish connection with %s on port %s.\n", host, tcpport);
        return 0;
    }
    verbose("Connection established with %s on port %s.\n", host, tcpport);
    // WELC message reception
    verbose("waitig for WELC message...\n");
    char *msg = receptLine(sock);
    verbose("Message received : \"%s\".\n", msg);
    verbose("Parsing message...\n");
    welc_msg *welc = parse_welc(msg);
    free(msg);
    if (welc == NULL) {
        fprintf(stderr, "Protocol error: bad response.\nInsertion failed.\n");
        free(welc);
        return 0;
    }
    verbose("Parsing successfull.\n");
    // NEWC message sending
    verbose("Preparing NEWC message...\n");
    char *newc_str = prepare_newc();
    verbose("Sending: \"%s\".\n", newc_str);
    send(sock, newc_str, strlen(newc_str), 0);
    verbose("Message sent.\n");
    fflush(stdout);
    // ACKC message reception
    verbose("Waiting for ACKC confirmation message...\n");
    msg = receptLine(sock);
    verbose("Message received: \"%s\".\n", msg);
    if (strcmp(msg, "ACKC") != 0) {
        fprintf(stderr, "Protocol error: bad response.\nInsertion failed.\n");
        free(welc);
        free(msg);
        return 0;
    }
    ++nring;
    verbose("Modifying current entity...");
    verbose("Current entity:\n%s\n", entitytostr(nring));
    ent.port_next[nring] = welc->port;
    ent.mdiff_port[nring] = welc->port_diff;
    strcpy(ent.ip_next[nring], welc->ip);
    strcpy(ent.mdiff_ip[nring], welc->ip_diff);
    verbose("Modified entity:\n%s\n", entitytostr(nring));
    // Socket creation
    verbose("Creating sockets for UDP communication...\n");
    _ent.socklisten = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp listening created.\n");
    verbose("Binding socket for listening...\n");
    if (!bind_udplisten(_ent.socklisten, ent.udp)) {
        fprintf(stderr, "Binding error, insertion failed.\n");
        return 0;
    }
    verbose("Binding done.\n");
    _ent.socksend = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp sending created.\n");
    verbose("Preparing structure for receiver address...\n");
    // receiver (next entity) socket
    char ipnz[16];
    ipnozeros(ipnz, ent.ip_next[nring]);
    if (!getsockaddr_in(&_ent.receiver[nring], ipnz,
                ent.port_next[nring], 1)) {
        verbose("Can't communicate with address %s on port %d.\n",
                ent.ip_next[nring], ent.port_next[nring]);
        return 0;
    }
    // multi diff
    verbose("Subscribing to channel %s...\n", ent.mdiff_ip[nring]);
    _ent.sockmdiff[nring] = socket(AF_INET, SOCK_DGRAM, 0);
    char mdiff_ip[16];
    ipnozeros(mdiff_ip, ent.mdiff_ip[nring]);
    if (!multicast_subscribe(_ent.sockmdiff[nring], ent.mdiff_port[nring],
               mdiff_ip)) {
        fprintf(stderr, 
                "can't subscribe to multicast channel ip %s on port %d\n",
                mdiff_ip, ent.mdiff_port[nring]);
        return 0;
    }
    verbose("Entity subscribed to channel.\n");
    verbose("Structure prepared.\n");
    verbose("Socket for UDP communication prepared.\n");
    verbose("Insertion done.\n");

    init_threads();
    return 1;
}


/**
 * Protocol for insertion of current entity into a ring.
 *
 * @param hostname of the entity on the ring
 * @param port of the entity on the ring
 * @return 1 if insertion succed, 0 else
 */
int dupplicate_rqst(const char *host, const char *tcpport) {
    ++nring;
    // preparing the structure
    struct sockaddr_in addr;
    if (!getsockaddr_in(&addr, host, atoi(tcpport), 0)) {
        verbose("Can't get address of %s at port %s.\n", host, tcpport);
        return 0;
    }
    // socket creation
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr *)&addr,
                (socklen_t)sizeof(struct sockaddr_in)) != 0)
    {
        close(sock);
        fprintf(stderr,
                "Can't establish connection with %s on port %s.\n", host, tcpport);
        return 0;
    }
    verbose("Connection established with %s on port %s.\n", host, tcpport);
    verbose("Creating sockets for UDP listening...\n");
    _ent.socklisten = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp listening created.\n");
    verbose("Binding socket for listening...\n");
    if (!bind_udplisten(_ent.socklisten, ent.udp)) {
        fprintf(stderr, "Binding error, insertion failed.\n");
        return 0;
    }
    verbose("Binding done.\n");
    // set up ip_next directly so only port will be missing
    char *ip_next = inet_ntoa(addr.sin_addr);
    char *ipsized = ipresize(ip_next);
    strcpy(ent.ip_next[nring], ipsized);
    free(ipsized);
    // WELC message reception
    verbose("waitig for WELC message...\n");
    char *msg = receptLine(sock);
    verbose("Message received : \"%s\".\n", msg);
    verbose("Parsing message...\n");
    welc_msg *welc = parse_welc(msg);
    free(msg);
    if (welc == NULL) {
        fprintf(stderr, "Protocol error: bad response.\nInsertion failed.\n");
        free(welc);
        return 0;
    }
    verbose("Parsing successfull.\n");
    // NEWC message sending
    verbose("Preparing DUPL message...\n");
    char *dupl_str = prepare_dupl();
    verbose("Sending: \"%s\".\n", dupl_str);
    send(sock, dupl_str, strlen(dupl_str), 0);
    verbose("Message sent.\n");
    fflush(stdout);
    // ACKC message reception
    verbose("Waiting for ACKC confirmation message...\n");
    msg = receptLine(sock);
    verbose("Message received: \"%s\".\n", msg);
    if (strncmp(msg, "ACKC ", 5) != 0 || strlen(msg) != 9) {
        fprintf(stderr, "Protocol error: bad response from server.\n"
                "Insertion failed.\n");
        free(welc);
        free(msg);
        return 0;
    }
    if (!isport(&msg[5])) {
        fprintf(stderr, "Protocol error: bad response from server.\n"
                "Needed port, found \"%s\".\n", &msg[5]);
        return 0;
    }
    verbose("Modifying current entity...");
    verbose("Current entity:\n%s\n", entitytostr(nring));
    ent.port_next[nring] = atoi(&msg[5]);
    verbose("Modified entity:\n%s\n", entitytostr(nring));
    // Socket creation
    _ent.socksend = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp sending created.\n");
    verbose("Preparing structure for receiver address...\n");
    // receiver (next entity) socket
    if (!getsockaddr_in(&_ent.receiver[nring], ent.ip_next[nring],
                ent.port_next[nring], 1)) {
        verbose("Can't communicate with address %s on port %d.\n",
                ent.ip_next[nring], ent.port_next[nring]);
        return 0;
    }
    // multi diff
    verbose("Subscribing to channel %s...\n", ent.mdiff_ip[nring]);
    _ent.sockmdiff[nring] = socket(AF_INET, SOCK_DGRAM, 0);
    char mdiff_ip[16];
    ipnozeros(mdiff_ip, ent.mdiff_ip[nring]);
    if (!multicast_subscribe(_ent.sockmdiff[nring], ent.mdiff_port[nring],
               mdiff_ip)) {
        fprintf(stderr, 
                "can't subscribe to multicast channel ip %s on port %d\n",
                mdiff_ip, ent.mdiff_port[nring]);
        return 0;
    }
    verbose("Entity subscribed to channel.\n");
    verbose("Structure prepared.\n");
    verbose("Socket for UDP communication prepared.\n");
    verbose("Insertion done.\n");

    init_threads();
    return 1;
}


/**
 * Mapping of function insertionsrv to fit the thread signature
 */
void *insertion_server(void *arg) {
    insertionsrv();
    return NULL;
}



/**
 * Message manager thread
 *
 * Process messages received from the ring.
 */
void *message_manager(void *args) {
    char buff[513];
    verbose(UNDERLINED "Message manager launched.\n" RESET);
    while (1) {
        int rec = recv(_ent.socklisten, buff, 512, 0);
#ifdef DEBUG
        if (rec > 0) {
            buff[rec] = 0;
            debug("message_manager", YELLOW "Packet received:\n---\n%s\n---\n", buff);
        }
#endif
        if (rec == 512) {
            verbose("Packet received.\n");
            char *packet = strndup(buff, 512);
            pthread_t t_packet_treat;
            pthread_create(&t_packet_treat, NULL, packet_treatment, (void*) packet);
        }

    }
    return NULL;
}



void sendpacket(char *content, int ring) {
    debug("sendpacket(char *content, int ring)",
            "Sending packet solo ring:\n---\n%s\n---\n...\n"
            "To ip %s on port %d.", content, inet_ntoa(_ent.receiver[ring].sin_addr),
            ntohs(_ent.receiver[ring].sin_port));
    /*pthread_mutex_lock(&mutexes.receiver[ring]);*/
    sendto(_ent.socksend, content, 512, 0,
            (struct sockaddr *) &_ent.receiver[ring],
            (socklen_t)sizeof(struct sockaddr_in));
    /*pthread_mutex_unlock(&mutexes.receiver[ring]);*/
    verbose("Packet sent.\n");
}

void sendpacket_all(char *content) {
    debug("sendpacket_all(char *content)", 
            "Sending packet multiple ring:\n---\n%s\n---\n...\n", content);
    for (int i = 0; i < nring + 1; ++i) {
        /*pthread_mutex_lock(&mutexes.receiver[i]);*/
        sendto(_ent.socksend, content, 512, 0,
                (struct sockaddr *)&_ent.receiver[i],
                (socklen_t)sizeof(struct sockaddr_in));
        /*pthread_mutex_unlock(&mutexes.receiver[i]);*/
    }
    verbose("Packets sent.\n");
}


void sendpacket_sockaddr(char *content, struct sockaddr_in *receiver) {
    debug("sendpacket_sockaddr(char *content, struct sockaddr_in *receiver)", 
            "Sending packet solo:\n---\n%s\n---\n...\n"
            "To ip %s on port %d.", content, inet_ntoa(receiver->sin_addr),
            ntohs(receiver->sin_port));
    sendto(_ent.socksend, content, 512, 0,
            (struct sockaddr *) receiver,
            (socklen_t)sizeof(struct sockaddr_in));
    verbose("Packet sent.\n");
}
/**
 * Initialize entity with given attributes.
 * ip_next and port_next are set to ip_self and udp_listen.
 */
void init_entity(char *id, uint16_t udp_listen, uint16_t tcp_listen,
        char *mdiff_ip, uint16_t mdiff_port) {
    // id
    strncpy(ent.id, id, 8);
    // ip_self
    char *ip = getLocalIp();
    strcpy(ent.ip_self, ip);
    free(ip);
    // udp_listen
    ent.udp = udp_listen;
    // tcp_listen
    ent.tcp = tcp_listen;
    // ip_next[0]
    strcpy(ent.ip_next[0], ent.ip_self);
    // port_next init
    ent.port_next[0] = udp_listen;
    // mdiff_ip
    if (mdiff_ip) {
        ip = ipresize(mdiff_ip);
        strcpy(ent.mdiff_ip[0], ip);
        // mdiff port
        ent.mdiff_port[0] = mdiff_port;
    }

    debug("init_entity", "%s\n", entitytostr(0));
}



/**
 * Create a ring.
 *
 * It consists of creating the sockets for sending message and listening, then
 * launching the message manager and the insertion server.
 */
void create_ring() {
    ++nring;
    // Socket creation
    verbose("Creating sockets for UDP communication...\n");
    // listening socket
    _ent.socklisten = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp listening created.\n");
    verbose("Binding socket for listening...\n");
    if (!bind_udplisten(_ent.socklisten, ent.udp)) {
        fprintf(stderr, "Binding error. Ring creation failed.\n");
        exit(1);
    }
    verbose("Binding done.\n");
    _ent.socksend  = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp sending created.\n");
    // receiver (next entity) socket
    verbose("Preparing structure for receiver address...\n");
    if (!getsockaddr_in(&_ent.receiver[nring], "localhost", ent.port_next[nring],
                0)) {
        fprintf(stderr, "Can't access to localhost on port %d.\n"
                "Ring creation failed.\n", ent.port_next[nring]);
    }
    verbose("Sockets created.\n");

    // multidiffusion
    _ent.sockmdiff[nring] = socket(PF_INET, SOCK_DGRAM, 0);
    // authorize multidiff on same machine
    verbose("Subscibing to multicast channel %s on port %d...\n", 
            ent.mdiff_ip[nring], ent.mdiff_port[nring]);
    if (!multicast_subscribe(_ent.sockmdiff[nring], ent.mdiff_port[nring],
                ent.mdiff_ip[nring])) {
        fprintf(stderr, "Can't subscribe to channel ip %s on port %d.\n"
                "Ring creation failed.\n", ent.mdiff_ip[nring],
                ent.mdiff_port[nring]);
        exit(1);
    }
    verbose("Subscribed to multicast channel.\n");

    init_threads();

    verbose("Ring created.\n");

}


void *ring_tester(void *args) {
    unsigned interval = 10;
    while (1) {
        sleep(interval);
        test_ring();
    }
}


void rm_ring(int ring) {

}
