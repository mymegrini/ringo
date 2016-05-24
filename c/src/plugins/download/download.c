#include "../../common.h"
#include "../../plugin_system/plugin_programmer_interface.h"
/* #include "../../list.h" */

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <getopt.h>



#define   TRANS_APPID            "TRANS###"
#define   TRANS_CONTENT_MAX      463
#define   TRANS_NUM_MESS(size)   \
  (size / TRANS_CONTENT_MAX + (size % TRANS_CONTENT_MAX == 0 ? 0 : 1))


typedef struct transfert_data {
  char *filename;
  /* FILE *f; */
  int fd;
  long nummess;
  long nextnum;
} transfert_data;


typedef struct request_data {
  int nrequest;
} request_data;


static int request_file(const char *filename);
static int action_sen(const char *message, const char *content, int lookup_flag);
static int action_rok(const char *message, const char *content, int lookup_flag);
static int action_req(const char *message, const char *content, int lookup_flag);
/* static int begin_transfert(FILE *f, const char *filename, const char *size_filename, */
static int begin_transfert(int fd, const char *filename, const char *size_filename,
    const char *id_trans);
static void free_transfert_data(transfert_data *t);
static void usage(char *argv0);
static void help(char *argv0);
static int ltole(char *le, long l, int size);
static long letol(const char *le);


static list requested;
static list transfert;



#define   OPT_HELP    'h'
#define   OPTL_HELP   "help"

#define   OPT_STRING  "h"

static struct option longopts[] = {
  {OPTL_HELP,   no_argument,         0,   OPT_HELP},
  {0,           0,                   0,   0}
};



int cmd_trans(int argc, char **argv)
{
  if (argc == 1) {
    usage(argv[0]);
    return 0;
  }
  int c, indexptr;
  optind = 0;
  while ((c = getopt_long(argc, argv, OPT_STRING,
          longopts, &indexptr)) != -1) {
    switch (c) {
      case OPT_HELP:
        help(argv[0]);
        return 0;
      default:
        usage(argv[0]);
        return 1;
    }
  }
  for (int i = 1; i < argc; ++i) {
    request_file(argv[i]);
  }
  return 0;
}



int action_trans(const char *message, const char *content, int lookup_flag)
{
  if (strncmp(content, "REQ ", 4) == 0)
    return action_req(message, content + 4, lookup_flag);
  else if (strncmp(content, "ROK ", 4) == 0)
    return action_rok(message, content + 4, lookup_flag);
  else if (strncmp(content, "SEN ", 4) == 0)
    return action_sen(message, content + 4, lookup_flag);
  else {
    debug("action_trans", "Message trans not following the protocol: %s", content);
    return 1;
  }
}


static int ltole(char *le, long l, int size)
{
  for (int i = 0; i < size; ++i)
  {
    le[i] = '0' + l % 10;
    l /= 10;
  }
  return l == 0;
}



static long letol(const char *le)
{
  long l = 0;
  long exp = 1;
  while (isdigit(*le)) {
    l += exp * (*le - '0');
    exp *= 10;
    ++le;
  }
  return l;
}

static int request_file(const char *filename)
{
  request_data *r = malloc(sizeof(request_data));
  r->nrequest = get_ring_number()+1;
  if (!insert_one(requested, filename, r)) {
    fprintf(stderr, "File %s already requested.\n", filename);
    return 0;
  }
  char fname_size[3];
  itoa(fname_size, 2, strlen(filename));
  send_message(TRANS_APPID, "REQ %s %s", fname_size, filename);
  return 1;
}



static int action_req(const char *message, const char *content, int lookup_flag)
{
  // Protocol test
  if (content[2] != ' ' || !isnumericn(content, 2)) {
    return 1;
  }
  unsigned short size = atoi(content);
  // Message already seen
  if (lookup_flag) {  // file already requested
    request_data *r;
    // test if a request for this message exists
#ifdef DEBUG
    if (!
#endif
    findn((void**)&r, requested, content, size)
#ifdef DEBUG
    ) {
      debug("action_req", "req message already seen but not found in the requested list:\n%s",
          content);
      return 1;
    }
#endif
    ;
    // if the request exist decrement the request counter
    // if the counter is at 0, remove the file and free allocated ressources
    if (--r->nrequest == 0) {
      char *filename = strndup(content+3, size);
      rm(requested, filename);
      verbose(RED "File %s not found on any rings. Transfer aborted.\n", filename);
      free(filename);
      return 0;
    }
  }
  // Message not seen
  else {
    // test if the file is present
    char *filename = malloc(size);
    strncpy(filename, content + 3, size);
    filename[size] = 0;
    int fd = open(filename, O_RDONLY);
    /* if (f) {  // File accessible */
    if (fd != -1) {  // File accessible
      char id_trans[9];
      strncpy(id_trans, message+5, 8);
      id_trans[8] = 0;
      char filename_size[3];
      strncpy(filename_size, content, 2);
      filename_size[2] = 0;
      begin_transfert(fd, filename, filename_size, id_trans);
    }
    else {
      retransmit(message);
    }

    free(filename);
    return 0;
  }
  debug("action_req", "reaching end of function, all test passed");
  return 0;
}



