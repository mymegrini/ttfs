#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>

#include "tfs.h"
#include "tfsll.h"
#include "utils.h"
#include "block.h"

#define OPT_HELP 'h'
#define OPT_HELP_LONG "help"

#define OPT_STRING          "h:"

#define PATH_MISSING -2


struct option options[2] = {
  { OPT_HELP_LONG, no_argument, 0, OPT_HELP },
  { 0, 0, 0, 0 }
};


void usage (char *argv0){
  printf("Usage :\t%s source destination\n",argv0);
}


void help (char *argv0) {
  usage(argv0);
  printf("%s move source to destination.\n"
	 "Options :\n"
	 "-%c\t--%s\tPrint this message.\n",
	 argv0,
	 OPT_HELP, OPT_HELP_LONG);
}

error mv (char *source, char *dest){

  char *entry_s;
  char *entry_d;
  char *parent_s;
  char *parent_d;
  error e;
  DIR *dir_s;
  struct dirent *dirent; 
  uint32_t ino_file,ino_s,ino_d;
  disk_id id_s,id_d;
  char *name_disk_s;
  char *name_disk_d;
  uint32_t vol_addr_s,vol_addr_d;
 
  if((e = path_follow(source, NULL)) == TFS_ERRPATH_NOPFX){
    fprintf(stderr,"Error: the path is not prefixed by FILE://\n");
    return e;
  }
  if((e = path_follow(NULL,&entry_s)) == TFS_ERRPATH_NOWORKINGPATH){
    fprintf(stderr,"Error: wrong path %s",source);
    return e;
  }
  name_disk_s = entry_s;
  strcat(name_disk_s,".tfs");
  if((e = start_disk(name_disk_s,&id_s)) != EXIT_SUCCESS)
    return e;

  if((e = path_follow(NULL,&entry_s)) == TFS_ERRPATH_NOWORKINGPATH){
    fprintf(stderr,"Error: wrong path %s",source);
    return e;
  }  
  vol_addr_s = atou(entry_s); 

 if((e = path_follow(dest, NULL)) == TFS_ERRPATH_NOPFX){
    fprintf(stderr,"Error: the path is not prefixed by FILE://\n");
    return e;
  }
  if((e = path_follow(NULL,&entry_d)) == TFS_ERRPATH_NOWORKINGPATH){
    fprintf(stderr,"Error: wrong path %s",dest);
    return e;
  }
  name_disk_d = entry_d;
  strcat(name_disk_d,".tfs");
  if((e = start_disk(name_disk_d,&id_d)) != EXIT_SUCCESS)
    return e;

  if((e = path_follow(NULL,&entry_d)) == TFS_ERRPATH_NOWORKINGPATH){
    fprintf(stderr,"Error: wrong path %s",dest);
    return e;
  }  
  vol_addr_d = atou(entry_d);

  if((e = path_split(entry_s,&parent_s)) != EXIT_SUCCESS)
     return e;

  if((e = path_split(entry_d,&parent_d)) != EXIT_SUCCESS)
     return e;

  if((e = find_inode(parent_s, &ino_s)) != EXIT_SUCCESS)
    return e;

  if((e = find_inode(parent_d, &ino_d)) != EXIT_SUCCESS)
    return e;

  if((e = find_inode(source, &ino_file)) != EXIT_SUCCESS)
    return e;

  dir_s = opendir(name_disk_s);
  dirent = readdir(dir_s);
  while(strcmp((*dirent).d_name,entry_s) != 0)
    dirent = readdir(dir_s);
  ino_file = (*dirent).d_ino;
  
  (*dirent).d_ino = ino_file;
  if((e = directory_pushent(id_d,vol_addr_d,ino_d,dirent)) != EXIT_SUCCESS)
    return e;

  if((e = directory_rment(id_s,vol_addr_s,ino_s,name_disk_s)) != EXIT_SUCCESS)
    return e;
  
  
  return EXIT_SUCCESS;
} 


int main (int argc, char *argv[] ){

  char c;
  char *source;
  char *dest;
  while ((c = getopt_long(argc, argv, OPT_STRING, options, 0)) != -1){
    switch (c){
    case OPT_HELP:
      help(argv[0]);
      return EXIT_SUCCESS;
      
    default:
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }
  if (optind == argc) {
    fputs("Argument path missing.", stderr);
    usage(argv[0]);
    return PATH_MISSING;
  }
  source = argv[optind-1];
  dest = argv[optind-1];

  return mv(source,dest);
}
