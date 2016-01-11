// tfs_cp.c
//
// last-edit-by: <syscp> 
// 
// Description: Command to copy files like like unix cp
//
//////////////////////////////////////////////////////////////////////

#include "tfsll.h"
#include "tfs.h"
#include "error.h"
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
void cp(char *argv0, char *oldpath, char *newpath);

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

  if (optind + 2 < argc) {
    fprintf(stderr, "%s : argument missing.\n", argv[0]);
    usage(argv[0]);
    exit(-1);
  }
  char *oldpath = argv[optind];
  char *newpath = argv[optind+1];
  cp(argv[0], oldpath, newpath);
  return 0;
}



void
cp(char *argv0, char *oldpath, char *newpath)
{
  char *oldparent = strdup(oldpath);
  char *newparent = strdup(newpath);
  char *old_last, *new_last;
  if (path_split(oldparent, &old_last) != EXIT_SUCCESS) {
    fprintf(stderr, "Path %s invalid.\n", oldpath);
    usage(argv0);
    exit(TFS_ERRPATH);
  }
  int ishost1, ishost2;
  // check if HOST or TFS
  char *disk1, *disk2;
  if (path_follow(NULL, &disk1) != EXIT_SUCCESS) {
   fprintf(stderr, "Path %s invalid.\n", oldpath);
   usage(argv0);
   exit(TFS_ERRPATH);
  }
  ishost1 = ISHOST(disk1);
  disk_id id;
  uint32_t vol_addr;
  if (!ishost1) {
    if (start_disk(disk1, &id) != EXIT_SUCCESS) {
      fprintf(stderr, "Can't access to disk %s.\n", disk1);
      usage(argv0);
      exit(TFS_ERRPATH);
    }
    char *token;
    if (path_follow(NULL, &token) != EXIT_SUCCESS) {
      fprintf(stderr, "Path %s invalid.\n", oldpath);
      usage(argv0);
      exit(TFS_ERRPATH);
    }
    long long int pid = atou(token);
    if (pid < 0 || p_index(id, pid, &vol_addr) != EXIT_SUCCESS) {
      fprintf(stderr, "Wrong volume index.\n");
      usage(argv0);
      exit(TFS_ERRPATH);
    }   
  }
  if (path_split(newparent, &new_last) != EXIT_SUCCESS) {
    fprintf(stderr, "Path %s invalid.\n", oldpath);
    usage(argv0);
    exit(TFS_ERRPATH);
  }  
  if (path_follow(NULL, &disk2) != EXIT_SUCCESS) {
   fprintf(stderr, "Path %s invalid.\n", oldpath);
   usage(argv0);
   exit(TFS_ERRPATH);
  }
  ishost2 = ISHOST(disk2);
  char buff[1024];
  
  if (ishost1)
    {
      char *hostpath = oldpath + PATH_FPFXLEN + 4;
      struct stat path_stat;
      stat(hostpath, &path_stat);
      if (S_ISREG(path_stat.st_mode))
	{
	  int oldfd = open(hostpath, O_RDONLY);
	  if (oldfd == -1) {
	    fprintf(stderr, "Can't open file %s on HOST.\n", hostpath);
	    usage(argv0);
	    exit(-1);
	  }
	  if (ishost2)
	    {
	      char *hostpath2 = newpath + PATH_FPFXLEN + 4;
	      int newfd = open(hostpath, O_WRONLY | O_CREAT | O_TRUNC,
			       S_IRUSR|S_IWUSR);
	      if (newfd == -1) {
		fprintf(stderr, "Can't open file %s on HOST.\n", hostpath);
		usage(argv0);
		exit(-1);
	      }
	      int lu;
	      while ((lu = read(newfd, buff, 1024)) > 0)
		if (write(newfd, buff, 1024) < lu) {
		  fprintf(stderr, "Writing error on %s at HOST.\n", hostpath2);
		  usage(argv0);
		  exit(-1);
		}
	      if (lu < 0) {
		fprintf(stderr, "Reading error on %s at HOST.\n", hostpath);
		usage(argv0);
		exit(-1);
	      }
	      printf("File %s copied at %s.\n", hostpath, hostpath2);
	      exit(0);
	    }
	  else
	    {
	      int newfd = tfs_open(newpath, O_WRONLY | O_CREAT | O_TRUNC);
	      if (newfd == -1) {
		fprintf(stderr, "Can't open file %s.\n", newpath);
		usage(argv0);
		exit(-1);
	      }
	      int lu;
	      puts("but");
	      while ((lu = read(newfd, buff, 1024)) > 0)
		if (tfs_write(newfd, buff, 1024) < lu) {
		  fprintf(stderr, "Writing error on %s.\n", newpath);
		  usage(argv0);
		  exit(-1);
		}
	      if (lu < 0) {
		fprintf(stderr, "Reading error on %s at HOST.\n", hostpath);
		usage(argv0);
		exit(-1);
	      }
	      printf("File %s copied at %s.\n", hostpath, newpath);
	      exit(0);
	    }
	}
      else if (S_ISDIR(path_stat.st_mode))
	{
	  fprintf(stderr, "File %s on HOST is a directory.\n", hostpath);
	  exit(-1);
	}
      else
	{
	  fprintf(stderr, "Copy not supported for this type of file.\n");
	  usage(argv0);
	  exit(-1);
	}
    }
  else
    {
      f_stat fstat;
      uint32_t ino;
      if (find_inode(oldpath, &ino)) {
	fprintf(stderr, "File %s doesn't exists.\n", oldpath);
	exit(-1);
      }
      if (file_stat(id, vol_addr, ino, &fstat) != EXIT_SUCCESS) {
	fprintf(stderr, "Can't access to file %s.\n", oldpath);
	exit(-1);
      }
      if (TFS_ISREG(fstat.type))
	{
	  int oldfd = tfs_open(oldpath, O_RDONLY);
	  if (oldfd == -1) {
	    fprintf(stderr, "Can't open file %s.\n", oldpath);
	    usage(argv0);
	    exit(-1);
	  }
	  if (ishost2)
	    {
	      char *hostpath2 = newpath + PATH_FPFXLEN + 4;
	      int newfd = open(hostpath2, O_WRONLY | O_CREAT | O_TRUNC
			       , S_IRUSR|S_IWUSR);
	      if (newfd == -1) {
		fprintf(stderr, "Can't open file %s on HOST.\n", hostpath2);
		exit(-1);
	      }
	      int lu;
	      while ((lu = tfs_read(newfd, buff, 1024)) > 0)
		if (write(newfd, buff, 1024) < lu) {
		  fprintf(stderr, "Writing error on %s at HOST.\n", hostpath2);
		  usage(argv0);
		  exit(-1);
		}
	      if (lu < 0) {
		fprintf(stderr, "Reading error on %s.", oldpath);
		exit(-1);
	      }
	      printf("File %s copied at %s.\n", oldpath, hostpath2);
	      exit(0);
	    }
	  else
	    {
	      int newfd = tfs_open(newpath, O_WRONLY | O_CREAT | O_TRUNC);
	      if (newfd == -1) {
		fprintf(stderr, "Can't open file %s.\n", newpath);
		usage(argv0);
		exit(-1);
	      }
	      int lu;
	      while ((lu = tfs_read(newfd, buff, 1024)) > 0)
		if (tfs_write(newfd, buff, 1024) < lu) {
		  fprintf(stderr, "Writing error on %s.\n", newpath);
		  exit(-1);
		}
	      if (lu < 0) {
		fprintf(stderr, "Reading error on %s at HOST.\n", oldpath);
		usage(argv0);
		exit(-1);
	      }
	      printf("File %s copied at %s.\n", oldpath, newpath);
	      exit(0);
	    }
	}
      else if (S_ISDIR(fstat.type))
	{
	  fprintf(stderr, "%s is a directory.\n", oldpath );
	  usage(argv0);
	  exit(-1);
	}
      else
	{
	  fprintf(stderr, "Copy not supported for this type of file.\n");
	  usage(argv0);
	  exit(-1);
	}
    }
}

void usage(char *argv0)
{
  printf("Usage :\t%s <oldpath> <newpath>\n", argv0);
}


void help(char *argv0)
{
  printf("%s copy files from <oldpath> to <newpath>\n", argv0);
  usage(argv0);
}