static int action_rok(const char *message, const char *content, int lookup_flag)
{
  const char *id_trans = content, *filename_size = content + 9, *filename = content + 12;
  if (id_trans[8] != ' ' || filename_size[2] != ' ' || !isnumericn(filename_size, 2)) {
    return 1;
  }
  unsigned short fname_size = atoi(filename_size);
  const char *nummess = filename + fname_size + 1;
  if (filename[fname_size] != ' ' || !isnumericn(nummess, 8)) {
    return 1;
  }
  
  int r = 0;
  // look for an eventual request...
  if (rmn(requested, filename, fname_size)) {  // request found and deleted from requested
    // add transfert structure
    char *fname = strndup(filename, fname_size);
    // OVERWRITE EXISTING FILE
    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd != -1) {
      transfert_data *t = malloc(sizeof(transfert_data));
      t->fd = fd;
      t->filename = fname;
      t->nummess  = letol(nummess);
      t->nextnum  = 0;
      char *idt = strndup(id_trans, 8);
      insert_noalloc(transfert, idt, t);
    }
    else {
      fprintf(stderr, "An error occurs while creating file %s, transfert aborted.\n", fname);
      r = 1;
      free(fname);
    }
    return r;
  }
  return 0;
}



static int action_sen(const char *message, const char *content, int lookup_flag)
{
  if (lookup_flag) {
    return 0;
  }
  const char *id_trans = content, *nomess = id_trans + 9, *size_content = nomess + 9,
       *fcontent = size_content + 4;
  if ( id_trans[8] != ' ' || nomess[8] != ' ' || size_content[3] != ' ' ||
      !isnumericn(nomess, 8) || !isnumericn(size_content, 3)) {
    return 1;
  }
  transfert_data *t;
  // search for an existing transfert...
  if (findn((void **)&t, transfert, id_trans, 8)) {
    long n = letol(nomess);
    if (n == t->nextnum) {
      int size = atoi(size_content);
      /* fwrite(fcontent, size, 1, t->f); */
      /* fflush(t->f); */
      if (write(t->fd, fcontent, size) == -1)
	  perror("plugin/download/ action_sen");
      if (++t->nextnum == t->nummess) {
        char *filename = strdup(t->filename);
        free_transfert_data(t);
        rmn(transfert, id_trans, 8);
        verbose(BOLD UNDERLINED "File %s downloaded.\n" RESET, filename);
        free(filename);
      }
    }
    else {
      free_transfert_data(t);
      rmn(transfert, id_trans, 8);
      verbose(RED "Transfert of %s aborted because a packet is missing.\n" RESET);
      return 1;
    }
  } 
  else {
    retransmit(message);
  }
  return 0;
}


static int begin_transfert(int fd, const char *filename, const char *size_filename,
    const char *id_trans)
{
  long size = lseek(fd, 0, SEEK_END);
  long nummess = TRANS_NUM_MESS(size);
  char nummess_str[9];
  ltole(nummess_str, nummess, 8);
  nummess_str[8] = 0;
  send_message(TRANS_APPID, "ROK %s %s %s %s", id_trans, size_filename,
      filename, nummess_str);

  // beginning transfert
  lseek(fd, 0, SEEK_SET);
  for (long i = 0; i < nummess; ++i) {
    sleep(1);
    char buff[465];
    ssize_t r = read(fd, buff, 464);
    if (r != -1) {
      buff[r] = 0;
      char no[9];
      ltole(no, i, 8);
      no[8] = 0;
      char sc[4];
      itoa(sc, 3, r);
      sc[3] = 0;
      send_message(TRANS_APPID, "SEN %s %s %s %s", id_trans, no, sc, buff); 
    }
    else {
      debug("begin_transfert", "read error, ret: %ld", r);
      return -1;
    }
  }
  return 0;
}


static void free_transfert_data(transfert_data *t) 
{
  close(t->fd);
  free(t->filename);
  free(t);
}




static void usage(char *argv0)
{
  printf("Usage:\t%s [-h] <filename1> <filename2> ...\n", argv0);
}



static void help(char *argv0)
{
  usage(argv0);
}




////////////////////////////////////////////////////////////////////////////////
// PLUGIN DATA
////////////////////////////////////////////////////////////////////////////////

static void close_plugin();



PluginCommand_t pcmd_download = {
  "downl",
  "Download files.",
  cmd_trans 
};

PluginAction_t paction_download = {
  TRANS_APPID,
  "Download files plugin.",
  &action_trans
};



Plugin plug_download = {
  1,
  &pcmd_download,
  1,
  &paction_download,
  close_plugin
};



int init_download(PluginManager *p)
{
  requested = new_list();
  transfert = new_list(); 
  return plugin_register(p, "download", &plug_download);
}



static void close_plugin()
{
  return;
}

//// END OF PLUGIN DATA
