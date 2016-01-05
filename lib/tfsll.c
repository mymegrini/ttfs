// tfsll.c
//
// last-edit-by: <nscott32> 
// 
// Description: TFS low level library source code
//
//////////////////////////////////////////////////////////////////////

#include "ll.h"
#include "tfsll.h"
#include "block.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define LASTINT_IDX (B_SIZE - 1 - SIZEOF_INT)

typedef struct {
  uint32_t magic_number;
  uint32_t block_size;
  uint32_t volume_size;
  uint32disk_id id, uint32_t vol, _t freeb_count;
  uint32_t freeb_first;
  uint32_t maxfile_count;
  uint32_t freefile_count;
  uint32_t freefile_first;
} tfs_description;

typedef struct {
  uint32_t size;
  uint32_t type;
  uint32_t subtype;
  uint32_t tfs_direct[TFS_DIRECT_BLOCKS_NUMBER];
  uint32_t tfs_indirect1;
  uint32_t tfs_indirect2;
  uint32_t nextfreefile;
} tfs_ftent;


#define NENTBYBLOCK (B_SIZE / TFS_FILE_TABLE_ENTRY_SIZE)
#define INO_FTBLOCK(inode) (inode/NENTYBLOCK)
#define INO_BPOS(inode) (inode%NENTYBLOCK)


void
read_ftent ( block b, const uint32_t bpos, tfs_ftent * ftent ) {
  const uint32_t entry_pos = TFS_FILE_TABLE_ENTRY * bpos; 
  rintle(&ftent->size, b, entry_pos + TFS_FILE_SIZE_INDEX);
  rintle(&ftent->type, b, entry_pos + TFS_FILE_TYPE_INDEX);
  rintle(&ftent->subtype, b, entry_pos + TFS_FILE_SUBTYPE_INDEX);
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++)
    rintle(&ftent->tfs_direct[i], b, entry_pos + TFS_DIRECT_INDEX(i));
  rintle(&ftent->tfs_indirect1, b, entry_pos + TFS_INDIRECT1_INDEX);
  rintle(&ftent->tfs_indirect2, b, entry_pos + TFS_INDIRECT2_INDEX);
}


error
write_tfsdescription (const disk_id id, const uint32_t vol_addr, const tfs_description * desc)
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
  error e = write_block(id, b, vol_addr);
  free(block);
  return e;
}



error
read_tfsdescription (const disk_id id, const uint32_t vol_addr, tfs_description * desc)
{
  block b = new_block();
  error e = read_block(id, b, vol_addr);
  if (e != EXIT_SUCCESS) {
    free(b);
    return e;
  }
  rintle(&desc->magic_number, b, 0);
  rintle(&desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX);
  rintle(&desc->tfs_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX);
  rintle(&desc->nfree_b, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX);
  rintle(&desc->afree_b, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX);
  rintle(&desc->nmax_f, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX);
  rintle(&desc->nfree_f, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX);
  rintle(&desc->ifree_f, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX);
  free(b);
  return des;
}


error
freeblock_push (const disk_id id, const uint32_t vol_addr, const uint32_t b_addr) {
  tfs_description tfs_d;
  error e = read_tfsdescription(id, vol, &tfs_d);
  if (e != EXIT_SUCCESS)
    return e;
  block b = new_block();
  wintle(tfs_d->freeb_first, b, LASTINT_IDX);
  e = write_block(id, b, vol_addr + b_addr);
  free(b);
  if (e != EXIT_SUCCESS)
    return e;
  tfs_d.freeb_first = b_addr;
  return write_tfsdescription(id, vol_addr, tfs_d);
}


#define TFS_FULL 200
error
freeblock_rm (const disk_id id, const uint32_t vol_addr) {
  tfs_description desc;
  error e = read_tfsdescription(id, vol_addr, &desc);
  if (e != EXIT_SUCCESS) 
    return e;
  block b = new_block();
  e = read_block(id, b, vol_addr + TFS_FILE_TABLE_ADDR desc.freeb_first);l
  if (e != EXIT_SUCCESS) {
    free(b);
    return e;
  }
  uint32_t nextfreeb;
  e = rintle(&nextfreeb, b, LASTINT_IDX);
  free(b);
  if (e != EXIT_SUCCESS) {
    return e;
  }
  desc.freeb_first = nextfreeb;
  e = write_tfsdescription(id, vol_addr, desc);
  return e;
}



error
directory_pushent (const disk_id id, const uint32_t vol_addr, const uint32_t inode, const struct dirent *entry ) {
  tfs_description desc;
  error e = read_tfsdescription(id, vol_addr, tfs_description *desc);
  if (e != EXIT_SUCCESS)
    return e;
  const uint32_t ft_inode_baddr = inode / NENTBYBLOCK;
  const uint32_t inode_bpos = inode % NENTBYBLOCK;
  block b = new_block();
  read_block(id, b, vol_addr + ft_inode_baddr);
  
  return EXIT_SUCCESS;
}



error
directory_rment (disk_id id, uint32_t vol, DIR directory, const struct dirent *restrict entry) {

  return EXIT_SUCCESS;
}



error
file_pushblock (disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr) {

  return EXIT_SUCCESS;
}



error
file_rmblock (disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr) {

  return EXIT_SUCCESS;
}



error
file_freeblocks (disk_id id, uint32_t vol, uint32_t inode) {

  return EXIT_SUCCESS;
}


// Macros for path_follow
#define PATH_STRSEP "/"
#define PATH_FPFX "FILE://"
#define PATH_FPFXLEN 7
#define PATH_ISVALID(p) (strncmp(p, PATH_FPFX, PATH_FPFXLEN) == 0)
error
path_follow (const char * path,  char **entry) {
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
