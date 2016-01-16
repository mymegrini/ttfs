// tfs_cp.c
//
// last-edit-by: <syscp> 
// 
// Description: Command to list informations about file.
//
//////////////////////////////////////////////////////////////////////

#include "tfsll.h"
#include "tfs.h"
#include "error.h"
#include "utils.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////
// OPTIONS
////////////////////////////////////////////////////////////////////////////////

#define OPT_HELP            'h'
#define OPT_HELP_LONG       "help"

#define OPT_STRING          "h"



struct option options[4] = {
  { OPT_HELP_LONG, no_argument, 0, OPT_HELP },
  { 0, 0, 0, 0 }
};
////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////////

void usage(char *argv0);
void help(char *argv0);
void ls(char *argv0, char *path);


int
main ( int argc, char *argv[] )
{
  char c;
  while ((c = getopt_long(argc, argv, OPT_STRING, options, 0)) != -1)
    {
      switch (c)
	{
	case OPT_HELP:
	  help(argv[0]);
	  return EXIT_SUCCESS;

	default:
	  usage(argv[0]);
	  return EXIT_FAILURE;
	}
    }

  if (optind == argc) {
    fprintf(stderr, "%s : argument missing.\n", argv[0]);
    usage(argv[0]);
    exit(-1);
  }
  char *path = argv[optind];
  ls(argv[0], path);
  return 0;
}


void ls(char *argv0, char *path)
{
  //char *parent = strdup(path);
  char *token;
  error e;

  //parsing disk name
  path_follow(path, NULL);
  if (path_follow(NULL, &token) != EXIT_SUCCESS) {
    fprintf(stderr, "Path %s invalid.\n", path);
    usage(argv0);
    exit(TFS_ERRPATH);
  }  
  if (ISHOST(token)){
    path+=PATH_FPFXLEN+4;
    exit(execlp("ls", "ls",(strlen(path)? path : NULL), NULL));
  }  
  disk_id id;  
  if ((e=start_disk(token, &id)) != EXIT_SUCCESS) {
    printerror("ls", e);
    usage(argv0);
    exit(TFS_ERRPATH);
  }
  
  //parsing volume
  if (path_follow(NULL, &token) != EXIT_SUCCESS) {    
    fprintf(stderr, "Path %s invalid.\n", path);
    usage(argv0);
    exit(TFS_ERRPATH);
  }
  long long int pid;
  if ((pid = atou(token)) < 0) {
    fprintf(stderr, "Volume %s incorrect.\n", token);
    usage(argv0);
    exit(TFS_ERRPATH);
  }
  uint32_t vol_addr;
  p_index(id, pid, &vol_addr);
  uint32_t ino;
  f_stat fstat;
  if (find_inode(path, &ino) != EXIT_SUCCESS || 
      file_stat(id, vol_addr, ino, &fstat) != EXIT_SUCCESS){
      fprintf(stderr, "Can't access file %s.\n", path);
      exit(-1);
  }
  if (TFS_ISREG(fstat.type)){
    printf("%s %d %d %d\n", path, fstat.size, fstat.type, fstat.subtype);
    exit(0);
  }
  else if (TFS_ISDIR(fstat.type))
    {
      DIR *dir = opendir(path);
      if (dir == NULL) {
	fprintf(stderr, "Can't open directory %s.\n", path);
	exit(-1);
      }
      struct dirent *ent;
      while ((ent = readdir(dir)) != NULL)
	printf("%s\n", ent->d_name);
      exit(0);
    }
  else {
    fprintf(stderr, "Can't open file %s, type invalid.\n", path);
    exit(-1);
  }   
}


void usage(char *argv0)
{
  printf("Usage :\t%s <path> \n", argv0);
}


void help(char *argv0)
{
  printf("%s list file information about <path>\n.", argv0);
  usage(argv0);
}
