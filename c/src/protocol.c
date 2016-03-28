#include "protocol.h"

#include "common.h"
#include "listmsg.h"
#include "network.h"
#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

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
// TODO replace current entity by an array.
entity ent;

typedef struct _entity {
    int socksend[NRING];
    struct sockaddr_in receiver[NRING];
    int socklisten;
} _entity;

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
 


int nring = -1;

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

/**
 * Return a string representing current entity.
 *
 * @return the string representing current entity
 */
static char *entitytostr(int ring) {
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
 * Parse a NEWC message
 * 
 * @param NEWC message
 * @return a newc_msg *, or NULL when message doesn't follow protocol
 */
static welc_msg *parse_welc(const char *w_msg) {
    char *msg = strdup(w_msg);
    debug("parse_welc(const char *w_msg)", "w_msg : \"%s\"\n", w_msg);
#ifdef DEBUG
#define parse_test(test, format, ...) \
    if (test) { \
        debug("parse_welc(\"%s\")", #test " -> " format, msg, ##__VA_ARGS__); \
        free(welc); \
        free(msg); \
        return NULL; \
    }
#else
#define parse_test(test, format, ...) \
    if (test) { \
        free(welc); \
        free(msg); \
        return NULL; \
    }
#endif
    welc_msg *welc = (welc_msg *)malloc(sizeof(welc_msg));
    // token 1: WELC
    char *token = strtok(msg, " ");
    parse_test(token == NULL, 
            "first call to strtok returned NULL, \"WELC\" required.\n");
    parse_test(strcmp(token, "WELC") != 0, 
            "token = \"%s\", \"WELC\" required.\n",
            token);
    // token 2: ip
    token = strtok(NULL, " ");
    parse_test(token == NULL, 
            "second call to strtok returned NULL, ip required.\n");
    parse_test(strlen(token) != 15,
            "ip length must be of 15.\nFound: \"%s\"\nLength: %zu\n", token, strlen(token));
    strncpy(welc->ip, token, 16);
    // token 3: port
    token = strtok(NULL, " ");
    parse_test(token == NULL, 
            "third call to strtok returned NULL, port required.\n");
    parse_test(!isnumeric(token),
            "non int found, port required.\n");
    welc->port = atoi(token);
    // token 4: multidiff ip
    token = strtok(NULL, " ");
    parse_test(token == NULL, 
            "fourth call to strtok returned NULL, multi-diff ip required.\n");
    parse_test(strlen(token) != 15,
            "multi diff ip length must be of 15.\nFound: \"%s\"\nLength: %zu\n", token, strlen(token));
    strncpy(welc->ip_diff, token, 16);
    // token 5: multidiff port
    token = strtok(NULL, " ");
    parse_test(token == NULL, 
            "fifth call to strtok returned NULL, port required.\n");
    parse_test(!isnumeric(token),
            "non int found, multi-diff port required.\n");
    welc->port_diff = atoi(token);
    // test for non valid extra things
    parse_test(strtok(NULL, " ") != NULL,
            "unvalid extra informations found : %s", token);
#undef parse_test
    free(msg);
    return welc;
}


/**
 * Parse a NEWC message
 * 
 * @param NEWC message
 * @return a newc_msg *, or NULL when message doesn't follow protocol
 */
static newc_msg *parse_newc(const char *n_msg) {
    char *msg = strdup(n_msg);
#ifdef DEBUG
#define parse_test(test, format, ...) \
    if (test) { \
        debug("parse_newc(\"%s\")", #test " -> " format, msg, ##__VA_ARGS__); \
        free(newc); \
        free(msg); \
        return NULL; \
    }
#else
#define parse_test(test, format, ...) \
    if (test) { \
        free(newc); \
        free(msg); \
        return NULL; \
    }
#endif
    newc_msg *newc = (newc_msg *)malloc(sizeof(newc_msg));
    // token 1: WELC
    char *token = strtok(msg, " ");
    parse_test(token == NULL, 
            "first call to strtok returned NULL, \"NEWC\" required.\n");
    parse_test(strcmp(token, "NEWC") != 0, 
            "token = \"%s\", \"NEWC\" required.\n",
            token);
    // token 2: ip
    token = strtok(NULL, " ");
    parse_test(token == NULL, 
            "second call to strtok returned NULL, ip required.\n");
    parse_test(strlen(token) != 15,
            "ip length must be of 15.\n");
    strncpy(newc->ip, token, 16);
    // token 3: port
    token = strtok(NULL, " ");
    parse_test(token == NULL, 
            "third call to strtok returned NULL, port required.\n");
    parse_test(!isnumeric(token),
            "non int found, port required.\n");
    newc->port = atoi(token);
    // test for non valid extra things
    parse_test(strtok(NULL, " ") != NULL,
            "unvalid extra informations found : %s", token);
#undef parse_test
    free(msg);
    return newc;
}



/**
 * Prepare WELC message for insertion protocol
 *
 * @return WELC message
 */
static char *prepare_welc() {
    char *msg = (char *)malloc(50);
    sprintf(msg, "WELC %s %s %s %s\n",
            ent.ip_next[nring], itoa4(ent.port_next[nring]), 
            ent.mdiff_ip[nring], itoa4(ent.mdiff_port[nring]));
    return msg;
}



/**
 * Prepare NEWC message for insertion protocol
 *
 * @return NEWC message
 */
static char *prepare_newc() {
    char *msg = (char *)malloc(30);
    sprintf(msg, "NEWC %s %s\n", ent.ip_self, itoa4(ent.udp));
    return msg;
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
        verbose("Insertion server: parsing message...\n");
        newc_msg *newc = parse_newc(msg);
        free(msg);
        if (newc == NULL) {
            fprintf(stderr, "Protocol error: bad response from client.\nInsertion failed.\n");
            free(newc);
            close(sock2);
            continue;
        }
        verbose("Insertion server: parsing successful.\n");
        // Actualize udp communication
        verbose("Insertion server: actualizing socket informations for next entity...\n");
        // receiver (next entity) socket
        verbose("Preparing structure for receiver address...\n");
        char *port = itoa4(newc->port);
        struct addrinfo hints;
        bzero(&hints, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype=SOCK_DGRAM;
        struct addrinfo *first_info;
#ifdef DEBUG
        if (getaddrinfo(newc->ip, port, &hints, &first_info) == -1) {
            debug("create_ring()", 
                    UNDERLINED
                    "getaddrinfo(\"localhost\", port, &hints, &first_info)\n" RESET \
                    "Returned -1 for port \"%s\".",
                    port);
            exit(1);
        }
#else
        getaddrinfo("localhost", port, &hints, &first_info);
#endif
        verbose("Structure prepared.\n");
        verbose("Replacing current structure...\n");
        memcpy(&_ent.receiver[nring], first_info->ai_addr, sizeof(struct sockaddr_in));
        freeaddrinfo(first_info);
        free(port);
        verbose("Current structure replaced.\n");

        // modifying entity
        verbose("Insertion server: modifying current entity...\n");
        verbose("Insertion server: current entity :\n%s\n", entitytostr(nring));
        ent.port_next[nring] = newc->port;
        strcpy(ent.ip_next[nring], newc->ip);
        verbose("Insertion server: modified entity :\n%s\n", entitytostr(nring));
        free(newc);
        // ACKC confirmation sending
        verbose("Insertion server: sending ACKC confirmation message...\n");
        send(sock2, "ACKC\n", 5, 0);
        verbose("Insertion server: message sent.\n");
        // closing connection
        close(sock2);
        verbose("Insertion server: connection closed.\n");
    }
}



static void *packet_treatment(void *args) {
    char *packet = (char *)args;
    packet[512] = 0;
    if (parsemsg(packet) != -1)
        sendpacket(packet);
    return NULL;
}


static void launch_message_manager() {
    pthread_t t_message_manager;
    pthread_create(&t_message_manager, NULL, message_manager, NULL);
}
////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////
/**
 * Protocol for insertion of current entity into a ring.
 *
 * @param hostname of the entity on the ring
 * @param port of the entity on the ring
 * @return 1 if insertion succed, 0 else
 */
int insert(const char *host, const char *tcpport) {
    // preparing the structure
    struct sockaddr_in *addr;
    struct addrinfo *first_info;
    struct addrinfo hints;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, tcpport, &hints, &first_info) != 0 ||
            first_info == NULL) {
        fprintf(stderr, 
                "Can't get address of %s at port %s.\n", host, tcpport);
        return 0;
    }
    addr = (struct sockaddr_in *) first_info->ai_addr;
    // socket creation
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr *)addr,
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
    freeaddrinfo(first_info);
    // Socket creation
    verbose("Creating sockets for UDP communication...\n");
    _ent.socklisten = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp listening created.\n");
    verbose("Binding socket for listening...\n");
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(ent.udp);
    address_sock.sin_addr = addr->sin_addr; // Bind udp socket to the address from tcp connection
    int r=bind(_ent.socklisten,(struct sockaddr *)&address_sock,
            sizeof(struct sockaddr_in));
    if (r == -1) {
        fprintf(stderr, "Binding error. Insertion failed.\n");
        return 0;
    }
    verbose("Binding done.\n");
    _ent.socksend[nring]   = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp sending created.\n");
    verbose("Preparing structure for receiver address...\n");
    // receiver (next entity) socket
    char *port = itoa4(ent.port_next[nring]);
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
#ifdef DEBUG
    if (getaddrinfo(ent.ip_next[nring], port, &hints, &first_info) == -1) {
        debug("insert()", 
                UNDERLINED
                "getaddrinfo(ent.ip_next[nring], port, &hints, &first_info)\n" RESET \
                "Returned -1 for host %s at port \"%s\".",
                ent.ip_next[nring], port);
        exit(1);
    }
#else
    getaddrinfo(ent.ip_next[nring], port, &hints, &first_info);
#endif
    memcpy(&_ent.receiver[nring], first_info->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(first_info);
    free(port);
    // TODO multi diff
    verbose("Structure prepared.\n");
    verbose("Socket for UDP communication prepared.");
    verbose("Insertion done.\n");

    verbose("Starting message manager...\n");
    launch_message_manager();
    verbose("Message manager started.\n");
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
            debug("message_manager", "Packet received:\n---\n%s\n---\n", buff);
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


void sendpacket(char *content) {
    debug("sendpacket(char *content)", "Sending packet:\n---\n%s\n---\n...\n", content);
    for (int i = 0; i <= nring; ++i) {
        sendto(_ent.socksend[i], content, 512, 0,
                (struct sockaddr *) &_ent.receiver[i],
                (socklen_t)sizeof(struct sockaddr_in));
#ifdef DEBUG
        debug("sendpacket(content)", "Packet sent to ip %s at port %d.",
                inet_ntoa(_ent.receiver[i].sin_addr), 
                ntohs(_ent.receiver[i].sin_port));
#else
        verbose("Packet sent to %s at port %u.\n",
                ent.ip_next[i], ent.port_next[i]);
#endif
    }
}



/**
 * Initialize entity with given attributes.
 * ip_next and port_next are set to ip_self and udp_listen.
 */
void init_entity(char *id, uint16_t udp_listen, uint16_t tcp_listen) {
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
    ent.port_next[0] = ent.udp;
    // mdiff_ip
    strcpy(ent.mdiff_ip[0], "255.255.255.255");
    // mdiff port
    ent.mdiff_port[0] = 6666;

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
    debug("create_ring()", "Ring index: %d.\nEntity:\n%s\n", nring, entitytostr(nring));
    // Socket creation
    verbose("Creating sockets for UDP communication...\n");
    // listening socket
    _ent.socklisten = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp listening created.\n");
    verbose("Binding socket for listening...\n");
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(ent.udp);
    address_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    int r=bind(_ent.socklisten,(struct sockaddr *)&address_sock,
            sizeof(struct sockaddr_in));
    if (r == -1) {
        fprintf(stderr, "Binding error. Ring creation failed.\n");
        exit(1);
    }
    verbose("Binding done.\n");
    _ent.socksend[nring]   = socket(PF_INET, SOCK_DGRAM, 0);
    verbose("Socket for udp sending created.\n");
    // receiver (next entity) socket
    verbose("Preparing structure for receiver address...\n");
    char *port = itoa4(ent.port_next[nring]);
    struct addrinfo hints;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    struct addrinfo *first_info;
#ifdef DEBUG
    if (getaddrinfo(ent.ip_self, port, &hints, &first_info) == -1) {
        debug("create_ring()", 
                UNDERLINED
                "getaddrinfo(\"localhost\", port, &hints, &first_info)\n" RESET \
                "Returned -1 for port \"%s\".",
                port);
        exit(1);
    }
#else
    getaddrinfo(ent.ip_self, port, &hints, &first_info);
#endif
    memcpy(&_ent.receiver[nring], first_info->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(first_info);
    free(port);
    verbose("Sockets created.\n");

    // lauch message manager thread
    pthread_t t_message_manager;
    pthread_create(&t_message_manager, NULL, message_manager, NULL);
    // lauch insertion server thread
    pthread_t t_insertion_server;
    pthread_create(&t_insertion_server, NULL, insertion_server, NULL);

    verbose("Ring created.\n");

}


