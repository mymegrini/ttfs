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
#define INT_BSIZE 4  /***< int byte size */
#define TFSENT_BSIZE 64   /***< file tab entry size */

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

/**
 * @brief This function initializes a partition's superblock
 * @param[in] id disk id
 * @param[in] pidx partition index
 * @param[in] psize partition size
 * @param[in] filecount max file count
 * @return void
 * 
 * This function fills in the initial filesystem's description
 * for the partition located at <pidx> on the disk opened at <id>
 */
void init_sblock(int id, uint32_t pidx, uint32_t psize, int filecount){
  block b = new_block();
  int fbc = psize-(filecount-1)/16-3;
  int mn = MAGIC_NUMBER;
  
  testerror("init_sblock", wintle(MAGIC_NUMBER, b, 0*INT_BSIZE));
  printf("Filesystem label= %s\n", (char*)&mn);
  
  testerror("init_sblock", wintle(B_SIZE, b, 1*INT_BSIZE));
  printf("Block size= %dB\n", B_SIZE);
  
  testerror("init_sblock", wintle(psize, b, 2*INT_BSIZE)); 
  printf("Volume block count= %d\n", psize);
  
  testerror("init_sblock", wintle(fbc, b, 3*INT_BSIZE));
  printf("Free block count= %d ", fbc);
  
  testerror("init_sblock", wintle(fbc ? (filecount-1)/16+3 : 0, b, 4*INT_BSIZE));
  if(fbc) printf("(%d .. %d)\n", (filecount-1)/16+3, psize-1);
  else puts("");
  
  testerror("init_sblock", wintle(filecount, b, 5*INT_BSIZE));
  printf("Maximum file count= %d ", filecount);
  
  testerror("init_sblock", wintle(filecount-1, b, 6*INT_BSIZE));
  printf("(%d free)\n", filecount-1);
  
  testerror("init_sblock", wintle(1, b, 7*INT_BSIZE));  
  testerror("init_sblock", write_block(id, b, pidx));  
  free(b);
}

/**
 * @brief This function initializes a partition's file table
 * @param[in] id disk id
 * @param[in] pidx partition index
 * @param[in] psize partition size
 * @param[in] filecount max file count
 * @return void
 * 
 * This function creates the root directory's file table entry
 * then proceeds to chain the remain free file table entries
 */
void init_ftab(int id, uint32_t pidx, uint32_t psize, int filecount){
  block b = new_block();
  int index = 0;
  
  testerror("init_ftab", wintle(B_SIZE, b, 0));
  testerror("init_ftab", wintle(1, b, 1*INT_BSIZE));
  testerror("init_ftab", wintle((filecount-1)/16+2, b, 3*INT_BSIZE));
  
  printf("Writing file table %3d%%", 100*index/filecount);
  
  while(index < filecount){
    testerror("init_ftab", wintle(index+1==filecount?index:index+1,
				  b,
				  15*INT_BSIZE+(index%16)*TFSENT_BSIZE));
    if ((index+1)/16==index/16+1){
      testerror("init_ftab", write_block(id, b, pidx+index/16+1));	      
      printf("\b\b\b\b%3d%%", 100*index/filecount);    
      memset(b->data, 0, sizeof(b->data));
    }
    index++;
  }
  if((index+1)/16 < index/16+1){
    testerror("init_ftab", write_block(id, b, pidx+index/16+1));  
    printf("\b\b\b\b%3d%%\n", 100*index/filecount);
  }
  free(b);
  printf("File table block size= %d\n", (filecount-1)/16+1);
}

/**
 * @brief This function initializes a file system's root directory
 * @param[in] id disk id
 * @param[in] ridx root directory's index on the disk
 * @return void
 * 
 * This function initializes the data block occupied by the root
 * directory with the appropriate values
 */
void init_root(int id, uint32_t ridx){
  block b = new_block();
  printf("Creating root directory: ");
  testerror("init_root", wintle(0, b, 0));
  strcpy((char*)b->data+INT_BSIZE, ".");
  testerror("init_root", wintle(0, b, 32));
  strcpy((char*)b->data+9*INT_BSIZE, "..");
  testerror("init_root", write_block(id, b, ridx)); 
  free(b);
  puts("done");
}

/**
 * @brief This function chains the volume's free blocks
 * @param[in] id disk id
 * @param[in] pidx partition index
 * @param[in] psize partition size
 * @param[in] filecount max file count
 * @return void
 * 
 * This function chains the volume's free blocks making
 * last free block its own successor
 */
void init_fblocks(int id, uint32_t pidx, uint32_t psize, int filecount){
  block b = new_block();
  uint32_t index = (filecount-1)/16+3;
  printf("Formatting volume %3d%%", index/psize);
  while(index<psize){    
    testerror("init_fblocks", wintle(index+1==psize?index:index+1, b, 255*INT_BSIZE));
    testerror("init_fblocks", write_block(id, b, pidx+index));
    printf("\b\b\b\b%3d%%", 100*index/psize);
    memset(b->data, 0, sizeof(b->data));
    index++;
  }
  printf("\b\b\b\b%3d%%\n", 100*index/psize);
  free(b);
}

/**
 * @brief This function opens a disk and formats one of its partitions
 * @param[in] name name of the disk
 * @param[in] partition index of the partition
 * @param[in] filecount max file count
 * @param[in] argv0 name of the calling command
 * @param[in] flags value containing option flags
 * @return void
 * 
 * This function opens a disk and proceeds to format one of its
 * partitions, provided it is empty or the user clears it for 
 * overwriting.
 *
 * @see init_sblock
 * @see init_ftab
 * @see init_root
 * @see init_fblocks
 */
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
    if(psize<((filecount-1)/16)+3){
      printf("%s: partition too small for specified <max-file-count> value\n"
	     ,argv0);
      exit(C_FORMAT);
    }
    
    for(k=0; k<partition; k++) pidx+=stat.part[k];
    
    if(pidx+psize>stat.size){
      printerror(argv0,P_CORRUPTED);
      exit(P_CORRUPTED);
    }
    
    b= new_block();
    testerror("read_block 0", read_block(id, b, pidx));
    testerror("rintle 0", rintle(&k, b, 0));

    if((flags & F_OWR)==0 && k == 0x31534654){
      printf("%s: partition %d already contains a filesystem.\n",
	     argv0, partition);
      if (answer("Overwrite? [Y/n] ")==0)
	exit(EXIT_SUCCESS);
    }

    free(b);
    
    init_sblock(id, pidx, psize, filecount);	
    init_ftab(id, pidx, psize, filecount);
    init_root(id, pidx+(filecount-1)/16+2);
    init_fblocks(id, pidx, psize, filecount);

    /*printf("%s : Formatting successfully completed.\n	\
\t%s : \t(%dB)\n\tPartition %d : \t(%dB)\n\t%dB\n",
argv0, name, stat.size*B_SIZE, partition, psize*B_SIZE, B_SIZE*((filecount-1)/16+2));*/    
  }
}

/**
 * @brief This function parses command line input
 * @param[in] id disk id
 * @param[in] pidx partition index
 * @param[in] psize partition size
 * @param[in] filecount max file count
 * @return void
 * 
 * This function parses command line input and launches
 * auxiliary functions with the appropriate parameters
 * and options
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
	    "%s: requires correct <partition> and <max-file-count> values\n"
	    ,argv[0]);
    usage(argv[0], stderr);
    exit(C_FORMAT);
  } else format_partition(name, partition, filecount, argv[0], flags);
}

