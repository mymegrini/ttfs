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
 * @see P_CORRUPTED
 * @see EXIT_SUCCESS
 */


/**
 * @brief This function waits for an answer from the user
 * @param[in] prompt message for the user
 * @return Returns 0 for [n/N] and 1 for [y/Y]
 *
 * This function displays a prompt message <prompt> and
 * waits for the user's answer and returns it
 */
int answer(char* prompt){
  printf("%s", prompt);
  switch(getchar()){
  case 'Y':	
  case 'y':
    return 1;
  case 'n':
  case 'N':
    return 0;
  default:
    while(getchar() != '\n');
    return answer(prompt);
  }
}


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
  fprintf(out,
	  "Usage: %s -p <partition> -mf <max-file-count> [<name>]\n",
	  argv0);
}



void init_sblock(int id, uint32_t pidx, uint32_t psize, int filecount){
  block b = new_block();  
  testerror("wintle magic number", wintle(MAGIC_NUMBER, b, 0*INT_SIZE));  
  testerror("wintle block size", wintle(B_SIZE, b, 1*INT_SIZE));
  testerror("wintle part size", wintle(psize, b, 2*INT_SIZE)); 
  testerror("wintle free block count", wintle(psize-(filecount/16+3), b, 3*INT_SIZE));    
  testerror("wintle first free block", wintle(filecount/16+2, b, 4*INT_SIZE));
  testerror("wintle max file count", wintle(filecount, b, 5*INT_SIZE));
  testerror("wintle free file count", wintle(filecount-1, b, 6*INT_SIZE));
  testerror("wintle first free file", wintle(1, b, 7*INT_SIZE));
  testerror("write_block 0", write_block(id, b, pidx));
  free(b);
}

void init_ftab(int id, uint32_t pidx, uint32_t psize, int filecount){

}

void init_root(int id, uint32_t ridx){

}

void init_fblocks(int id, uint32_t pidx, uint32_t psize, int filecount){

}

void format_partition(char* name, int partition, int filecount, char* argv0, int flags){
  disk_id id;
  d_stat stat;
  
  testerror("start_disk", start_disk(name, &id));  
  testerror("disk_stat", disk_stat(id, &stat));
  
  if (partition<0 || partition >D_PARTMAX-1 || partition>stat.npart-1){
    printerror(argv0, P_WRONGIDX);
    exit(P_WRONGIDX);
  } else {
    uint32_t psize;
    uint32_t pidx =1;
    block b;
    uint32_t k;
    
    psize = stat.part[partition];      
    if(psize<(filecount/16)+3){
      printf("%s: partition too small for specified <max-file-count> value\n"
	     ,argv0);
      exit(C_FORMAT);	
    }
    
    for(k=0; k<partition; k++) pidx+=stat.part[k];
    
    if(pidx+psize>stat.size-1){
      printerror(argv0,P_CORRUPTED);
      exit(P_CORRUPTED);
    }
    
    b= new_block();
    testerror("read_block 0", read_block(id, b, pidx));
    testerror("rintle 0", rintle(&k, b, 0));
    
    if((flags & F_OWR)==0 && k == 0x31534654){
      printf("%s: partition %d already contains a filesystem.\n",
	     argv0, partition);
      if (answer("Overwrite? [Y/n]")==0)
	exit(EXIT_SUCCESS);
      else {
	free(b);
		
	puts("Initializing superblock...");
	init_sblock(id, pidx, psize, filecount);
	puts("Done.");
	
	puts("Creating file table...");
	init_ftab(id, pidx, psize, filecount);
	puts("Done.");

	puts("Creating root directory...");
	init_root(id, pidx+filecount/16+2);
	puts("Done.");

	puts("Formatting volume..."); 
	init_fblocks(id, pidx, psize, filecount);
	puts("Done.");

	printf("%s : Formatting successfully completed.\n\
Disk: %s (%d)\nPartition: %d (%d)\nTTFS max file count = %d\n",
	       argv0, name, stat.size, partition, psize, filecount);

      }
    }
  }
}


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
      puts("This command creates a minimal filesystem in an \
existing partition");
      usage(argv[0], stdout);
      puts("\
  -p\t--partition\tspecify partition id\n\
  -f\t-mf\t\tspecify maximum file count for the file system\n\
  -o\t--overwrite\toverwrite existing partition\n\
    \t--help\t\tdisplay this help and exit\n\n");
      exit(EXIT_SUCCESS);
    case 'p':
      if (optarg != NULL){
	partition = (partition==-1) ? atoi(optarg) : partition;
	break;
      } else {
	usage(argv[0], stderr);
	exit(C_FORMAT);
      }     
    case 'f':
      if (optarg != NULL){
	filecount = (filecount==-1) ? atoi(optarg) : filecount;
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
	    "%s: requires positive <partition> and strictly positive <max-file-count> values\n"
	    ,argv[0]);
    usage(argv[0], stderr);
    exit(C_FORMAT);
  } else format_partition(name, partition, filecount, argv[0], flags);
}

