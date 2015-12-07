#include "error.h"
#include "ll.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DEF_PATH "../dev/"    /***< preferred disk location */
#define DEF_NAME "disk"   /***< default disk name */
#define DEF_EXT ".tfs"  /***< preferred disk location */

#define F_PATH 1     /***< flag for default disk location */
#define F_NAME 2    /***< flag for default disk name */
#define F_EXT 4     /***< flag for default disk extention */

/**
 * Creates a disk of specified size
 * under an optinally specified name
 *
 * \param size size of the disk 
 * \param name name of the disk if specified
 * otherwise "../dev/disk.tfs" is default name
 * \return error non-null value indicates failure 
 */

int main(int argc, char* argv[]){
  int opt;
  int size = 1;
  int flags = 0;
  char name[D_NAME_MAXLEN+1];
  char* argn;
  error r;
  
  
  while ((opt = getopt(argc, argv, "s:d")) != -1) {
    switch (opt) {
    case 's':
      size = atoi(optarg);
      break;
    case 'd':
      flags|=F_PATH;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s -s size [[-d] name]\n",argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  
  if(optind >= argc) {
    flags|= F_NAME;
    argn=DEF_NAME;
  }
  else argn=argv[optind];

  if (strncmp(argn+(strlen(argn)-4), DEF_EXT, 4)==0) flags|= F_EXT;

  if ((flags & F_PATH) !=0) strncpy(name, DEF_PATH, D_NAME_MAXLEN+1);

  strncat(name, argn, D_NAME_MAXLEN-strlen(name));
  
  if ((flags & F_EXT) == 0) strncat(name, DEF_EXT, D_NAME_MAXLEN-strlen(name));
  
  if ((r = init_disk(name,size)) != EXIT_SUCCESS) {printerror("init_disk"); return r;}
  else return r;
}
