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


struct tfs_description {
  uint32_t magic_number;
  uint32_t block_size;
  uint32_t volume_size;
  uint32disk_id id, uint32_t vol, _t freeb_count;
  uint32_t freeb_first;
  uint32_t maxfile_count;
  uint32_t freefile_count;
  uint32_t freefile_first;
}

void
write_tfsdescription ( uint32_t vol_addr, tfs_description *desc )
{
  block b = new_block();
  wintle(desc->magic_number, b, 0);
  wintle(desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX);
  wintle(desc->tfs_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX);
  wintle(desc->nfree_b, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX);
  wintle(desc->afree_b, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX);
  wintle(desc->nmax_f, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX);
  wintle(desc->nfree_f, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX);
  wintle(desc->ifree_f, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX);
  write_block(id, b, vol_addr);
  free(block);
}

tfs_description *
read_tfsdescription ( uint32_t vol_addr )
{
  tfs_description *desc = (tfs_description *) malloc(sizeof(tfs_description));
  rintle(&desc->magic_number, b, 0);
  rintle(&desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX);
  rintle(&desc->tfs_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX);
  rintle(&desc->nfree_b, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX);
  rintle(&desc->afree_b, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX);
  rintle(&desc->nmax_f, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX);
  rintle(&desc->nfree_f, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX);
  rintle(&desc->ifree_f, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX);
  return des;
}


error
freeblock_push ( disk_id id, uint32_t vol, const uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
freeblock_rm ( disk_id id, uint32_t vol, const uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
directory_pushent ( disk_id id, uint32_t vol, DIR directory, const struct dirent *restrict entry  ) {

  return EXIT_SUCCESS;
}



error
directory_rment ( disk_id id, uint32_t vol, DIR directory, const struct dirent *restrict entry ) {

  return EXIT_SUCCESS;
}



error
file_pushblock ( disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
file_rmblock( disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr ) {

  return EXIT_SUCCESS;
}



error
file_freeblocks ( disk_id id, uint32_t vol, uint32_t inode ) {

  return EXIT_SUCCESS;
}


// Macros for path_follow
#define PATH_STRSEP "/"
#define PATH_FPFX "FILE://"
#define PATH_FPFXLEN 7
#define PATH_ISVALID(p) (strncmp(p, PATH_FPFX, PATH_FPFXLEN) == 0)
error
path_follow( const char * path,  char **entry ) {
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
      if (workpath != NULL)
	free(workpath);
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
