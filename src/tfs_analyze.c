#include "ll.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "block.h"
#include "tfs.h"
#include <getopt.h>

#define MODE_DISK 1 /***< Mode analyze disk */
#define MODE_PART 2 /***< Mode analyze partition */
#define MODE_LONG 4 /***< Mode analyze long */

#define DEF_NAME "disk.tfs"   /***< default disk name */
#define D_NAME_MAXLEN 79     /***< disk name maximum length */

/**
 * @brief Analyze a disk
 * Print the size of the disk, number of partitions and size of them
 * \@param name the name of the disk 
 * \@param mode the mode (MODE_LONG,MODE_PART,MODE_DISK)
 * \@param partition the index of partition
 * \@return error 
 */
error tfs_analyze(char *name,int mode,int partition){
  disk_id id;
  
  error err_start = start_disk(name,&id);
  if(err_start != EXIT_SUCCESS){
    //error message
    fprintf(stderr,"Error start : %d\n",err_start);
    return err_start;
  }

  d_stat d_s;
  error err_d_stat = disk_stat(id,&d_s);
  if(err_d_stat != EXIT_SUCCESS){
    //error message
    fprintf(stderr,"Error disk stat : %d\n",err_d_stat);
    return err_d_stat;
  }

  if(mode & MODE_PART){
    if(partition < 0){
      fprintf(stderr,"Error value of partition : enter a positive index of partition\n");
      return P_WRONGVALUE;
    }
    if(partition+1 > d_s.npart){
      fprintf(stderr,"Error value of partition : enter an existing index of partition\n");
      return P_WRONGVALUE;
    }
  }
  
  if(mode & MODE_DISK){
    printf("\n\t\t\tDISK %s :\n\n",name);
 
    printf("%-22s %6d blocks %10d bytes\n\n","Size of this disk :",d_s.size,d_s.size*B_SIZE);
    printf("%-22s %6d \n\n","Number of partitions :",d_s.npart);
  }
  
 if(mode & MODE_LONG){
   printf("%-20s %15s %15s %15s\n","Partition Index","Size (blocks)","Size (bytes)","File System");
   int i = 0;
   block b_zero = new_block();
   for(i=0;i<d_s.npart;i++){
     uint32_t partidx;
     p_index(id,i,&partidx);
     read_block(id,b_zero,partidx);
     uint32_t magic_n;
     rintle(&magic_n,b_zero,TFS_MAGIC_NUMBER_INDEX);
     if(magic_n == TFS_MAGIC_NUMBER){
       printf("%-20d %15d %15d %13s\n",i,d_s.part[i],d_s.part[i]*B_SIZE,"tfs ");  
     }else{
       printf("%-20d %15d %15d %13s\n",i,d_s.part[i],d_s.part[i]*B_SIZE,"unknow");
     }
   }
   free(b_zero);
 }

 if(mode & MODE_PART){
   uint32_t partidx_p;
   p_index(id,partition,&partidx_p);
   block b_zero_part = new_block();
   read_block(id,b_zero_part,partidx_p);
   uint32_t magic_n_p;
   rintle(&magic_n_p,b_zero_part,TFS_MAGIC_NUMBER_INDEX);
   free(b_zero_part);
   printf("%-20s %15s %15s %15s\n","Partition Index","Size (blocks)","Size (bytes)","File System");
   if(magic_n_p == TFS_MAGIC_NUMBER)
     printf("%-20d %15d %15d %13s\n",(partition),d_s.part[partition],d_s.part[partition]*B_SIZE,"tfs ");
   else
     printf("%-20d %15d %15d %13s\n",(partition),d_s.part[partition],d_s.part[partition]*B_SIZE,"unknow");
 }

 return EXIT_SUCCESS;
}


int main (int argc, char *argv[]){
  int c;
  int mode = MODE_DISK;
  char name[D_NAME_MAXLEN+1];
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
      mode |= MODE_LONG;
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

  return tfs_analyze(name,mode,partition);
}
