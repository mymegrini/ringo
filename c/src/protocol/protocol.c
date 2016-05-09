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
#include <time.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>




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
entity _ent_;
entity *ent = (entity * )&_ent_;

_entity __ent = {
  .socklisten = NEED_SOCKET,
  .socksend = NEED_SOCKET,
  .socktcp = NEED_SOCKET
};
_entity *_ent = & __ent;

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
volatile const int * const ring_number = &nring;

short volatile ring_check[NRING+1];
/*short ring_check[NRING+1];*/
short volatile *rc = ring_check;

struct pollfd poll_mdiff[NRING];

extern short volatile need_thread;

#define TIMEOUT 30

volatile struct test_data _test_data;
volatile struct test_data *test_data = &_test_data;
////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////



static welc_msg *parse_welc(const char *w_msg)
{
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


static newc_msg *parse_newc(const char *n_msg)
{
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


static dupl_msg *parse_dupl(const char *d_msg)
{
  return parse_welc(d_msg);
}


/**
 * Prepare WELC message for insertion protocol
 *
 * @return WELC message
 */
static char *prepare_welc()
{
  char *msg = (char *)malloc(50);
  char port_next[5], mdiff_port[5];
  itoa4(port_next, ent->port_next[nring]);
  itoa4(mdiff_port, ent->mdiff_port[nring]);
  sprintf(msg, "WELC %s %s %s %s\n",
      ent->ip_next[nring], port_next,
      ent->mdiff_ip[nring], mdiff_port);
  return msg;
}



/**
 * Prepare NEWC message for insertion protocol
 *
 * @return NEWC message
 */
static char *prepare_newc()
{
  char *msg = (char *)malloc(30);
  char udp[5];
  itoa4(udp, ent->udp);
  sprintf(msg, "NEWC %s %s\n", ent->ip_self, udp);
  return msg;
}

/**
 * Prepare WELC message for insertion protocol
 *
 * @return WELC message
 */
static char *prepare_dupl(const char *mip, uint16_t mport)
{
  char *msg = (char *)malloc(50);
  char port[5], mdiff_port[5], mdiff_ip[16];
  itoa4(port, ent->udp);
  itoa4(mdiff_port, mport);
  ipresize_noalloc(mdiff_ip, mip);
  sprintf(msg, "DUPL %s %s %s %s\n",
      ent->ip_self, port, mdiff_ip, mdiff_port);
  debug("prepare_dupl", "%s\nmdiff_ip:%s\nmdiff_port:%s", msg, mdiff_ip, mdiff_port);
  return msg;
}
/* static char *prepare_dupl() */
/* { */
/*   char *msg = (char *)malloc(50); */
/*   char port[5], mdiff_port[5]; */
/*   itoa4(port, ent->udp); */
/*   itoa4(mdiff_port, ent->mdiff_port[nring]); */
/*   sprintf(msg, "DUPL %s %s %s %s\n", */
/*       ent->ip_self, port, */
/*       ent->mdiff_ip[nring], mdiff_port); */
/*   debug("prepare_dupl", "%s\nmdiff_ip:%s\nmdiff_port:%s", msg, ent->mdiff_ip[nring], mdiff_port); */
/*   return msg; */
/* } */


static void insert(int ring, char *n_msg, int sock2)
{
  verbose(UNDERLINED "Insertion server: parsing NEWC message...\n" RESET);
  newc_msg *newc = parse_newc(n_msg);
  if (newc == NULL) {
    fprintf(stderr, "Protocol error: bad response from client->\nInsertion failed.\n");
    free(newc);
    return;
  }
  verbose(UNDERLINED "Insertion server: NEWC parsing successful.\n" RESET);
  // Actualize udp communication
  verbose(UNDERLINED "Insertion server: actualizing socket informations for next entity...\n" RESET);
  // receiver (next entity) socket
  verbose(UNDERLINED "Preparing structure for receiver address...\n" RESET);
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
  verbose(UNDERLINED "Structure prepared.\n" RESET);
  // modifying entity
  verbose(UNDERLINED "Insertion server: modifying current entity...\n" RESET);
  verbose(UNDERLINED "Insertion server: current entity :\n%s\n" RESET, entitytostr(ring));
  strcpy(ent->ip_next[ring], newc->ip);
  ent->port_next[ring] = newc->port;
  verbose(UNDERLINED "Insertion server: modified entity :\n%s\n" RESET, entitytostr(ring));
  free(newc);
  // ACKC confirmation sending
  verbose(UNDERLINED "Insertion server: sending ACKC confirmation message...\n" RESET);
  send(sock2, "ACKC\n", 5, 0);
  verbose(UNDERLINED "Insertion server: message ACKC sent.\n" RESET);
  _ent->receiver[ring] = receiver;
  verbose(UNDERLINED "Actualizing receviver...\n" RESET);
  verbose(UNDERLINED "Current structure replaced.\n" RESET);
  // closing connection
  /*close(sock2);*/
  debug("insert", MAGENTA "modified entity:\n%s", entitytostr(ring));
}



static void duplicate(char *d_msg, int sock2)
{
  verbose(UNDERLINED "Insertion server: parsing DUPL message...\n" RESET);
  dupl_msg *dupl = parse_dupl(d_msg);
  if (dupl == NULL) {
    verbose(UNDERLINED "Protocol error: bad response from client->\nInsertion failed.\n" RESET);
    free(dupl);
    return;
  }
  verbose(UNDERLINED "Insertion server: DUPL parsing successful.\n" RESET);
  verbose(UNDERLINED "Retreiveing struct for communication udp communication...\n" RESET);
  char ip[16];
  ipnozeros(ip, dupl->ip);
  struct sockaddr_in receiver;
  if (!getsockaddr_in(&receiver, ip, dupl->port, 1)) {
    verbose(UNDERLINED "Can't create communication with %s on port %d.\n" RESET
        "Dupplication failed.\n", ip, dupl->port);
    free(dupl);
    return;
  }
  verbose(UNDERLINED "Struct retreived.\n" RESET);
  int sockmdiff = socket(AF_INET, SOCK_DGRAM, 0);
  ipnozeros(ip, dupl->ip_diff);
  verbose(UNDERLINED "Subscribing to multicast channel ip %s on port %d.\n" RESET, 
      ip, dupl->port_diff);
  if (!multicast_subscribe(sockmdiff, dupl->port_diff, ip)) {
    verbose(UNDERLINED "Can't subscribe to channel ip %s on port %d.\n" RESET
        "Dupplication failed.\n", ip, dupl->port_diff);
    close(_ent->sockmdiff[nring+1]);
    free(dupl);
    return;
  }
  verbose(UNDERLINED "Sending confirmation message with listening port...\n" RESET);
  char msg[11];
  char port[5];
  itoa4(port, ent->udp);
  sprintf(msg, "ACKC %s\n", port);
  send(sock2, msg, 10, 0);
  verbose(UNDERLINED "Confirmation message sent->\n" RESET);

  verbose(UNDERLINED "Modifying entity...\n" RESET);
  ++nring;
  _ent->receiver[nring] = receiver;
  _ent->sockmdiff[nring] = sockmdiff;
  ent->port_next[nring] = dupl->port;
  strcpy(ent->ip_next[nring], dupl->ip);
  ent->mdiff_port[nring] = dupl->port_diff;
  strcpy(ent->mdiff_ip[nring], dupl->ip_diff);
  verbose(UNDERLINED "Entity modified.\n" RESET);
  verbose(UNDERLINED "Ring number actualized. Number of rings: %d.\n" RESET, nring+1);
  verbose(UNDERLINED "Dupplication finished.\n" RESET);
}
/**
 * Server waiting for new entity insertions
 */
static void insertionsrv()
{
  verbose(UNDERLINED "Starting insertion server...\n" RESET);
  // socket preparation
  int sock = socket(PF_INET,SOCK_STREAM, 0);
  struct sockaddr_in addr_sock;
  // server socket filling
  addr_sock.sin_family      = AF_INET;
  addr_sock.sin_port        = htons(ent->tcp);
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
  _ent->socktcp = sock;
  while (1) {
    // wait for connection
    verbose(UNDERLINED "Insertion server: waiting for client...\n" RESET);
    struct sockaddr_in caller;
    socklen_t size = sizeof(caller);
    int sock2;
    sock2 = accept(sock, (struct sockaddr *)&caller, &size);
    // error from accept
    if ( sock2 == -1 ) {
      perror("Error accept.");
      continue;
    }
    verbose(UNDERLINED "Insertion server: connection established.\n" RESET);
    verbose(UNDERLINED "Locking access to entity...\n" RESET);
    wlock_entity();
    verbose(UNDERLINED "Access locked.\n" RESET);
    // insertion protocol
    // WELC message sending
    verbose(UNDERLINED "Insertion server: preparing WELC message...\n" RESET);
    char *msg = prepare_welc();
    verbose(UNDERLINED "Insertion server: sending \"%s\"...\n" RESET, msg);
    send(sock2, msg, strlen(msg), 0);
    verbose(UNDERLINED "Insertion server: message WELC sent->\n" RESET);
    free(msg);
    // NEWC message reception
    verbose(UNDERLINED "Insertion server: waiting for NEWC message...\n" RESET);
    msg = receptLine(sock2);
    verbose(UNDERLINED "Insertion server: received : \"%s\".\n" RESET, msg);
    if (strncmp(msg, "NEWC", 4) == 0)
      insert(nring, msg, sock2);
    else if (strncmp(msg, "DUPL", 4) == 0)
      duplicate(msg, sock2);
    else
      verbose(UNDERLINED "Message not supported: \"%s\".\n" RESET, msg);
    verbose(UNDERLINED "Unlocking access to entity...\n" RESET);
    unlock_entity();
    verbose(UNDERLINED "Access unlocked.\n" RESET);
    verbose(UNDERLINED "Closing connection...\n" RESET);
    close(sock2);
    verbose(UNDERLINED "Connection closed.\n" RESET);
    free(msg);
  }
}



static void *packet_treatment(void *args)
{
  char *packet = (char *)args;
  packet[512] = 0;
  verbose("Packet received:\n%s\n", packet);
  parsemsg(packet);
  free(packet);
  return NULL;
}



static void test_ring()
{
  // send test messages in each rings
  rlock_entity();
  test_data->nring = nring+1;
  test_data->count = test_data->nring;
  char port_diff[5];
  debug("ring_test", GREEN "test beginning...");
  for (int i = 0; i < test_data->nring; i++) {
    test_data->ring_check[i] = 0;
    itoa4(port_diff, ent->mdiff_port[i]);
    sendmessage(i, "TEST", "%s %s", ent->mdiff_ip[i], port_diff);
  }
  unlock_entity();
  debug("test_ring", GREEN "timeout beginning...");
  int err = 0;
  struct timespec to = {0,0};
  to.tv_sec = time(NULL) + TIMEOUT;
  pthread_mutex_lock(&mutex->test.m);
  while (test_data->count && err == 0) {
    err = pthread_cond_timedwait(&mutex->test.c, &mutex->test.m, &to);
  }
  if (err != 0) {
    debug("test_ring", GREEN "end of timeout.");
    for (int i = 0; i < test_data->nring; i++) {
      if (test_data->ring_check[i]) {
        debug("test_ring", GREEN "ring %d: checked.", i);
        continue;
      }
      else {
        debug("test_ring", GREEN "ring %d: checking failed. Ring broken...", i);
        continue;
      }
    }
  }
  else
    debug("test_ring", GREEN "all ring checked.");
  pthread_mutex_unlock(&mutex->test.m);
}


////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

/**
 * Return a string representing current entity.
 *
 * @return the string representing current entity
 */
char *entitytostr(int ring)
{
  char *str = (char *)malloc(400);
  char *id = malloc(50), *udp = malloc(50), *tcp = malloc(50),
       *ip = malloc(50), *np = malloc(50), *mdip = malloc(50), 
       *mdp = malloc(50), ip_self[ 50 ];
  sprintf(ip_self, "%30s - %s", "\x1b[4mself ip\x1b[0m", ent->ip_self);
  sprintf(id, "%30s - %s", "\x1b[4mid\x1b[0m", ent->id);
  sprintf(udp, "%30s - %d", "\x1b[4mudp listening port\x1b[0m", ent->udp);
  sprintf(tcp, "%30s - %d", "\x1b[4mtcp listening port\x1b[0m", ent->tcp);
  sprintf(ip, "%30s - %s", "\x1b[4mip of next entity\x1b[0m", ent->ip_next[ring]);
  sprintf(np, "%30s - %d", "\x1b[4mport of next entity\x1b[0m", ent->port_next[ring]);
  sprintf(mdip, "%30s - %s", "\x1b[4mmultidiff ip\x1b[0m", ent->mdiff_ip[ring]);
  sprintf(mdp, "%30s - %d", "\x1b[4mmultidiff port\x1b[0m", ent->mdiff_port[ring]);
  sprintf(str, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", ip_self, id, udp, tcp, ip, np, mdip, mdp);
  free(id);free(udp);free(tcp);free(ip);free(np);free(mdip);free(mdp);
  return str;
}



/**
 * Mapping of function insertionsrv to fit the thread signature
 */
void *insertion_server(void *arg)
{
  insertionsrv();
  return NULL;
}



/**
 * Message manager thread
 *
 * Process messages received from the ring.
 */
void *message_manager(void *args)
{
  char buff[513];
  verbose(UNDERLINED "Message manager launched.\n" RESET);
  while (1) {
    int rec = recv(_ent->socklisten, buff, 512, 0);
#ifdef DEBUG
    if (rec > 0) {
      buff[rec] = 0;
      debug("message_manager", 
          YELLOW "Packet received of length %d:\n---\n%s\n---\n", rec, buff);
    }
#endif
    if (rec == 512) {
      char *packet = strndup(buff, 512);
      pthread_t t_packet_treat;
      pthread_create(&t_packet_treat, NULL, packet_treatment, (void*) packet);
    }

  }
  return NULL;
}



void sendpacket(const char *content, int ring)
{
  rlock_entity();
  debug("sendpacket(char *content, int ring)",
      "Sending packet solo ring:\n---\n%s\n---\n...\n"
      "To ip %s on port %d.", content, inet_ntoa(_ent->receiver[ring].sin_addr),
      ntohs(_ent->receiver[ring].sin_port));
  sendto(_ent->socksend, content, 512, 0,
      (struct sockaddr *) &_ent->receiver[ring],
      (socklen_t)sizeof(struct sockaddr_in));
  verbose("Packet sent:\n%s\n", content);
  unlock_entity();
}

void sendpacket_all(const char *content)
{
  rlock_entity();
  debug("sendpacket_all(char *content)", 
      "Sending packet multiple ring (%d):\n---\n%s\n---\n...\n", nring, content);
  for (int i = 0; i < nring + 1; ++i) {
    sendto(_ent->socksend, content, 512, 0,
        (struct sockaddr *)&_ent->receiver[i],
        (socklen_t)sizeof(struct sockaddr_in));
  }
  verbose("Packets sent:\n%s\n", content);
  unlock_entity();
}


void sendpacket_sockaddr(const char *content, const struct sockaddr_in *receiver)
{
  debug("sendpacket_sockaddr(char *content, struct sockaddr_in *receiver)", 
      BLUE "Sending packet solo:\n---\n%s\n---\n...\n"
      "To ip %s on port %d.", content, inet_ntoa(receiver->sin_addr),
      ntohs(receiver->sin_port));
  sendto(_ent->socksend, content, 512, 0,
      (struct sockaddr *) receiver,
      (socklen_t)sizeof(struct sockaddr_in));
  verbose("Packet sent:\n%s\n", content);
}
/**
 * Initialize entity with given attributes.
 * ip_next and port_next are set to ip_self and udp_listen.
 */
void init_entity(char *id, uint16_t udp_listen, uint16_t tcp_listen,
    char *mdiff_ip, uint16_t mdiff_port) {
  // id
  strncpy(ent->id, id, 8);
  // ip_self
  char *ip = getLocalIp();
  strcpy(ent->ip_self, ip);
  free(ip);
  // udp_listen
  ent->udp = udp_listen;
  // tcp_listen
  ent->tcp = tcp_listen;
  // ip_next[0]
  strcpy(ent->ip_next[0], ent->ip_self);
  // port_next init
  ent->port_next[0] = udp_listen;
  // mdiff_ip
  if (mdiff_ip) {
    ip = ipresize(mdiff_ip);
    strcpy(ent->mdiff_ip[0], ip);
    // mdiff port
    ent->mdiff_port[0] = mdiff_port;
  }

  debug("init_entity", "%s\n", entitytostr(0));
}



static int init_sockets(uint16_t udp_listen) {
  if (_ent->socklisten == NEED_SOCKET) {
    // Socket creation
    verbose(UNDERLINED "Creating sockets for UDP communication...\n" RESET);
    // listening socket
    _ent->socklisten = socket(PF_INET, SOCK_DGRAM, 0);
    verbose(UNDERLINED "Socket for udp listening created.\n" RESET);
    if (!bind_udplisten(_ent->socklisten, ent->udp)) {
      fprintf(stderr, "Binding error. Ring creation failed.\n");
      return 0;
    }
    verbose(UNDERLINED "Binding done.\n" RESET);
  }
  if (_ent->socksend == NEED_SOCKET) {
    _ent->socksend  = socket(PF_INET, SOCK_DGRAM, 0);
    verbose(UNDERLINED "Socket for udp sending created.\n" RESET);
  }
  return _ent->socklisten != -1 && _ent->socksend != -1;
}



static void actualize_receiver(int ring, char ip_next[16], uint16_t port_next, 
    const struct sockaddr_in *receiver) {
  verbose(UNDERLINED "Writing new entity...\n" RESET);
  wlock_entity();

  strcpy(ent->ip_next[ring], ip_next);
  ent->port_next[ring] = port_next;
  _ent->receiver[ring] = *receiver;

  unlock_entity();
  verbose(UNDERLINED "Entity written.\n" RESET);
}


static void add_ring(char ip_next[16], uint16_t port_next, char mdiff_ip[16],
    uint16_t mdiff_port, const struct sockaddr_in *receiver, int mdiff_sock) {

  verbose(UNDERLINED "Writing new entity...\n" RESET);
  wlock_entity();

  ++nring;

  strcpy(ent->ip_next[nring], ip_next);
  strcpy(ent->mdiff_ip[nring], mdiff_ip);

  ent->port_next[nring]    = port_next;
  ent->mdiff_port[nring]   = mdiff_port;
  _ent->receiver[nring]    = *receiver;
  _ent->sockmdiff[nring]   = mdiff_sock;
  poll_mdiff[nring].fd     = mdiff_sock;
  poll_mdiff[nring].events = POLLIN;

  fcntl(mdiff_sock, F_SETFL, O_NONBLOCK);

  unlock_entity();
  verbose(UNDERLINED "Entity written.\n" RESET);
}



static int tcp_connection(const char *host, const char *tcpport)
{
  /* if (nring == NRING-1) { */
  /*   fprintf(stderr, "Maximum number of ring reached (%d).\n", NRING); */
  /*   return 0; */
  /* } */
  // preparing the structure
  struct sockaddr_in addr;
  if (!getsockaddr_in(&addr, host, atoi(tcpport), 0)) {
    debug("tcp_connection", "Can't get address of %s at port %s.\n", host, tcpport);
    fprintf(stderr, "Can't get address of %s at port %s.\n", host, tcpport);
    return 0;
  }
  // socket creation
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  debug("tcp_connection", "socket created: %d", sock);
  if (connect(sock, (struct sockaddr *)&addr,
        (socklen_t)sizeof(struct sockaddr_in)) != 0)
  {
    close(sock);
    fprintf(stderr,
        "Can't establish connection with %s on port %s.\n", host, tcpport);
    return 0;
  }
  verbose(UNDERLINED "Connection established with %s on port %s.\n" RESET, host, tcpport);
  return sock;
}



/**
 * Create a ring.
 *
 * Create the sockets if needed.(socket == NEED_SOCKET)
 * Launch the threads if needed (need_thread == 1)
 */
int create_ring2(char *mdiff_ip, uint16_t mdiff_port)
{
  if (nring == NRING-1) {
    fprintf(stderr, "Maximum number of ring reached (%d).\n", NRING);
    return 0;
  }
  if (!init_sockets(ent->udp)) {
    fprintf(stderr, "Socket error.\n");
    return 0;
  }
  struct sockaddr_in receiver;
  // receiver (next entity) socket
  verbose(UNDERLINED "Preparing structure for receiver address...\n" RESET);
  if (!getsockaddr_in(&receiver, "localhost", ent->udp, 0)) {
    fprintf(stderr, "Can't access to localhost on port %d.\n"
        "Ring creation failed.\n", ent->udp);
    return 0;
  }
  verbose(UNDERLINED "Sockets created.\n" RESET);

  // multidiffusion
  int sockmdiff = socket(PF_INET, SOCK_DGRAM, 0);
  // authorize multidiff on same machine
  verbose(UNDERLINED "Subscibing to multicast channel %s on port %d...\n" RESET, 
      mdiff_ip, mdiff_port);
  if (!multicast_subscribe(sockmdiff, mdiff_port,
        mdiff_ip)) {
    fprintf(stderr, "Can't subscribe to channel ip %s on port %d.\n"
        "Ring creation failed.\n", mdiff_ip,
        mdiff_port);
    return 0;
  }
  verbose(UNDERLINED "Subscribed to multicast channel.\n" RESET);
  wlock_entity();
  char mdiff_ipr[16];
  ipresize_noalloc(mdiff_ipr, mdiff_ip);
  add_ring(ent->ip_self, ent->udp, mdiff_ipr, mdiff_port, &receiver, sockmdiff);

  init_threads();
  verbose(UNDERLINED "Ring created.\n" RESET);
  return 1;
}


/**
 * Protocol for insertion of current entity into a ring.
 *
 * @param hostname of the entity on the ring
 * @param port of the entity on the ring
 * @return 1 if insertion succed, 0 else
 */
int join2(const char *host, const char *tcpport)
{
  if (nring == NRING-1) {
    fprintf(stderr, "Maximum number of rings reached (%d).\n", NRING);
    return 0;
  }
  int sock = tcp_connection(host, tcpport);
  if (!sock) {
    debug("join2", "host: %s, tcp: %s, sock: %d", host, tcpport, sock);
    return 0;
  }
  // WELC message reception
  verbose(UNDERLINED "waitig for WELC message...\n" RESET);
  char *msg = receptLine(sock);
  verbose(UNDERLINED "Message received : \" RESET%s\".\n", msg);
  verbose(UNDERLINED "Parsing message...\n" RESET);
  welc_msg *welc = parse_welc(msg);
  free(msg);
  if (welc == NULL) {
    fprintf(stderr, "Protocol error: bad response.\nInsertion failed.\n");
    free(welc);
    return 0;
  }
  verbose(UNDERLINED "Parsing successfull.\n" RESET);
  // NEWC message sending
  verbose(UNDERLINED "Preparing NEWC message...\n" RESET);
  char *newc_str = prepare_newc();
  verbose(UNDERLINED "Sending: \"%s\".\n" RESET, newc_str);
  send(sock, newc_str, strlen(newc_str), 0);
  verbose(UNDERLINED "Message sent.\n" RESET);
  // ACKC message reception
  verbose(UNDERLINED "Waiting for ACKC confirmation message...\n" RESET);
  msg = receptLine(sock);
  verbose(UNDERLINED "Message received: \" RESET%s\".\n", msg);
  if (strcmp(msg, "ACKC") != 0) {
    fprintf(stderr, "Protocol error: bad response.\nInsertion failed.\n");
    free(welc);
    free(msg);
    return 0;
  }

  if (!init_sockets(ent->udp)) {
    fprintf(stderr, "Socket error.\n");
    return 0;
  }
  char ipnz[16];
  ipnozeros(ipnz, welc->ip);
  struct sockaddr_in receiver;
  if (!getsockaddr_in(&receiver, ipnz,
        welc->port, 1)) {
    fprintf(stderr, "Can't communicate with address %s on port %d.\n",
        ipnz, welc->port);
    return 0;
  }
  // multi diff
  char mdiff_ip[16];
  ipnozeros(mdiff_ip, welc->ip_diff);
  verbose(UNDERLINED "Subscribing to channel %s...\n" RESET, mdiff_ip);
  int sockmdiff = socket(AF_INET, SOCK_DGRAM, 0);
  if (!multicast_subscribe(sockmdiff, welc->port_diff,
        mdiff_ip)) {
    fprintf(stderr, 
        "can't subscribe to multicast channel ip %s on port %d\n",
        mdiff_ip, welc->port_diff);
    return 0;
  }
  
  add_ring(welc->ip, welc->port, welc->ip_diff, welc->port_diff, &receiver, sockmdiff);
  verbose(UNDERLINED "Insertion done.\n" RESET);
  debug("insert", MAGENTA "modified entity:\n%s", entitytostr( nring ));

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
int duplicate_rqst2(const char *host, const char *tcpport, const char *mdiff_ip, uint16_t mdiff_port)
{
  if (nring == NRING-1) {
    fprintf(stderr, "Maximum number of rings reached (%d).\n", NRING);
    return 0;
  }
  int sock = tcp_connection(host, tcpport);
  if (!sock) {
    return 0;
  }
  verbose(UNDERLINED "waitig for WELC message...\n" RESET);
  char *msg = receptLine(sock);
  verbose(UNDERLINED "Message received : \"%s\".\n" RESET, msg);
  verbose(UNDERLINED "Parsing message...\n" RESET);
  welc_msg *welc = parse_welc(msg);
  free(msg);
  if (welc == NULL) {
    fprintf(stderr, "Protocol error: bad response.\nInsertion failed.\n");
    free(welc);
    return 0;
  }
  verbose(UNDERLINED "Parsing successfull.\n" RESET);
  // NEWC message sending
  verbose(UNDERLINED "Preparing DUPL message...\n" RESET);
  char *dupl_str = prepare_dupl(mdiff_ip, mdiff_port);
  verbose(UNDERLINED "Sending: \"%s\".\n" RESET, dupl_str);
  send(sock, dupl_str, strlen(dupl_str), 0);
  verbose(UNDERLINED "Message sent->\n" RESET);
  fflush(stdout);
  // ACKC message reception
  verbose(UNDERLINED "Waiting for ACKC confirmation message...\n" RESET);
  msg = receptLine(sock);
  verbose(UNDERLINED "Message received: \"%s\".\n" RESET, msg);
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

  // Preparing structure
  if (!init_sockets(ent->udp)) {
    fprintf(stderr, "Socket error.\n");
    return 0;
  }
  char ipnz[16];
  ipnozeros(ipnz, welc->ip);
  struct sockaddr_in receiver;
  if (!getsockaddr_in(&receiver, ipnz,
        welc->port, 1)) {
    fprintf(stderr, "Can't communicate with address %s on port %d.\n",
        ipnz, welc->port);
    return 0;
  }
  // multi diff
  verbose(UNDERLINED "Subscribing to channel %s...\n" RESET, mdiff_ip);
  int sockmdiff = socket(AF_INET, SOCK_DGRAM, 0);
  if (!multicast_subscribe(sockmdiff, mdiff_port, mdiff_ip)) {
    fprintf(stderr, 
        "can't subscribe to multicast channel ip %s on port %d\n",
        mdiff_ip, welc->port_diff);
    return 0;
  }
  
  char mdiff_ipr[16];
  ipresize_noalloc(mdiff_ipr, mdiff_ip);
  add_ring(welc->ip, welc->port, mdiff_ipr, mdiff_port, &receiver, sockmdiff);
  verbose(UNDERLINED "Dupplication done.\n" RESET);
  debug("duplicate_rqst", MAGENTA "modified entity:\n%s", entitytostr( nring ));

  init_threads();

  return 1;
}


void *ring_tester(void *args)
{
  unsigned interval = 20;
  while (1) {
    sleep(interval);
    test_ring();
  }
}


void rm_ring(int ring)
{
  wlock_entity();
  debug(RED "rm_ring", RED "removing ring %d", ring);
  if (ring < nring) {
    _ent->receiver[ring] = _ent->receiver[nring];
    strcpy(ent->ip_next[ring], ent->ip_next[nring]);
    ent->port_next[ring] = ent->port_next[nring];
    strcpy(ent->mdiff_ip[ring], ent->mdiff_ip[nring]);
    ent->mdiff_port[ring] = ent->mdiff_port[nring];
  }
  close(_ent->sockmdiff[ring]);
  verbose(UNDERLINED "Ring %d quit.\n" RESET, ring);
  debug(RED "rm_ring", RED "Ring %d quit.\n", ring);
  unlock_entity();
  if (--nring == -1) {
    verbose(UNDERLINED "Last ring quit.\n" RESET);
    close_threads();
  }
  else {
    debug("rm_ring", "Actual number of ring: %d", nring);
    debug("rm_ring", "Sending SIGUSR1 signal to mdiff_manager...");
    int r = pthread_kill(thread->mdiff_manager, SIGUSR1);
    debug("rm_ring", "SIGUSR1 sent, kill retval: %d.", r);
  }
}




void *mdiff_manager(void *args)
{
  sigset_t mask;
	sigemptyset (&mask);
	sigaddset (&mask, SIGUSR1);
  /* pthread_sigmask(SIG_BLOCK, &mask, NULL); */
  char buff[513];
  int ret;
#ifdef DEBUG
  int count = 0;
#endif
  while (1) {
    debug("mdiff_manager", "ppoll number %d...", ++count);
    int ret = ppoll(poll_mdiff, nring+1, NULL, &mask);
    if (ret > 0) {
#ifdef DEBUG
      int found = 0;
#endif
      for (int i = 0; i < nring; ++i) {
        if (poll_mdiff[i].revents == POLLIN) {
#ifdef DEBUG
          found = 1;
#endif
          int rec1 = recv(poll_mdiff[i].fd, buff, 512, 0);
          if (rec1 >= 0) {
            buff[rec1] = 0;
            debug("mdiff_manager", "message received: %s", buff);
            if (strcmp(buff, "DOWN"))
              rm_ring(i);
          }
#ifdef DEBUG
          else {
            debug("mdiff_manager", "poll_mdiff[%d].revents == POLLIN whereas recv returns %d",
                i, rec1);
          }
#endif
        }
      }
#ifdef DEBUG
      if (!found) 
        debug("mdiff_manager", "Is it SIGUSR1?");
#endif
    }
#ifdef DEBUG
    else {
      debug("mdiff_manager", "pprol retval: %d", ret);
    }
#endif
  }
}

