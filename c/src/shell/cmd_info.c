#include <stdio.h>
#include "../protocol/protocol.h"
#include "../protocol/message.h"
#include "../protocol/common.h"

#define   ORING    'r'
#define   OLRING   "ring"

void print_ringinfo(int ring) {
    printf(UNDERLINED BOLD "Ring %d:\n" RESET
            UNDERLINED "multidiffusion:" RESET " %16s %4d\n"
            UNDERLINED "next entity:" RESET "%16s %4d\n",
            ring, ent->mdiff_ip[ring], ent->mdiff_port[ring], 
            ent->ip_next[ring], ent->port_next[ring]);
}

void print_selfinfo() {
    printf(UNDERLINED BOLD "Personnal:\n" RESET
            UNDERLINED "ip:" RESET " %16s\n"
            UNDERLINED "ring communication:" RESET " %4d\n"
            UNDERLINED "tcp server:" RESET " %4d\n",
            ent->ip_self, ent->udp, ent->tcp);
}

void print_ringnumber() {
    printf(UNDERLINED BOLD "Ring number:" RESET " %d\n", *ring_number+1);
}

void cmd_info(int argc, char *argv[]) {
    print_selfinfo();
    print_ringnumber();
    for (int i = 0; i < *ring_number+1; ++i)
        print_ringinfo(i);
}
