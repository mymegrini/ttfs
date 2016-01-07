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
#define INO_FTBLOCK(inode) (1 + ((inode)/NENTBYBLOCK))
#define INO_BPOS(inode) ((inode) % NENTBYBLOCK)


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

static error
read_ftent (block b, const uint32_t bpos, tfs_ftent * ftent) {
  error e;
  const uint32_t entry_pos = TFS_FILE_TABLE_ENTRY_SIZE*bpos; 
  if ((e = rintle(&ftent->size, b, entry_pos + TFS_FILE_SIZE_INDEX))!=EXIT_SUCCESS) return e;
  if ((e = rintle(&ftent->type, b, entry_pos + TFS_FILE_TYPE_INDEX))!=EXIT_SUCCESS) return e;
  if ((e = rintle(&ftent->subtype, b, entry_pos + TFS_FILE_SUBTYPE_INDEX))!=EXIT_SUCCESS) return e;
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++)
    if ((e = rintle(&ftent->tfs_direct[i], b, entry_pos + TFS_DIRECT_INDEX(i)))!=EXIT_SUCCESS) return e;
  if ((e = rintle(&ftent->tfs_indirect1, b, entry_pos + TFS_INDIRECT1_INDEX))!=EXIT_SUCCESS) return e;
  if ((e = rintle(&ftent->tfs_indirect2, b, entry_pos + TFS_INDIRECT2_INDEX))!=EXIT_SUCCESS) return e;
  return EXIT_SUCCESS;
}

static error
get_ftent (disk_id id, uint32_t vol, uint32_t inode, tfs_ftent* ftent){
  error e;
  block b = new_block();
  if ((e = read_block(id, b, vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = read_ftent(b, INO_BPOS(inode), ftent))!=EXIT_SUCCESS){free(b); return e;}
  free(b);
  return EXIT_SUCCESS;
}

static error
write_ftent (block b, const uint32_t bpos, tfs_ftent * ftent) {
  error e;
  const uint32_t entry_pos = TFS_FILE_TABLE_ENTRY_SIZE*bpos;

  if ((e = wintle(ftent->size, b, entry_pos + TFS_FILE_SIZE_INDEX))!=EXIT_SUCCESS) return e;
  if ((e = wintle(ftent->type, b, entry_pos + TFS_FILE_TYPE_INDEX))!=EXIT_SUCCESS) return e;
  if ((e = wintle(ftent->subtype, b, entry_pos + TFS_FILE_SUBTYPE_INDEX))!=EXIT_SUCCESS) return e;
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++)
    if ((e = wintle(ftent->tfs_direct[i], b, entry_pos + TFS_DIRECT_INDEX(i)))!=EXIT_SUCCESS) return e;
  if ((e = wintle(ftent->tfs_indirect1, b, entry_pos + TFS_INDIRECT1_INDEX))!=EXIT_SUCCESS) return e;
  if ((e = wintle(ftent->tfs_indirect2, b, entry_pos + TFS_INDIRECT2_INDEX))!=EXIT_SUCCESS) return e;
  return EXIT_SUCCESS;
}

