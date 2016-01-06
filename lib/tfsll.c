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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////

#define LASTINT_IDX (B_SIZE - 1 - SIZEOF_INT)
#define TFS_ERR_BIGFILE 100
#define NENTBYBLOCK (B_SIZE / TFS_FILE_TABLE_ENTRY_SIZE)
#define INO_FTBLOCK(inode) (1 + (inode/NENTBYBLOCK))
#define INO_BPOS(inode) (inode % NENTBYBLOCK)



////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

typedef struct {
  uint32_t magic_number;
  uint32_t block_size;
  uint32_t volume_size;
  uint32_t freeb_count;
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



////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static void
read_ftent (block b, const uint32_t bpos, tfs_ftent * ftent) {
  const uint32_t entry_pos = TFS_FILE_TABLE_ENTRY_SIZE*bpos; 
  rintle(&ftent->size, b, entry_pos + TFS_FILE_SIZE_INDEX);
  rintle(&ftent->type, b, entry_pos + TFS_FILE_TYPE_INDEX);
  rintle(&ftent->subtype, b, entry_pos + TFS_FILE_SUBTYPE_INDEX);
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++)
    rintle(&ftent->tfs_direct[i], b, entry_pos + TFS_DIRECT_INDEX(i));
  rintle(&ftent->tfs_indirect1, b, entry_pos + TFS_INDIRECT1_INDEX);
  rintle(&ftent->tfs_indirect2, b, entry_pos + TFS_INDIRECT2_INDEX);
}


static void
write_ftent (block b, const uint32_t bpos, tfs_ftent * ftent) {
  const uint32_t entry_pos = TFS_FILE_TABLE_ENTRY_SIZE*bpos;
  wintle(ftent->size, b, entry_pos + TFS_FILE_SIZE_INDEX);
  wintle(ftent->type, b, entry_pos + TFS_FILE_TYPE_INDEX);
  wintle(ftent->subtype, b, entry_pos + TFS_FILE_SUBTYPE_INDEX);
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++)
    wintle(ftent->tfs_direct[i], b, entry_pos + TFS_DIRECT_INDEX(i));
  wintle(ftent->tfs_indirect1, b, entry_pos + TFS_INDIRECT1_INDEX);
  wintle(ftent->tfs_indirect2, b, entry_pos + TFS_INDIRECT2_INDEX);
}



static error
write_tfsdescription (const disk_id id, const uint32_t vol_addr, const tfs_description * desc)
{
  block b = new_block();
  wintle(desc->magic_number, b, 0);
  wintle(desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX);
  wintle(desc->volume_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX);
  wintle(desc->freefile_count, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX);
  wintle(desc->freeb_first, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX);
  wintle(desc->maxfile_count, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX);
  wintle(desc->freefile_count, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX);
  wintle(desc->freefile_first, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX);
  error e = write_block(id, b, vol_addr);
  free(b);
  return e;
}



static error
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
  rintle(&desc->volume_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX);
  rintle(&desc->freefile_count, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX);
  rintle(&desc->freeb_first, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX);
  rintle(&desc->maxfile_count, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX);
  rintle(&desc->freefile_count, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX);
  rintle(&desc->freefile_first, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX);
  free(b);
  return EXIT_SUCCESS;
}


error
freeblock_push (const disk_id id, const uint32_t vol_addr, const uint32_t b_addr) {
  tfs_description tfs_d;
  error e = read_tfsdescription(id, vol_addr, &tfs_d);
  if (e != EXIT_SUCCESS)
    return e;
  block b = new_block();
  wintle(tfs_d.freeb_first, b, LASTINT_IDX);
  e = write_block(id, b, vol_addr + b_addr);
  free(b);
  if (e != EXIT_SUCCESS)
    return e;
  tfs_d.freeb_first = b_addr;
  return write_tfsdescription(id, vol_addr, &tfs_d);
}


#define TFS_FULL 200
error
freeblock_rm (const disk_id id, const uint32_t vol_addr) {
  tfs_description desc;
  error e = read_tfsdescription(id, vol_addr, &desc);
  if (e != EXIT_SUCCESS) 
    return e;
  block b = new_block();
  e = read_block(id, b, vol_addr + desc.freeb_first);
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
  return write_tfsdescription(id, vol_addr, &desc);
}


#define TFS_ERR_OPERATION 214
#define DIR_BLOCKFULL -1
int
find_freedirent (block b) {
  for (int i = 0; i < TFS_DIRECTORY_ENTRIES_PER_BLOCK(B_SIZE); i++) {
    if (b->data[TFS_DIRECTORY_ENTRY_INDEX(i) + SIZEOF_INT] == 0) {
      // free entry
      return TFS_DIRECTORY_ENTRY_INDEX(i);
    }
  }
  return DIR_BLOCKFULL;
}



