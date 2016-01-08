// mkdir.c
//
// last-edit-by: <syscp team> 
// 
// Description: mkdir commands, working like unix mkdir
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>

#include "tfs.h"
#include "tfsll.h"
#include "default.h"

////////////////////////////////////////////////////////////////////////////////
// OPTIONS
////////////////////////////////////////////////////////////////////////////////

#define OPT_PARENTS         'p'
#define OPT_PARENTS_LONG    "path"
#define OPT_NAME            'n'
#define OPT_NAME_LONG       "name"
#define OPT_HELP            'h'
#define OPT_HELP_LONG       "help"

#define OPT_STRING          "hpn:"

struct option options[4] = {
  { OPT_HELP_LONG, no_argument, 0, OPT_HELP },
  { OPT_NAME_LONG, required_argument, 0, OPT_NAME },
  { OPT_PARENTS_LONG, no_argument, 0, OPT_PARENTS },
  { 0, 0, 0, 0 }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ERRORS
////////////////////////////////////////////////////////////////////////////////
#define PATH_FORMAT     -1
#define DIRPATH_MISSING -2
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// PROTOYPES
////////////////////////////////////////////////////////////////////////////////
error recursive_mkdir (char *fullpath, mode_t mode);
void usage (char *argv0);
void  help (char *argv0);



int
main ( int argc, char *argv[] )
{
  char c;
  int opt_parents = 0;
  char *diskname = strdup(DEF_FILENAME);
  char *path;
  while ((c = getopt_long(argc, argv, OPT_STRING, options, 0)) != -1)
    {
      switch (c)
	{
	case OPT_HELP:
	  help(argv[0]);
	  return EXIT_SUCCESS;

	case OPT_PARENTS:
	  opt_parents = 1;
	  break;

	case OPT_NAME:
	  free(diskname);
	  diskname = optarg;
	  break;
	  
	default:
	  usage(argv[0]);
	  return EXIT_FAILURE;
	}
    }

  if (optind == argc) {
    fputs("Argument dirpath missing.", stderr);
    usage(argv[0]);
    return DIRPATH_MISSING;
  }
  path = argv[optind];
  error e;
  if (opt_parents)
    {
      if ((e = tfs_mkdir(path, 0777)) != EXIT_SUCCESS)
	{
	  /* TODO:  Print an adapted error message */
	  fprintf(stderr, "Can't create directory %s\n", path);
	  return e;
	}
      printf("Directory %s created.\n", path);
      return EXIT_SUCCESS;
    }
  else
    return recursive_mkdir(path, 0777);
}



error
recursive_mkdir(char *fullpath, mode_t mode)
{
  error e;
  char *path2;
  if (path_follow(fullpath, &path2) == TFS_ERRPATH_NOPFX)
    {
      fputs("Unvalid path.\n", stderr);
      return PATH_FORMAT;
    }
  int offset = strlen(path2);
  char *token;
  while ((e = path_follow(NULL, &token)) == EXIT_SUCCESS) // i'm not sure about that
    {
      offset = strlen(token);
      strcpy(&path2[offset], token);
      e = mkdir(path2, mode);
    }
  if (e == EXIT_SUCCESS)
    printf("Directory %s created.\n", fullpath);
  else
    fprintf(stderr, "Can't create directory %s\n", fullpath);
  return e;
}

void
usage (char *argv0) {
  printf("Usage :\t%s (-%c) (-%c <diskname>) dirpath\n",
	 argv0, OPT_PARENTS, OPT_NAME);  
}


void
help (char *argv0) {
  usage(argv0);
  printf("%s create a directory.\n"
	 "Options :\n"
	 "-%c\t--%s\tPrint this message.\n"
	 "-%c\t--%s\tCreates parents, returns no error.\n"
	 "-%c\t--%s <diskname>\tSpecify diskname, defaults : %s\n",
	 argv0,
	 OPT_HELP, OPT_HELP_LONG,
	 OPT_PARENTS, OPT_PARENTS_LONG,
	 OPT_NAME, OPT_NAME_LONG, DEF_FILENAME);
}

////////////////////////////////////////////////////////////////////////////////
// $Log:$
//
