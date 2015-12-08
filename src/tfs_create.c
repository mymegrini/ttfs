#include "error.h"
#include "block.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define DEF_PATH "dev/"    /***< preferred disk location */
#define DEF_NAME "disk"   /***< default disk name */
#define DEF_EXT ".tfs"  /***< preferred disk location */
#define F_PATH 1     /***< flag for default disk location */
#define F_NAME 2    /***< flag for default disk name */
#define F_EXT 4     /***< flag for default disk extention */

#ifndef D_NAME_MAXLEN
#define D_NAME_MAXLEN 79     /***< disk name maximum length */
#endif

/**
 * @brief This function creates and initializes a new disk
 * @param[in] name disk name
 * @param[in] size disk size
 * @param[out] id wheere disk id is stored
 * @return Returns an error if encountered
 *
 * This command creates and initializes a new disk.
 * It allocates enough memory for size blocks
 * and puts size in the block 0
 *
 * @see C_FORMAT
 * @see D_OPEN_ERR
 * @see D_SEEK_ERR
 * @see EXIT_SUCCESS
 */

int main(int argc, char* argv[]){
  int opt;
  int size = 0;
  int flags = 0;
  char name[D_NAME_MAXLEN+1];
  char* argn;
  int disk;
  block b;
  
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
      exit(C_FORMAT);
    }
  }

  if(size==0){
    fprintf(stderr, "%s: non-null size value required\nUsage: %s -s size [[-d] name]\n",argv[0],argv[0]);
    exit(C_FORMAT);
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

  if (access(name, F_OK)==0){
    char answer = 0;
    while(answer == 0){
      printf("tfscreate: disk %s already exists. Overwrite? [Y/n] ", name);
      answer = getchar();
      switch(answer){
      case 'Y':	
      case 'y':
	break;
      case 'n':
      case 'N':
	exit(EXIT_SUCCESS);
      default:
	answer = 0;
      }
    }
  }

  puts(name);
  
  if ((disk = open(name,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))==-1){
    printerror("tfscreate", D_OPEN_ERR);
    exit(D_OPEN_ERR);
  } else {
    wintle(size, b, 0);
    wintle(0, b, 1);
    
    if (write(disk, &b, B_SIZE)==-1){
	printerror("tfscreate", D_OPEN_ERR);
	exit(D_OPEN_ERR);
    }
      
    if (lseek(disk, (size-1)*B_SIZE, SEEK_SET)==-1){
      printerror("tfscreate", D_SEEK_ERR);
      exit(D_SEEK_ERR);
    }
    
    if (write(disk, &b, B_SIZE)==-1){
	printerror("tfscreate", D_OPEN_ERR);
	exit(D_OPEN_ERR);
    }
    close(disk);
    exit(EXIT_SUCCESS);
  }
}