error
directory_pushent (const disk_id id, const uint32_t vol_addr, const uint32_t inode, const struct dirent *entry ) {
  tfs_description desc;
  error e = read_tfsdescription(id, vol_addr, &desc);
  if (e != EXIT_SUCCESS)
    return e;
  const uint32_t ft_inode_baddr = INO_FTBLOCK(inode);
  const uint32_t inode_bpos = INO_BPOS(inode);
  block b = new_block();
  read_block(id, b, vol_addr + ft_inode_baddr);
  tfs_ftent ftent;
  read_ftent(b, inode_bpos, &ftent);
  if (ftent.type != TFS_DIRECTORY_TYPE) {
    free(b);
    return TFS_ERR_OPERATION;
  }
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++) {
    read_block(id, b, vol_addr + ftent.tfs_direct[i]);
    int pos = find_freedirent(b);
    if (pos != DIR_BLOCKFULL) {
      wintle(entry->d_ino, b, pos);
      memcpy(&b->data[pos + SIZEOF_INT], entry->d_name,
	     sizeof(byte)*TFS_NAME_MAX);
      e = write_block(id, b, vol_addr + ft_inode_baddr);
      free(b);
      return e;
    }
  }
 
  if ((e = read_block(id, b,  vol_addr + ftent.tfs_indirect1)) != EXIT_SUCCESS) {
    free(b);
    return e;
  }
  block indirect = new_block();
  uint32_t indirect_addr;
  for (int i = 0; i < B_SIZE/SIZEOF_INT; i++) {
    rintle(&indirect_addr, b, i * SIZEOF_INT);
    if ((e = read_block(id, indirect, vol_addr + indirect_addr)) != EXIT_SUCCESS)
      return e;
    int pos = find_freedirent(indirect);
    if (pos != DIR_BLOCKFULL) {
      wintle(entry->d_ino, b, pos);
      memcpy(&b->data[pos + SIZEOF_INT], entry->d_name,
	     sizeof(byte)*TFS_NAME_MAX);
      e = write_block(id, b, vol_addr + ft_inode_baddr);
      free(b);
      free(indirect);
      return e;
    }    
  }
  if ((e = read_block(id, b,  vol_addr + ftent.tfs_indirect2)) != EXIT_SUCCESS) {
    free(b);
    free(indirect);
    return e;
  }
  block indirect2 = new_block();
  for (int i = 0; i < B_SIZE/SIZEOF_INT; i++) {
    rintle(&indirect_addr, b, i * SIZEOF_INT);
    if ((e = read_block(id, indirect, vol_addr + indirect_addr)) != EXIT_SUCCESS) {
      free(b);
      free(indirect);
      free(indirect2);
      return e;
    }
    for (int j = 0; j < B_SIZE/SIZEOF_INT; i++) {
      uint32_t indirect2_addr;
      rintle(&indirect2_addr, indirect, INTX(j));
      if ((e = read_block(id, indirect2, vol_addr + indirect2_addr)) != EXIT_SUCCESS) {
	free(b);
	free(indirect);
	free(indirect2);
      	return e;
      }
      int pos = find_freedirent(indirect);
      if (pos != DIR_BLOCKFULL) {
	wintle(entry->d_ino, b, pos);
	memcpy(&b->data[pos + SIZEOF_INT], entry->d_name,
	       sizeof(byte)*TFS_NAME_MAX);
	e = write_block(id, b, vol_addr + ft_inode_baddr);
	free(b);
	free(indirect);
	free(indirect2);
	return e;
      }    
    }
  }
  free(b);
  free(indirect);
  free(indirect2);
  return TFS_ERR_BIGFILE;
}



error
directory_rment (disk_id id, uint32_t vol, DIR directory, const struct dirent *restrict entry) {

  return EXIT_SUCCESS;
}



error
file_pushblock (disk_id id, uint32_t vol_addr, uint32_t inode, uint32_t b_addr) {
  tfs_ftent ftent;
  block b = new_block();
  error e = read_block(id,b,vol_addr + INO_FTBLOCK(inode));
  if(e != EXIT_SUCCESS){
    free(b);
    return e;
  }
  read_ftent(b,INO_BPOS(inode),&ftent);
  int i = 0;
  for(i=0;i<TFS_DIRECT_BLOCKS_NUMBER;i++){
    if(ftent.tfs_direct[i] == 0){
      ftent.tfs_direct[i] = b_addr;
      write_ftent(b,INO_BPOS(inode),&ftent);
      e = write_block(id,b,vol_addr + INO_FTBLOCK(inode));
      free(b);
      return e;
    }
  }
  
  e = read_block(id,b,vol_addr + ftent.tfs_indirect1);
  if(e != EXIT_SUCCESS){
    free(b);
    return e;
  }
  uint32_t value;
  int j = 0;
  for(j=0;j<(B_SIZE/SIZEOF_INT);j++){
    rintle(&value,b,j*SIZEOF_INT);
    if(value == 0){
      wintle(b_addr,b,j*SIZEOF_INT);
      e = write_block(id,b,vol_addr + ftent.tfs_indirect1);
      free(b);
      return e;
    }
  }
  
  block b_i = new_block();
  e = read_block(id,b,vol_addr + ftent.tfs_indirect2);
  if(e != EXIT_SUCCESS){
    free(b_i);
    free(b);
    return e;
  }
  uint32_t addr;
  int k = 0;
  for(k=0;k<(B_SIZE/SIZEOF_INT);k++){
    rintle(&addr,b,k*SIZEOF_INT);
    e = read_block(id,b_i,vol_addr + addr);
    if(e != EXIT_SUCCESS){
      free(b_i);
      free(b);
      return e;
    }
    int l = 0;
    for(l=0;l<(B_SIZE/SIZEOF_INT);l++){
      rintle(&value,b_i,l*SIZEOF_INT);
      if(value == 0){
	wintle(b_addr,b_i,l*SIZEOF_INT);
	e = write_block(id,b_i,vol_addr + addr);
	free(b_i);
	free(b);
	return e;
      }
    }
  }

  free(b_i);
  free(b);
  return TFS_ERR_BIGFILE;
}

static error
get_ftent (disk_id id, uint32_t vol, uint32_t inode, tfs_ftent* ftent){
  error e;
  block b;
  
  if ((e = read_block(id, b, vol+INO_FTBLOCK(inode)))!=-1)
    return e;
  
}

error
file_rmblock (disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr) {
  error e;
  tfs_ftent ftent;
  block b;
  
  if ((e = freeblock_push(id, vol_addr, b_addr))!= EXIT_SUCCESS)
    return e;

  if ((e = read_ftent(b, 
  

  
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
