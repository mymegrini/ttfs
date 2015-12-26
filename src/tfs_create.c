#include "error.h"
#include "block.h"
#include "utils.h"
#include "default.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define F_OWR 1    /***< flag for overwrite option */
#define D_NAME_MAXLEN 79     /***< disk name maximum length */

/**
 * This command creates and initializes a new disk.
 * It allocates enough memory for <size> blocks
 * and stores <size> in the first block
 *
 * @see C_FORMAT
 * @see D_OPEN_ERR
 * @see D_SEEK_ERR
 * @see EXIT_SUCCESS
 */

/**
 * @brief This function prints the command prototype
 * @param[in] argv0 command name
 * @param[out] out output stream
 * @return void
 * 
 * This function prints the command prototype to the
 * <out> stream
 */
void usage(char* argv0, FILE* out){
  fprintf(out, "Usage: %s [-o] -s <size> [<name>]\n", argv0);
}

/**
 * @brief This function creates and initializes a disk
 * @param[in] name containing path for the disk
 * @param[in] size of the disk
 * @param[in] argv0 name of the calling command
 * @return void
 *
 * This function creates a disk <name> of size <size>
 * and fills in the appropriate values in the description
 * block
 *
 * @see C_FORMAT
 * @see D_OPEN_ERR
 * @see D_SEEK_ERR
 * @see EXIT_SUCCESS
 */
void create_disk(char* name, int size, char* argv0){
  int disk;
  block b = new_block();   // block filled with 0 bytes

  if ((disk = open(name,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))==-1){
    printerror(argv0, D_OPEN_ERR);
    exit(D_OPEN_ERR);
  } else {    
    if (lseek(disk, (size-1)*B_SIZE, SEEK_SET)==-1){
      printerror(argv0, D_SEEK_ERR);
      exit(D_SEEK_ERR);
    }    
    if (write(disk, b, B_SIZE)==-1){
      printerror(argv0, D_OPEN_ERR);
      exit(D_OPEN_ERR);
    }
    
    wintle(size, b, 0);
    wintle(0, b, 4);
    
    if (lseek(disk, 0, SEEK_SET)==-1){
      printerror(argv0, D_SEEK_ERR);
      exit(D_SEEK_ERR);
    }
    if (write(disk, b, B_SIZE)==-1){
      printerror(argv0, D_OPEN_ERR);
      exit(D_OPEN_ERR);
    }    
    free(b);  //free block
    close(disk);
    printf("%s: %dB\n",name,size*B_SIZE);
    exit(EXIT_SUCCESS);
  }
}

/**
 * @brief This function parses command line inputs and
 * launches auxiliary functions
 * @param[in] argc number of command line inputs
 * @param[in] argv array of command line inputs
 * @return int value storing the error encountered
 *
 * This function parses command line inputs and options
 * then launches the auxiliary function create_disk to 
 * create the disk with the appropriate parameters
 *
 * @see create_disk
 * @see answer
 */
int main(int argc, char* argv[]){
  int c;
  int size = -1;
  int flags = 0;
  char name[D_NAME_MAXLEN+1];
  
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
      puts("This command creates and initializes a new disk");
      usage(argv[0], stdout);
      puts("  -s\t--size\t\tspecify size of disk (requires strictly positive value)\n \
  -o\t--overwrite\toverwrite existing disk\n\
    \t--help\t\tdisplay this help and exit\n");
      exit(EXIT_SUCCESS);
    case 's':
      if (optarg != NULL) {
	if(atoi(optarg)<1){
	  fprintf(stderr,
		  "%s: requires strictly positive <size> value\n",
		  argv[0]);
	  usage(argv[0], stderr);
	  exit(C_FORMAT);
	} else
	size = (size==-1) ? atoi(optarg) : size;
	break;
      } else {
	usage(argv[0], stderr);
	exit(C_FORMAT);
      }
    case 'o':
      flags |= F_OWR;
      break;
    default: /* '?' */
      usage(argv[0], stderr);
      fprintf(stderr, "Usage: %s [-o] -s <size> [<name>]\n",argv[0]);
      exit(C_FORMAT);
    }
  }
  
  if(size<1){
    fprintf(stderr,
	    "%s: strictly positive <size> value required\n",
	    argv[0]);
    usage(argv[0], stderr);
    exit(C_FORMAT);
  }
  
  if(optind < argc){
    if(strncmp(argv[optind]+strlen(argv[optind])-4, DEF_EXT, 4)==0)
      strncpy(name, argv[optind], D_NAME_MAXLEN);
    else {      
      strncpy(name, argv[optind], D_NAME_MAXLEN-4);
      strncat(name, ".tfs", D_NAME_MAXLEN-strlen(name));
    }
  } else strncpy(name, DEF_FILENAME, 9);

  if ((flags & F_OWR)==0 && access(name, F_OK)==0){
    printf("%s: disk '%s' already exists.\n",
	   argv[0], name);
    if (answer("Overwrite? [Y/n] ")==0) exit(EXIT_SUCCESS);
  }
  create_disk(name, size, argv[0]);
}
