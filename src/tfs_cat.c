#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>

#include "tfs.h"
#include "tfsll.h"
#include "utils.h"
#include "block.h"

#define OPT_HELP 'h'
#define OPT_HELP_LONG "help"

#define OPT_STRING          "h:"

#define PATH_FORMAT -1
#define PATH_MISSING -2

struct option options[2] = {
  { OPT_HELP_LONG, no_argument, 0, OPT_HELP },
  { 0, 0, 0, 0 }
};


void usage (char *argv0){
  printf("Usage :\t%s name\n",argv0);
}


void help (char *argv0) {
  usage(argv0);
  printf("%s concatenate files and print on the standard output.\n"
	 "Options :\n"
	 "-%c\t--%s\tPrint this message.\n",
	 argv0,
	 OPT_HELP, OPT_HELP_LONG);
}

error cat (char *path) {

  int fd = tfs_open(path,O_RDONLY);
  char *buf;
  int s;
  while((s = tfs_read(fd,buf,100)) != 0){
    if(s == -1)
      return ERR_TFS_READ;
    printf("%s",buf);
  }
  
  return tfs_close(fd);
}


int main(int argc, char *argv[] ){

  char c;
  char *path;
  while ((c = getopt_long(argc, argv, OPT_STRING, options, 0)) != -1){
    switch (c){
    case OPT_HELP:
      help(argv[0]);
      return EXIT_SUCCESS;
      
    default:
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }
  if (optind == argc) {
    fputs("Argument path missing.", stderr);
    usage(argv[0]);
    return PATH_MISSING;
  }
  path = argv[optind];

  return cat(path);
}
