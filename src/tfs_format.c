#include "error.h"
#include "block.h"
#include "ll.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEF_NAME "disk.tfs"   /***< default disk name */
#define F_OWR 1    /***< flag for overwrite option */
#define MAGIC_NUMBER 0x31534654 /***< magic number */
#define D_NAME_MAXLEN 79     /***< disk name maximum length */

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
 * @see P_WRONGIDX
 * @see EXIT_SUCCESS
 */
int main(int argc, char* argv[]){
  int c;
  int partition = -1;
  int filecount = -1;
  char name[D_NAME_MAXLEN+1];
  int flags = 0;
  
  while (1) {
    int index = 0;
    static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"partition",  required_argument, NULL, 'p'},
      {"mf",  required_argument, NULL, 'f'},
      {"overwrite",  no_argument, NULL, 'o'},
      {0, 0, 0, 0}
    };
    c = getopt_long_only(argc, argv, "p:f:o",
			 long_options, &index);
    
    if (c==-1) break;
    
    switch (c) {
    case 'h':
      printf("This command creates a minimal filesystem in an \
existing partition\n\
Usage: %s -p <partition> -mf <max-file-count> [<name>]\n\n\
  -p\t--partition\tspecify partition id\n\
  -f\t-mf\t\tspecify maximum file count for the file system\n\
  -o\t--overwrite\toverwrite existing disk\n\
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
    case 'o':
      flags |= F_OWR;
      break;
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

  if (partition<0 || filecount<1){
    fprintf(stderr,
	    "%s: requires positive <partition> and strictly positive <max-file-count> values\n\
Usage: %s -p <partition> -mf <max-file-count> [<name>]\n"
	    ,argv[0],argv[0]);
    exit(C_FORMAT);
  } else {
    disk_id id;
    error err;
    d_stat stat;
    
    err = start_disk(name, &id);    
    if(err!=EXIT_SUCCESS){
      printerror("start_disk", err);
      exit(err);
    }

    err = disk_stat(id, &stat);    
    if(err!=EXIT_SUCCESS){
      printerror("disk_stat", err);
      exit(err);
    }

    if (partition<0 || partition >D_PARTMAX-1 || partition>stat.npart-1){
      printerror(argv[0], P_WRONGIDX);
      exit(P_WRONGIDX);
    } else {
      uint32_t psize;
      uint32_t pidx =1;
      block b = new_block();
      uint32_t k;
      
      psize = stat.part[partition];
      for(k=0; k<partition; k++) pidx+=stat.part[k];

      err = read_block(id, b, pidx);    
      if(err!=EXIT_SUCCESS){
	printerror("read_block", err);
	exit(err);
      }

      err = rintle(&k, b, 0);    
      if(err!=EXIT_SUCCESS){
	printerror("rintle 0", err);
	exit(err);
      }

      if((flags & F_OWR)==0 && k == 0x31534654){
	char answer = 0;
	while(answer == 0){
	  printf("%s: partition %d already contains a filesystem. Overwrite? [Y/n] ",
		 argv[0], partition);
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

      free(b);
      b=new_block();

      err = wrintle(MAGIC_NUMBER, b, 0);    
      if(err!=EXIT_SUCCESS){
	printerror("wintle 0", err);
	exit(err);
      }

      err = wrintle(B_SIZE, b, 4);    
      if(err!=EXIT_SUCCESS){
	printerror("wintle 4", err);
	exit(err);
      }
      
      err = wrintle(psize, b, 8);    
      if(err!=EXIT_SUCCESS){
	printerror("wintle 0", err);
	exit(err);
      }

      
      err = wrintle(psize, b, 8);    
      if(err!=EXIT_SUCCESS){
	printerror("wintle 0", err);
	exit(err);
      }

    }    
  }
}
