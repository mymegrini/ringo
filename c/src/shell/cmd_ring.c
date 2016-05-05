#include "../protocol/protocol.h"
#include "../protocol/common.h"
#include <getopt.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#define MODE_JOIN "join"
#define MODE_CREA "create"
#define MODE_DUPL "dupplicate"

/* #define OPT_JOIN 'j' */
/* #define OPT_DUPL 'd' */
/* #define OPT_CREA 'c' */
#define OPT_MODE 'm'
#define OPT_HELP 'h'

#define OPT_HOST_PORT 't'
#define OPT_HOST_ADDR 'a'
#define OPT_MDIF_PORT 'u'
#define OPT_MDIF_ADDR 'i'

/* #define OPTL_JOIN "join" */
/* #define OPTL_DUPL "duplicate" */
/* #define OPTL_CREA "create" */
#define OPTL_MODE "mode"
#define OPTL_HELP "help"

#define OPTL_HOST_PORT "host-tcp-port"
#define OPTL_HOST_ADDR "host-addr"
#define OPTL_MDIF_PORT "mdiff-udp-port"
#define OPTL_MDIF_ADDR "mdiff-ip"

#define OPT_STRING "ht:a:m:u:i:"

static struct option longopts[] = {
  {OPTL_HELP,      no_argument,         0,   OPT_HELP},
  {OPTL_MODE,      required_argument,   0,   OPT_MODE},
  {OPTL_HOST_PORT, required_argument,   0,   OPT_HOST_PORT},
  {OPTL_HOST_ADDR, required_argument,   0,   OPT_HOST_ADDR},
  {OPTL_MDIF_PORT, required_argument,   0,   OPT_MDIF_PORT},
  {OPTL_MDIF_ADDR, required_argument,   0,   OPT_MDIF_ADDR},
  /* {OPTL_DUPL,   no_argument,         0,   OPT_DUPL}, */
  /* {OPTL_CREA,   no_argument,         0,   OPT_CREA}, */
  /* {OPTL_JOIN,   no_argument,         0,   OPT_JOIN}, */
  {0,           0,                   0,   0}
};


static void usage(char *argv0) {
  printf("Usage:\t%s -h --mode=<mode> -t <tcp port> -a <hostname/adress> -u <mdiff port> -i <mdiff ip>\n\n"
      "Mode can be one of create, duplicate or join.\n",
      argv0);
}


static void help(char *argv0) {
  usage(argv0);
}



#define CREA 1
#define DUPL 2
#define JOIN 3

int cmd_ring(int argc, char **argv) {
  int mode = 0;
  int c, indexptr;

  char *mdiff_ip = NULL;
  uint16_t mdiff_port = 0;
  char *hostname = NULL;
  char *host_port = NULL;
  
  optind = 0;

  while ((c = getopt_long(argc, argv, OPT_STRING, longopts, &indexptr)) != -1) {
    
    switch(c) {
      case OPT_HELP:
        help(argv[0]);
        return 0;
      case OPT_MODE:
        if (strcmp(optarg, MODE_CREA) == 0)
          mode = CREA;
        else if (strcmp(optarg, MODE_CREA) == 0)
          mode = DUPL;
        else if (strcmp(optarg, MODE_JOIN) == 0)
          mode = JOIN;
        else {
          usage(argv[0]);
          return 1;
        }
        break;
      case OPT_HOST_PORT:
        if (!isnumeric(optarg)) {
          fprintf(stderr, "Host port must be integer value.\n");
          return 1;
        }
        host_port = strdup(optarg);
        break;
      case OPT_MDIF_PORT:
        if (!isnumeric(optarg)) {
          fprintf(stderr, "Multi-diffusion port must be integer value.\n");
          return 1;
        }
        mdiff_port = atoi(optarg);
        break;
      case OPT_HOST_ADDR:
        hostname = strdup(optarg);
        break;
      case OPT_MDIF_ADDR:
        mdiff_ip = strdup(optarg);
        break;
      default:
        usage(argv[0]);
        return 1;
    }
  }

  switch (mode) {
    case CREA:
     if (mdiff_ip == NULL) {
        fprintf(stderr, "Multi diffusion ip is missing.\n");
        return 1;
     }
     else if (mdiff_port == 0) {
        fprintf(stderr, "Multi diffusion port is missing.\n");
        return 1;
     }
     else
       return create_ring2(mdiff_port, mdiff_ip);
    case JOIN:
     if (hostname == NULL) {
        fprintf(stderr, "Hostname is missing.\n");
        return 1;
     }
     else if (host_port == 0) {
        fprintf(stderr, "Host tcp port is missing.\n");
        return 1;
     }
     else
       return join2(hostname, host_port);
    default:
      fprintf(stderr, "No mode specified.\n");
      return 1;
  }

  return 0;
}