static error
put_ftent (disk_id id, uint32_t vol, uint32_t inode, tfs_ftent* ftent){
  error e;
  block b = new_block();
  
  if ((e = read_block(id, b, vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = write_ftent(b, INO_BPOS(inode), ftent))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = write_block(id, b, vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS){free(b); return e;}
  free(b);
  return EXIT_SUCCESS;
}


static error
write_tfsdescription (const disk_id id, const uint32_t vol_addr, const tfs_description * desc)
{
  error e;
  block b = new_block();
  if ((e = wintle(desc->magic_number, b, 0))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(desc->volume_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(desc->freefile_count, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(desc->freeb_first, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(desc->maxfile_count, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(desc->freefile_count, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(desc->freefile_first, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = write_block(id, b, vol_addr))!=EXIT_SUCCESS){free(b); return e;}
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
  if ((e = rintle(&desc->magic_number, b, 0))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = rintle(&desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = rintle(&desc->volume_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = rintle(&desc->freefile_count, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = rintle(&desc->freeb_first, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = rintle(&desc->maxfile_count, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = rintle(&desc->freefile_count, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = rintle(&desc->freefile_first, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX))!=EXIT_SUCCESS){free(b); return e;}
  free(b);
  return EXIT_SUCCESS;
}


error
freeblock_push (const disk_id id, const uint32_t vol_addr, const uint32_t b_addr) {
  tfs_description tfs_d;
  error e;
  block b = new_block();
  
  if ((e = read_tfsdescription(id, vol_addr, &tfs_d))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = wintle(tfs_d.freeb_first, b, LASTINT_IDX))!=EXIT_SUCCESS){free(b); return e;}
  if ((e = write_block(id, b, vol_addr + b_addr))!=EXIT_SUCCESS){free(b); return e;}
  free(b);
  tfs_d.freeb_first = b_addr;
  return write_tfsdescription(id, vol_addr, &tfs_d);
}


#define TFS_FULL 200
error
freeblock_rm (const disk_id id, const uint32_t vol_addr, uint32_t * b_addr) {
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
  *b_addr = desc.freeb_first;
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
  for (int i = 0; i < TFS_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
    if (b->data[TFS_DIRECTORY_ENTRY_INDEX(i) + SIZEOF_INT] == 0) {
      // free entry
      return TFS_DIRECTORY_ENTRY_INDEX(i);
    }
  }
  return DIR_BLOCKFULL;
}

#define INDIRECT1_BEGIN (TFS_DIRECT_BLOCKS_NUMBER*B_SIZE)
#define INDIRECT2_BEGIN (INDIRECT1_BEGIN + B_SIZE*B_SIZE/INT_SIZE)
#define ISINDIRECT2(fileindex) ((fileindex) >= INDIRECT2_BEGIN)
#define ISINDIRECT1(fileindex) ((fileindex) >= INDIRECT1_BEGIN	\
				&& ! ISINDIRECT2(fileindex))
#define ISDIRECT(fileindex) ((fileindex) < INDIRECT1_BEGIN)
#define DIRECT_IDX(fileindex) ((fileindex)/B_SIZE)
#define INDIRECT1_IDX(fileindex) ((fileindex)/B_SIZE - TFS_DIRECT_BLOCKS_NUMBER)
#define INDIRECT2_IDX1(fileindex) (((fileindex)/B_SIZE - INDIRECT1_BEGIN) \
				   / B_SIZE/INT_SIZE)
#define INDIRECT2_IDX2(fileindex) (((fileindex)/B_SIZE - INDIRECT1_BEGIN) \
				   % B_SIZE/INT_SIZE)
#define LASTBYTE_POS(fileindex) ((fileindex)%B_SIZE)
#define TFS_FILE_FULL 123
error
directory_pushent (const disk_id id, const uint32_t vol_addr,
		   const uint32_t inode, const struct dirent *entry )
{
  tfs_description desc;
  error e = read_tfsdescription(id, vol_addr, &desc);
  if (e != EXIT_SUCCESS)
    return e;

  // Read filetable entry
  const uint32_t ft_inode_baddr = INO_FTBLOCK(inode);
  const uint32_t inode_bpos = INO_BPOS(inode);
  block b = new_block();
  read_block(id, b, vol_addr + ft_inode_baddr);
  tfs_ftent ftent;
  read_ftent(b, inode_bpos, &ftent);
  // It might be useless to check the type, but for now there is a test
  if (ftent.type != TFS_DIRECTORY_TYPE) {
    free(b);
    return TFS_ERR_OPERATION;
  }
  // DIRECTORY FULL
  if (ftent.size == TFS_FILE_MAX_SIZE){
    free(b);
    return TFS_FILE_FULL;
  }
  // actualize size in the cache for further writing
  // before incrementation, size at least one time TFS_DIRECTORY_ENTRY_SIZE
  // lesser than TFS_FILE_MAX_SIZE
  ftent.size += TFS_DIRECTORY_ENTRY_SIZE;
  

  uint32_t b_addr;   // data block adress where to put the entry
  uint32_t bpos = LASTBYTE_POS(inode);    // relative inode pos in the data
					  // block 
  // Add entry to a direct block
  if (ISDIRECT(ftent.size))
    {
      int isnewblock = 0;
      uint32_t direct_idx = DIRECT_IDX(ftent.size);    // index in tfs_direct
      // if a new block is needed, add it to tfs_direct
      if (ftent.tfs_direct[direct_idx] == 0) {
	isnewblock = 1;
	e = freeblock_rm(id, vol_addr, &b_addr);
	if (e != EXIT_SUCCESS) {
	  free(b);
	  return e;
	}
	ftent.tfs_direct[direct_idx] = b_addr;
      }
      // else read the block
      else {
	b_addr = ftent.tfs_direct[DIRECT_IDX(ftent.size)];
      }
      // read the direct block
      block db = new_block();
      if ((e = read_block(id, db, vol_addr + b_addr)) != EXIT_SUCCESS) {
	if (isnewblock)
	  freeblock_push(id, vol_addr, b_addr);
	free(b);
	free(db);
	return e;
      }
      // write the entry
      wintle(entry->d_ino, db, bpos);
      memcpy(&db->data[bpos+INT_SIZE], entry->d_name, TFS_NAME_MAX);
      if ((e = write_block(id, db, vol_addr + b_addr)) != EXIT_SUCCESS
	  ||(e = write_block(id, b, vol_addr + ft_inode_baddr)) != EXIT_SUCCESS)
	{
	  if (isnewblock)     // don't lose it from the filesystem
	    freeblock_push(id, vol_addr, b_addr);
	  free(b);
	  free(db);
	  return e;
	}
      free(b);
      free(db);
      return EXIT_SUCCESS;
    }
  // Entry must be written in IN
  if (ISINDIRECT1(ftent.size))
    {
      block b_indir1 = new_block();
      if ((e = read_block(id, b_indir1, vol_addr + ftent.tfs_indirect1))
	  != EXIT_SUCCESS)
	{
	  free(b);
	  free(b_indir1);
	  return e;
	}
      uint32_t indir_b_pos = INDIRECT1_IDX(ftent.size);
      rintle(&b_addr, b_indir1, indir_b_pos);
      // provide new block if needed
      if (b_addr == 0) {
	freeblock_rm(id, vol_addr, &b_addr);
	wintle(b_addr, b_indir1, indir_b_pos);
	if ((e = write_block(id, b_indir1, vol_addr + b_addr) != EXIT_SUCCESS))
	  {
	    freeblock_push(id, vol_addr, b_addr);
	    free(b);
	    free(b_indir1);
	    return e;
	  }
      }
      free(b_indir1);
      block b_data = new_block();
      if ((e = read_block(id, b_data, vol_addr + b_addr)) != EXIT_SUCCESS) {
	free(b);
	free(b_data);
	return e;
      }
      wintle(entry->d_ino, b_data, bpos);
      memcpy(&b_data->data[bpos+INT_SIZE], entry->d_name, TFS_NAME_MAX);
      // write modifications
      if ((e = write_block(id, b_data, vol_addr + b_addr)) != EXIT_SUCCESS
	  ||(e = write_block(id, b, vol_addr + ft_inode_baddr)) != EXIT_SUCCESS)
	{
	  free(b);
	  free(b_data);
	  return e;
	}
      free(b);
      free(b_data);
      return EXIT_SUCCESS;
    }
  // Indirect 2
  if (ISINDIRECT2(ftent.size)) {
    block b_indir1 = new_block();
    block b_indir2;
    uint32_t  b_indir2_addr;
    if ((e = read_block(id, b_indir1, vol_addr + ftent.tfs_indirect2))
	!= EXIT_SUCCESS)
      {
	free(b);
	free(b_indir1);
	return e;
      }
    uint32_t b_indir1_pos = INDIRECT2_IDX1(ftent.size);
    rintle(&b_indir2_addr, b_indir1, b_indir1_pos);
    // provide new indir2 block if needed
    if (b_indir2_addr == 0)
      {
	if ((e = freeblock_rm(id, vol_addr, &b_indir2_addr)) != EXIT_SUCCESS)
	  {
	    free(b);
	    free(b_indir1);
	    return e;
	  }
	wintle(b_indir2_addr, b_indir1, b_indir1_pos);
	// fail to actualize indirect1
	if ((e = write_block(id, b_indir1, vol_addr + ftent.tfs_indirect2))
	     != EXIT_SUCCESS) {
	  freeblock_push(id, vol_addr, b_indir2_addr);
	  free(b);
	  free(b_indir1);
	  return e;
	}         
      }
    free(b_indir1); // don't need it anymore since we have recovered
		    // b_indir2_addr, move one b_indir2
    // read b_indir2
    b_indir2 = new_block();
    if ((e == read_block(id, b_indir2, vol_addr + b_indir2_addr))
	!= EXIT_SUCCESS)
      {
	free(b);
	free(b_indir2);
	return e;
      }
    uint32_t b_indir2_pos = INDIRECT2_IDX2(ftent.size);
    rintle(&b_addr, b_indir2, b_indir2_pos);
    block b_data;
    // provide new data block if needed
    if (b_indir2_addr == 0) {
      if ((e = freeblock_rm(id, vol_addr, &b_addr)) != EXIT_SUCCESS) {
	free(b);
	free(b_indir2);
	return e;
      }
      wintle(b_addr, b_indir2, b_indir2_pos);
      if ((e = write_block(id, b_indir2, vol_addr + b_indir2_addr))
	  != EXIT_SUCCESS)
	{
	  freeblock_push(id, vol_addr, b_addr);
	  free(b);
	  free(b_indir2);
	  return e;
	}
    }
    free(b_indir2); // don't need it anymore, move on data block
    b_data = new_block();
    if ((e = read_block(id, b_data, vol_addr + b_addr)) != EXIT_SUCCESS) {
      free(b);
      free(b_data);
      return e;
    }
    // finally write the entry !!!
    wintle(entry->d_ino, b_data, bpos);
    memcpy(&b_data->data[bpos+INT_SIZE], entry->d_name, TFS_NAME_MAX);
    // write modifications in the disk
    if ((e = write_block(id, b_data, vol_addr + b_addr)) != EXIT_SUCCESS
	|| (e = write_block(id, b, vol_addr + ft_inode_baddr)) != EXIT_SUCCESS)
      {
	free(b);
	free(b_data);
	return e;
      }
    free(b);
    free(b_data);
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}



error
directory_rment (disk_id id, uint32_t vol, const struct dirent *restrict entry) {
  
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

error
file_rmblock (disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr) {
  error e;
  tfs_ftent ftent;
  //block b;
  
  if ((e = freeblock_push(id, vol, b_addr))!=EXIT_SUCCESS) return e;
  if ((e = get_ftent(id, vol, inode, &ftent))!=EXIT_SUCCESS) return e;
  if ((e = put_ftent(id, vol, inode, &ftent))!=EXIT_SUCCESS) return e;
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
