#include "error.h"
#include "block.h"
#include "ll.h"
#include <getopt.h>

#define DEF_NAME "disk.tfs"   /***< default disk name */

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
  int partition = -1;
  int filecount = -1;
  char name[D_NAME_MAXLEN+1];
  
  while (1) {
    int index = 0;
    static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"partition",  required_argument, NULL, 'p'},
      {"mf",  required_argument, NULL, 'f'},
      {0, 0, 0, 0}
    };
    c = getopt_long_only(argc, argv, "p:f:",
			 long_options, &index);
    
    if (c==-1) break;
    
    switch (c) {
    case 'h':
      printf("This command creates a minimal filesystem on an\n\
existing partition\n					       \
Usage: %s -p <partition> -mf <max-file-count> [<name>]\n\n     \
  -p\t--partition\tspecify partition id\n		       \
  -f\t-mf\t\tspecify maximum file count for the file system\n  \
    \t--help\t\tdisplay this help and exit\n\n",
	     argv[0]);
      exit(EXIT_SUCCESS);
    case 'p':
      if (optarg != NULL){
	partition = (partition==-1) ? atoi(optarg) : partition;
	break;
      } else {
	fprintf(stderr, "Usage: %s -p <partition> -mf <max-file-count> [<name>]\n",argv[0]);
	exit(C_FORMAT);
      }     
    case 'f':
      if (optarg != NULL){
	filecount = (filecount==-1) ? atoi(optarg) : filecount;
	break;
      } else {
	fprintf(stderr, "Usage: %s -p <partition> -mf <max-file-count> [<name>]\n",argv[0]);
	exit(C_FORMAT);
      }
    default: /* '?' */
      fprintf(stderr, "Usage: %s -p <partition> -mf <max-file-count> [<name>]\n",argv[0]);
      exit(C_FORMAT);
    }
  }
  
  if(optind < argc){
    if(strncmp(argv[optind]+strlen(argv[optind])-4, ".tfs", 4)==0)
      strncpy(name, argv[optind], D_NAME_MAXLEN);
    else {      
      strncpy(name, argv[optind], D_NAME_MAXLEN-4);
      strncat(name, ".tfs", D_NAME_MAXLEN-strlen(name));
    }
  } else strncpy(name, DEF_NAME, 9);

  if (partition==-1 || filecount==-1){
    fprintf(stderr, "Usage: %s -p <partition> -mf <max-file-count> [<name>]\n",argv[0]);
    exit(C_FORMAT);
  } else {
    disk_id id;
    block b;
    error err;

    err = start_disk(name, &id);
    
    if(err!=EXIT_SUCCESS){
      printerror("start_disk", err);
      exit(err);
    }

    b= new_block();
    
  }  
}
