#include "error.h"
#include "block.h"
#include <getopt.h>

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
 * @brief This command creates a minimal filesystem
 * on an existing partition
 * @param[in] partition
 * @param[in] max-file-count
 * @return prints name of disk and its size
 *
 * This command creates a minimal filesystem on the
 * <partition> partition which supports a maximum of
 * <max-file-count> files
 *
 * @see C_FORMAT
 * @see EXIT_SUCCESS
 */
int main(int argc, char* argv[]){
  int c;
  int size = -1;
  int flags = 0;
  char name[D_NAME_MAXLEN+1];
  char* argn;
  int disk;
  block b = new_block();   // block filled with 0 bytes
  
  while (1) {
    int index = 0;
    static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"partition",  required_argument, NULL, 'p'},
      {"mf",  required_argument, NULL, 'f'},
      {"max-file-count",  required_argument, NULL, 'f'},
      {"dev",  no_argument, NULL, 'd'},
      {0, 0, 0, 0}
    };
    c = getopt_long_only(argc, argv, "p:f:d",
                 long_options, &index);
    
    if (c==-1) break;

    switch (c) {
    case 'h':
      printf("This command creates a minimal\
filesystem on an existing partition\n\
Usage: %s [--help] -p <partition> -mf <max-file-count>[[-d] <name>]\n\n\
  -p\t--partition\t\tspecify size of disk (strictly positive integer required)\n\
  -f\t-mf\t\tstore disk in 'dev' folder\n\
    \t--max-file-count
  -d\t--dev\t\tread disk from 'dev/' folder\n\
    \t--help\t\tdisplay this help and exit\n\n",
	     argv[0]);
      exit(EXIT_SUCCESS);
    case 's':
      if (size==-1 && optarg != NULL) {
	if(atoi(optarg)<1){
	  fprintf(stderr,
		  "%s: strictly positive <size> value required\n\
Usage: %s [--help] -s <size> [[-do] <name>]\n",
		  argv[0],argv[0]);
	  exit(C_FORMAT);
	}
	size = (size==-1 && atoi(optarg)>0) ? atoi(optarg) : size;
	break;
      } else {
	fprintf(stderr, "Usage: %s [--help] -s <size> [[-do] <name>]\n",argv[0]);
	exit(C_FORMAT);
      }     
    case 'd':
      flags |= F_PATH;
      break;
    case 'o':
      flags |= F_OWR;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [--help] -s <size> [[-do] <name>]\n",argv[0]);
      exit(C_FORMAT);
    }
  }
  
  if(size<1){
    fprintf(stderr,
	    "%s: strictly positive <size> value required\n\
Usage: %s [--help] -s <size> [[-do] <name>]\n",
	    argv[0],argv[0]);
    exit(C_FORMAT);
  }
  
  if(optind >= argc) {
    flags|= F_NAME;
    argn=DEF_NAME;
  }
  else argn=argv[optind];

  if (strncmp(argn+(strlen(argn)-4), DEF_EXT, 4)==0) flags|= F_EXT;

  if ((flags & F_PATH) !=0) strncpy(name, DEF_PATH, D_NAME_MAXLEN+1);

  strncat(name, argn, D_NAME_MAXLEN-strlen(name)-4);
  
  if ((flags & F_EXT) == 0) strncat(name, DEF_EXT, D_NAME_MAXLEN-strlen(name));

  if ((flags & F_OWR)== 0 && access(name, F_OK)==0){
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
