// tfsll.c
//
// last-edit-by: <nscott32> 
// 
// Description: TFS low level library source code
//
//////////////////////////////////////////////////////////////////////

#include "tfsll.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


error
freeblock_push ( const uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
freeblock_rm ( const uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
directory_pushent ( DIR directory, const struct dirent *restrict entry  ) {

  return EXIT_SUCCESS;
}



error
directory_rment ( DIR directory, const struct dirent *restrict entry ) {

  return EXIT_SUCCESS;
}



error
file_pushblock ( uint32_t inode, uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
file_rmblock( uint32_t inode, uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
file_freeblocks ( uint32_t inode ) {

  return EXIT_SUCCESS;
}


// Macros for path_follow
#define PATH_STRSEP "/"
#define PATH_STRSEP "/"
#define PATH_FPFX "FILE://"
#define PATH_FPFXLEN 7
#define PATH_ISVALID(p) (strncmp(p, PATH_FPFX, PATH_FPFXLEN) == 0)
error
path_follow( char * path, char **entry ) {
  static char *workpath = NULL;
  if (path == NULL) {
    if (workpath == NULL)
      return TFS_ERRPATH_NOWORKINGPATH;
    else {
      *entry = strtok(NULL, PATH_STRSEP);
      if (*entry == NULL) {
	free(workpath);
	workpath = NULL;
	return TFS_PATHLEAF;
      }
      else
	return EXIT_SUCCESS;
    }
  }
  else {
    if (PATH_ISVALID(path)) {
      workpath = strdup(path);
      strtok(workpath, PATH_STRSEP);
      return EXIT_SUCCESS;
    }
    else
      return TFS_ERRPATH_NOPFX;
  }
}



//////////////////////////////////////////////////////////////////////
// $Log:$
//
