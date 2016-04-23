#include "../plugin_system/plugin_interface.h"
#include "../plugin_system/protocol_interface.h"

#include <stdio.h>
#include <stdlib.h>


static int cmd_zyva(int argc, char **argv);
static int action_zyva(char *message, char *content, int lookup_flag);

#define ZYVA_TYPE "ZYVAKOI!"

PluginCommand_t pcmd_zyva = {
  "zyva",
  "Zyva koi!",
  cmd_zyva
};


PluginAction_t paction_zyva = {
  ZYVA_TYPE,
  "Zyva, koikeskya???",
  &action_zyva
};


Plugin plug_zyva = {
  1,
  &pcmd_zyva,
  1,
  &paction_zyva
};


int init_zyva(PluginManager *p)
{
  return plugin_register(p, "zyva", &plug_zyva);
}




static int cmd_zyva(int argc, char **argv) 
{
  printf("Zyva la mouche qui pète !\n");
  /* send_message(ZYVA_TYPE, "Zyva la mouche qui pète !\n"); */
  send_message(ZYVA_TYPE, "Zyva la mouche qui pète !\n");
  return 0;
}

static int action_zyva(char *message, char *content, int lookup_flag)
{
  printf("someone said: %s\n", content);
  printf("KOI KESKYA?!\n");
  if (!lookup_flag)
    /* retransmit(message); */
    retransmit(message);
  return 0;
}
