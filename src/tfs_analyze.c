#include "ll.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "block.h"
#include <getopt.h>

#define F_LONG 1 /***< Long flag */

#define MODE_DISK 1 /***< Mode analyze disk */
#define MODE_PART 2 /***< Mode analyze partition */

#define DEF_NAME "disk.tfs"   /***< default disk name */
#define D_NAME_MAXLEN 79     /***< disk name maximum length */

/**
 * Analyze a disk
 * Print the size of the disk, number of partitions and size of them
 *
 * \param name the name of the disk 
 * \return error 
 */
error tfs_analyze(char *name){

disk_id id;
 
error err_start = start_disk(name,&id);
 if(err_start != EXIT_SUCCESS){
   //error message
   printf("Erreur start : %d\n",err_start);
   return err_start;
 }
 block b_zero = new_block();
 error err_read = read_block(id,b_zero,0);
 if(err_read != EXIT_SUCCESS){  
   //error message
   printf("Erreur read : %d\n",err_read);
   return err_read;
 }

 printf("\t\t\tDISK %s :\n\n",name);
 
 uint32_t size_disk;
 rintle(&size_disk,b_zero,0*INT_SIZE);
 printf("%-22s %6d blocks %10d bytes\n","Size of this disk :",size_disk,size_disk*B_SIZE);
 
 uint32_t nb_part;
 rintle(&nb_part,b_zero,1*INT_SIZE);
 printf("%-22s %6d \n\n","Number of partitions :",nb_part);
 
 int i = 0;
 if(nb_part != 0)
   printf("%-20s %15s %15s\n","Partition Index","Size (blocks)","Size (bytes)");
 for(i=0;i<nb_part;i++){
   uint32_t size_part;
   rintle(&size_part,b_zero,(i+2)*INT_SIZE);
   printf("%-20d %15d %15d\n",(i+1),size_part,size_part*B_SIZE);  
 }
 
 return EXIT_SUCCESS;
}


int main (int argc, char *argv[]){
  int c;
  int mode = MODE_DISK;
  char name[D_NAME_MAXLEN+1];
  int flags = 0;
  int partition = -1;
  
  while(1) {
    int index = 0;
    static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"long", no_argument, NULL, 'l'},
      {"partition", required_argument, NULL, 'p'},
      {0, 0, 0, 0}
    };
    c = getopt_long_only(argc, argv, "p:l", long_options, &index);
    
    if(c == -1) break;
    
    switch (c) {
    case 'h':
      printf("This command prints informations about a disk\n\
Usage: %s [-p <partition>] [-l] [name]\n\n\
  -p\t--partition\tspecify partition id\n\
  -l\t--long\t\tprint detailed informations\n\
    \t--help\t\tdisplay this help and exit\n\n",argv[0]);
      exit(EXIT_SUCCESS);
    case 'l':
      flags |= F_LONG;
      break;
    case 'p':
      if (optarg != NULL){
	partition = (partition==-1) ? atoi(optarg) : partition;
	mode |= MODE_PART;
	break;
      } else {
	fprintf(stderr,"Usage: %s [-p <partition>] [-l] [name]\n",argv[0]);
	exit(C_FORMAT);
      }
    default : 
      fprintf(stderr,"Usage: %s [-p <partition>] [-l] [name]\n",argv[0]);
      exit(C_FORMAT);
    }
  }
  
  if(optind < argc){
    if(strncmp(argv[optind]+strlen(argv[optind])-4, ".tfs", 4) == 0)
      strncpy(name, argv[optind], D_NAME_MAXLEN);
    else {      
      strncpy(name, argv[optind], D_NAME_MAXLEN-4);
      strncat(name, ".tfs", D_NAME_MAXLEN-strlen(name));
    }
  } else 
    strncpy(name, DEF_NAME, 9);

  /* test si la partition fournies est negative  
  if ((mode == 3) && partition < 0){
    fprintf(stderr,"%s: requires positive <partition>\n\
Usage: %s [-p <partition>] [-l] [name]\n",argv[0],argv[0]);
    exit(C_FORMAT);
  }
  */
  
  tfs_analyze(name);

  return 0;
}