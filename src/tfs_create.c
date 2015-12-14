#include "error.h"
#include "block.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define DEF_NAME "disk.tfs"   /***< default disk name */
#define F_OWR 1    /***< flag for overwrite option */

#ifndef D_NAME_MAXLEN
#define D_NAME_MAXLEN 81     /***< disk name maximum length */
#endif

/**
 * @brief This command creates and initializes a new disk
 * @param[in] name disk name
 * @param[in] size disk size
 * @param[out] id wheere disk id is stored
 * @return Returns an error if encountered
 *
 * This command creates and initializes a new disk.
 * It allocates enough memory for <size> blocks
 * and stores <size> in the first block
 *
 * @see C_FORMAT
 * @see D_OPEN_ERR
 * @see D_SEEK_ERR
 * @see EXIT_SUCCESS
 */

int main(int argc, char* argv[]){
   int c;
  int size = -1;
  int flags = 0;
  char name[D_NAME_MAXLEN+1];
  int disk;
  block b = new_block();   // block filled with 0 bytes
  
  while (1) {
    int index = 0;
    static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"size",  required_argument, NULL, 's'},
      {"overwrite",  no_argument, NULL, 'o'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "s:o",
                 long_options, &index);
    
    if (c==-1) break;

    switch (c) {
    case 'h':
      printf("This command creates and initializes a new disk\n\
Usage: %s [-o] -s <size> [<name>]\n\n\
  -s\t--size\t\tspecify size of disk (requires strictly positive value)\n\
  -o\t--overwrite\toverwrite existing disk\n\
    \t--help\t\tdisplay this help and exit\n\n",
	     argv[0]);
      exit(EXIT_SUCCESS);
    case 's':
      if (optarg != NULL) {
	if(atoi(optarg)<1){
	  fprintf(stderr,
		  "%s: requires strictly positive <size> value\n\
Usage: %s [-o] -s <size> [<name>]\n",
		  argv[0], argv[0]);
	  exit(C_FORMAT);
	} else
	size = (size==-1) ? atoi(optarg) : size;
	break;
      } else {
	fprintf(stderr, "Usage: %s [-o] -s <size> [<name>]\n",argv[0]);
	exit(C_FORMAT);
      }
    case 'o':
      flags |= F_OWR;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-o] -s <size> [<name>]\n",argv[0]);
      exit(C_FORMAT);
    }
  }
  
  if(size<1){
    fprintf(stderr,
	    "%s: strictly positive <size> value required\n\
Usage: %s [-o] -s <size> [<name>]\n",
	    argv[0], argv[0]);
    exit(C_FORMAT);
  }
  
  if(optind < argc){
    if(strncmp(argv[optind]+strlen(argv[optind])-4, ".tfs", 4)==0)
      strncpy(name, argv[optind], D_NAME_MAXLEN);
    else {      
      strncpy(name, argv[optind], D_NAME_MAXLEN-4);
      strncat(name, ".tfs", D_NAME_MAXLEN-strlen(name));
    }
  } else strncpy(name, DEF_NAME, 9);

  if ((flags & F_OWR)==0 && access(name, F_OK)==0){
    char answer = 0;
    while(answer == 0){
      printf("%s: disk '%s' already exists. Overwrite? [Y/n] ",
	     argv[0], name);
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
  
  if ((disk = open(name,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))==-1){
    printerror(argv[0], D_OPEN_ERR);
    exit(D_OPEN_ERR);
  } else {    
    if (lseek(disk, (size-1)*B_SIZE, SEEK_SET)==-1){
      printerror(argv[0], D_SEEK_ERR);
      exit(D_SEEK_ERR);
    }    
    if (write(disk, b, B_SIZE)==-1){
	printerror(argv[0], D_OPEN_ERR);
	exit(D_OPEN_ERR);
    }
    
    wintle(size, b, 0);
    wintle(0, b, 4);
    
    if (lseek(disk, 0, SEEK_SET)==-1){
      printerror(argv[0], D_SEEK_ERR);
      exit(D_SEEK_ERR);
    }
    if (write(disk, b, B_SIZE)==-1){
	printerror(argv[0], D_OPEN_ERR);
	exit(D_OPEN_ERR);
    }    
    free(b);  //free block
    close(disk);
    printf("%s: %dB\n",name,size*B_SIZE);
    exit(EXIT_SUCCESS);
  }
}
