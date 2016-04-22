#include "../plugin_system/plugin_interface.h"

#include <stdio.h>
#include <stdlib.h>


int cmd_zyva(int argc, char **argv);

PluginCommand_t pcmd_zyva = {
  "zyva",
  "Zyva koi!",
  cmd_zyva
};

Plugin plug_ziva = {
  1,
  &pcmd_zyva,
  0,
  NULL
};


int init_zyva(PluginManager *p)
{
  return plugin_register(p, "zyva", &plug_ziva);
}




int cmd_zyva(int argc, char **argv) {
  printf("Zyva la mouche qui pête !\n");
  return 0;
}

