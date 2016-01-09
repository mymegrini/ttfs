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

#define LASTINT_IDX (TFS_VOLUME_BLOCK_SIZE - 1 - SIZEOF_INT)
#define INDIRECT1_BEGIN (TFS_DIRECT_BLOCKS_NUMBER*TFS_VOLUME_BLOCK_SIZE)
#define INDIRECT2_BEGIN (INDIRECT1_BEGIN + TFS_VOLUME_BLOCK_SIZE*INT_PER_BLOCK)
#define ISINDIRECT2(fileindex) ((fileindex) >= INDIRECT2_BEGIN)
#define ISINDIRECT1(fileindex) ((fileindex) >= INDIRECT1_BEGIN \
				&& ! ISINDIRECT2(fileindex))
#define ISDIRECT(fileindex) ((fileindex) < INDIRECT1_BEGIN)
#define DIRECT_IDX(fileindex) ((fileindex)/TFS_VOLUME_BLOCK_SIZE)
#define INDIRECT1_IDX(fileindex) (((fileindex)/TFS_VOLUME_BLOCK_SIZE	\
				   - TFS_DIRECT_BLOCKS_NUMBER)% INT_PER_BLOCK)
#define INDIRECT2_IDX(fileindex) (((fileindex)/TFS_VOLUME_BLOCK_SIZE \
				    - INDIRECT1_BEGIN)/ INT_PER_BLOCK)
#define LASTBYTE_POS(fileindex) ((fileindex)%TFS_VOLUME_BLOCK_SIZE)

#define TFS_ERR_BIGFILE 100

#define SEM_FBL_T 0
#define SEM_FEL_T 1
#define SEM_FILE_T 2
#define SEM_FBL_S "semb"
#define SEM_FEL_S "semt"
#define SEM_FILE_S "semf"
#define SEM_NAME_LEN NAME_MAX-4

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

struct _index{
  int32_t direct;           /**< current index of direct block (-1 empty) >*/
  int32_t indirect1;        /**< current index in indirect1 block (-1 empty) >*/
  int32_t indirect2;        /**< current index in indirect2 block (-1 empty) >*/
  uint32_t indirect1_addr;   /**< current indirect1 block address >*/
  uint32_t indirect2_addr;   /**< current indirect2 block address >*/
};

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

#define NENTBYBLOCK (TFS_VOLUME_BLOCK_SIZE / TFS_FILE_TABLE_ENTRY_SIZE)
#define INO_FTBLOCK(inode) (1 + ((inode)/NENTBYBLOCK))
#define INO_BPOS(inode) ((inode) % NENTBYBLOCK)

static error
read_ftent(disk_id id, uint32_t vol, uint32_t inode, tfs_ftent * ftent) {
  error e;
  block b = new_block();
  uint32_t entry_pos = TFS_FILE_TABLE_ENTRY_SIZE*INO_BPOS(inode); 
  
  //read filetable block
  if ((e = read_block(id,b,vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS)
    {free(b); return e;}

  //transfer block data to ftent structure
  rintle(&ftent->size, b, entry_pos + TFS_FILE_SIZE_INDEX);
  rintle(&ftent->type, b, entry_pos + TFS_FILE_TYPE_INDEX);
  rintle(&ftent->subtype, b, entry_pos + TFS_FILE_SUBTYPE_INDEX);
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++)
    rintle(&ftent->tfs_direct[i], b, entry_pos + TFS_DIRECT_INDEX(i));
  rintle(&ftent->tfs_indirect1, b, entry_pos + TFS_INDIRECT1_INDEX);
  rintle(&ftent->tfs_indirect2, b, entry_pos + TFS_INDIRECT2_INDEX);
  
  free(b);
  return EXIT_SUCCESS;
}

static error
write_ftent(disk_id id, uint32_t vol, uint32_t inode, const tfs_ftent* ftent) {
  error e;
  block b = new_block();
  uint32_t entry_pos = TFS_FILE_TABLE_ENTRY_SIZE*INO_BPOS(inode); 

  //read filetable block
  if ((e = read_block(id,b,vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS)
    {free(b); return e;}
  
  //transfer ftent data to block
  wintle(ftent->size, b, entry_pos + TFS_FILE_SIZE_INDEX);
  wintle(ftent->type, b, entry_pos + TFS_FILE_TYPE_INDEX);
  wintle(ftent->subtype, b, entry_pos + TFS_FILE_SUBTYPE_INDEX);
  for (int i = 0; i < TFS_DIRECT_BLOCKS_NUMBER; i++)
    wintle(ftent->tfs_direct[i], b, entry_pos + TFS_DIRECT_INDEX(i));
  wintle(ftent->tfs_indirect1, b, entry_pos + TFS_INDIRECT1_INDEX);
  wintle(ftent->tfs_indirect2, b, entry_pos + TFS_INDIRECT2_INDEX);

  //write block back to disk
  if ((e = write_block(id,b,vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS)
    {free(b); return e;}
  
  free(b);
  return EXIT_SUCCESS;
}

static error
write_tfsdescription (disk_id id, uint32_t vol_addr, const tfs_description* desc)
{
  error e;
  block b = new_block();

  //write <desc> data to block
  wintle(desc->magic_number, b, TFS_MAGIC_NUMBER_INDEX);
  wintle(desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX);
  wintle(desc->volume_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX);
  wintle(desc->freeb_count, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX);
  wintle(desc->freeb_first, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX);
  wintle(desc->maxfile_count, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX);
  wintle(desc->freefile_count, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX);
  wintle(desc->freefile_first, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX);

  //write block to disk
  e = write_block(id, b, vol_addr);
  free(b);
  return e;
}

static error
read_tfsdescription (disk_id id, uint32_t vol_addr, tfs_description * desc){
  block b = new_block();
  //read superblock
  error e = read_block(id, b, vol_addr);
  if (e != EXIT_SUCCESS) {
    free(b);
    return e;
  }
  
  //transfer superblock data to tfs_description structure
  rintle(&desc->magic_number, b, TFS_MAGIC_NUMBER_INDEX);
  rintle(&desc->block_size, b, TFS_VOLUME_BLOCK_SIZE_INDEX);
  rintle(&desc->volume_size, b, TFS_VOLUME_BLOCK_COUNT_INDEX);
  rintle(&desc->freeb_count, b, TFS_VOLUME_FREE_BLOCK_COUNT_INDEX);
  rintle(&desc->freeb_first, b, TFS_VOLUME_FIRST_FREE_BLOCK_INDEX);
  rintle(&desc->maxfile_count, b, TFS_VOLUME_MAX_FILE_COUNT_INDEX);
  rintle(&desc->freefile_count, b, TFS_VOLUME_FREE_FILE_COUNT_INDEX);
  rintle(&desc->freefile_first, b, TFS_VOLUME_FIRST_FREE_FILE_INDEX);
  
  free(b);
  return EXIT_SUCCESS;
}

/**
 * @brief Creates a semaphore name using disk name
 * @param[in] id Disk id
 * @param[in] vol volume address
 * @param[in] type type of semaphore (FBL, FEL, FILE)
 * @param[out] name to store semaphone name (maximum size NAME_MAX-4)
 * @return Returns an error if encountered
 */
static error
sem_name(char* name, int type, disk_id id, uint32_t vol, uint32_t inode){
  d_stat stat;
  disk_stat(id, &stat);
  
  switch(type){
  case SEM_FBL_T:
    snprintf(name, SEM_NAME_LEN, "/%s-%s-%d", SEM_FBL_S, stat->name, vol_addr);
    return EXIT_SUCCESS;    
  case SEM_FEL_T:
    snprintf(name, SEM_NAME_LEN, "/%s-%s-%d", SEM_FEL_S, stat->name, vol_addr);
    return EXIT_SUCCESS;    
  case SEM_FILE_T:    
    snprintf(name, SEM_NAME_LEN, "/%s-%s-%d-%d",
	     SEM_FILE_S, stat->name, vol_addr, inode);
    return EXIT_SUCCESS;
  default:    
    return S_WRONGTYPE;    
  }
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

error
index_init(disk_id id, uint32_t vol_addr, uint32_t inode, _index index){
  error e;
  tfs_ftent ftent;

  //read file entry <inode>
  if ((e = read_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;

  //case 1: empty file
  if(ftent.size == 0){
    index->direct = -1;
    index->indirect1 = -1;
    index->indirect2 = -1;
    return EXIT_SUCCESS;
  }

  //case 2: only direct blocks
  if(ISDIRECT((ftent.size)-1)){
    index->direct = DIRECT_IDX(ftent.size);
    index->indirect1 = -1;
    index->indirect2 = -1;
    return EXIT_SUCCESS;
  }

  //case 3: indirect1 block
  if(ISINDIRECT1((ftent.size)-1)){
    index->direct = 10;
    index->indirect1 = ftent.tfs_indirect1;
    index->indirect2 = -1;
    return EXIT_SUCCESS;
  }

  //case 4: indirect2 block
  if(ISINDIRECT2((ftent.size)-1)){
    block b = new_block();
    index->direct = 10;
    index->indirect1 = INDIRECT1_IDX((ftent.size)-1);
    index->indirect2 = INDIRECT2_IDX((ftent.size)-1);
    index->indirect2_addr = ftent.tfs_indirect2;
    //read indirect2 to get address of current indirect1 block
    e = read_block(id, b, vol_addr+index->indirect2_addr);
    rintle(&(index->indirect1_addr), b, index->indirect2);
    free(b);
    return e;
  }

  //case 5: corrupted file (size too big)
  return F_SIZE_CORRUPTED;
}

error
fileblock_add(disk_id id, uint32_t vol_addr, uint32_t inode, _index index)
{
  error e;
  uint32_t b_addr;
  tfs_ftent ftent;
  block b;
    
  //case 1: we need to add direct block
  if (index->direct < TFS_DIRECT_BLOCKS_NUMBER-1){
    //obtain freeblock
    if((e = freeblock_pop(id, vol_addr, &b_addr))!= EXIT_SUCCESS) return e;
    //update <index>
    index->direct++;
    //read and update <ftent>
    if ((e = read_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    ftent.tfs_direct[index->direct]= b_addr;
    //write <ftent> to disk
    if ((e = write_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    return EXIT_SUCCESS;
  }

  //case 2: we need to add first indirect1 block
  if (index->direct == TFS_DIRECT_BLOCKS_NUMBER-1){
    //obtain freeblock for indirect1 block
    if((e = freeblock_pop(id, vol_addr, &b_addr))!= EXIT_SUCCESS) return e;
    //update <index>
    index->direct++;
    index->indirect1_addr = b_addr;
    index->indirect1 = -1; //empty indirect1 block
    //read and update <ftent>
    if ((e = read_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    ftent.tfs_indirect1= b_addr;
    //write <ftent> to disk
    if ((e = write_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    //relaunch function to add data block to indirect1 block (case 5)
    return fileblock_add(id, vol_addr, inode, index);
  }

  //case 3: we need to add indirect2 block
  if (index->indirect1 == INT_PER_BLOCK-1 && index->indirect2 == -1){
    //obtain freeblock for indirect2
    if((e = freeblock_pop(id, vol_addr, &b_addr))!= EXIT_SUCCESS) return e;
    //read and update <ftent>
    if ((e = read_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    ftent.tfs_indirect2= b_addr;
    //write <ftent> to disk
    if ((e = write_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    //relaunch function to add first indirect1 block to indirect2 block (case 4)
    index->indirect1++; //guarantees that we land in case 4
    return fileblock_add(id, vol_addr, inode, index);
  }

  //case 4: we need to a add indirect1 block to indirect2 block
  if (index->indirect1 == INT_PER_BLOCK){
    //obtain freeblock for indirect1
    if((e = freeblock_pop(id, vol_addr, &b_addr))!= EXIT_SUCCESS) return e;
    //update <index>
    index->indirect1_addr = b_addr;
    index->indirect2++;
    index->indirect1 = -1; //empty indirect1 block
    //update indirect2 block
    b = new_block();
    if ((e = read_block(id, b, index->indirect2_addr))!=EXIT_SUCCESS) return e;
    wintle(index->indirect1_addr, b, index->indirect2);
    if ((e = write_block(id, b, index->indirect2_addr))!=EXIT_SUCCESS) return e;
    free(b);
    //relaunch function to add first data block to new indirect1 block (case 5)
    return fileblock_add(id, vol_addr, inode, index);
  }
  
  //case 5: we need to add a data block to current indirect1 block
  if (index->indirect1 < INT_PER_BLOCK-1){
    //obtain free block
    if((e = freeblock_pop(id, vol_addr, &b_addr))!= EXIT_SUCCESS) return e;
    //update <index>
    index->indirect1++;
    //update indirect1 block
    b = new_block();
    if ((e = read_block(id, b, index->indirect1_addr))!=EXIT_SUCCESS) return e;
    wintle(b_addr, b, index->indirect1);
    if ((e = write_block(id, b, index->indirect1_addr))!=EXIT_SUCCESS) return e;
    free(b);
    return EXIT_SUCCESS;
  }  

  //case 6: corrupted index
  return I_CORRUPTED;
}

error
fileblock_rm(disk_id id, uint32_t vol_addr, uint32_t inode, _index index)
{
  error e;
  tfs_ftent ftent;
  block b;
  uint32_t b_addr;

  //case 0: file is empty
  if(index->direct == -1) return F_EMPTY;
    
  //case 1: we need to remove a direct block
  if (index->direct < TFS_DIRECT_BLOCKS_NUMBER){
    //find direct block address
    if ((e = read_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    b_addr = ftent.tfs_direct[index->direct];
    //free direct block
    if((e = freeblock_push(id, vol_addr, b_addr))!= EXIT_SUCCESS) return e;
    //update <index>
    index->direct--;
    return EXIT_SUCCESS;
  }

  //case 2: we need to remove data block from indirect1 block
  if (index->indirect1 > -1){
    //read data block address from indirect1
    b = new_block();
    if ((e = read_block(id, b, index->indirect1_addr))!=EXIT_SUCCESS) return e;
    rintle(&b_addr, b, index->indirect1);
    //free data block
    if((e = freeblock_push(id, vol_addr, index->indirect2))!= EXIT_SUCCESS) return e;
    //update <index>
    index->indirect1--;
    free(b);
    //relaunch function if current indirect1 empty (cases 3 and 5)
    if (index->indirect1== -1) return fileblock_rm(id, vol_addr, inode, index);
    else return EXIT_SUCCESS;
  }

  //case 3: we need to remove indirect1 block from indirect2 block
  if (index->indirect1 == -1 && index->indirect2 > 0){
    //free for indirect1
    if((e = freeblock_push(id, vol_addr, index->indirect1_addr))!= EXIT_SUCCESS)
      return e;
    //update index
    index->indirect2--;
    index->indirect1+= INT_PER_BLOCK;
    //find new indirect1 address from indirect2
    b = new_block();
    if ((e = read_block(id, b, index->indirect2_addr))!=EXIT_SUCCESS) return e;
    rintle(&(index->indirect1_addr), b, index->indirect2);    
    free(b);
    return EXIT_SUCCESS;
  }

  //case 4: we need to remove last indirect1 block from indirect2 block
  if (index->indirect1 == -1 && index->indirect2 == 0){
    //free blocks indirect1 and indirect2
    if((e = freeblock_push(id, vol_addr, index->indirect1_addr))!= EXIT_SUCCESS)
      return e;
    if((e = freeblock_push(id, vol_addr, index->indirect2_addr))!= EXIT_SUCCESS)
      return e;
    //update <index>
    index->indirect2--;
    index->indirect1+= INT_PER_BLOCK;
    //find new indirect1 address from <ftent>
    if ((e = read_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
    index->indirect1 = ftent.tfs_indirect1;
    return EXIT_SUCCESS;
  }
  
  //case 5: we need to remove last indirect1 block
  if (index->indirect1 == -1 && index->direct==TFS_DIRECT_BLOCKS_NUMBER){
    //obtain free block
    if((e = freeblock_pop(id, vol_addr, &b_addr))!= EXIT_SUCCESS) return e;
    //update <index>
    index->direct--;
    return EXIT_SUCCESS;
  }

  //case 6: corrupted index
  return I_CORRUPTED;
}

error
freeblock_push (disk_id id, uint32_t vol_addr, uint32_t b_addr) {
  tfs_description tfs_d;
  error e;

  //read volume superblock
  if ((e = read_tfsdescription(id, vol_addr, &tfs_d))!=EXIT_SUCCESS) return e;
  else {
    block b= new_block();
    
    //fill appropriate TFS_VOLUME_NEXT_FREE_BLOCK_INDEX value in <b>
    if (tfs_d.freeb_count == 0)
      wintle(b_addr, b, TFS_VOLUME_NEXT_FREE_BLOCK_INDEX);
    else
      wintle(tfs_d.freeb_first, b, TFS_VOLUME_NEXT_FREE_BLOCK_INDEX);
    
    //write <b> in <b_addr> volume block
    if ((e = write_block(id, b, vol_addr + b_addr))!=EXIT_SUCCESS)
      {free(b); return e;}
    free(b);
    
    //update <tfs_d> and write it to disk
    tfs_d.freeb_first = b_addr;
    tfs_d.freeb_count++;
    return write_tfsdescription(id, vol_addr, &tfs_d);
  }
}

error
freeblock_pop (disk_id id, uint32_t vol_addr, uint32_t* b_addr) {
  tfs_description desc;
  error e;
  //read volume superblock
  if ((e = read_tfsdescription(id, vol_addr, &desc))!= EXIT_SUCCESS) return e;

  //test if volume is full
  if (desc.freeb_count == 0) return V_FULL;
  else {
    uint32_t nextfreeb;
    block b = new_block();
    
    //read first free volume block
    e = read_block(id, b, vol_addr + desc.freeb_first);
    if (e != EXIT_SUCCESS) {free(b); return e;}
    
    //store block address in <b_addr>
    *b_addr = desc.freeb_first;
    
    //determine next free block index
    e = rintle(&nextfreeb, b, TFS_VOLUME_NEXT_FREE_BLOCK_INDEX);
    free(b);
    if (e != EXIT_SUCCESS) return e;
    
    //test if we have reached the end of the list
    if (nextfreeb == *b_addr && desc.freeb_count >1) return V_FBL_CORRUPTED;
    else {
      //update tfs_description structure
      desc.freeb_first = nextfreeb;
      desc.freeb_count--;
      
      //update superblock on disk
      return write_tfsdescription(id, vol_addr, &desc);
    }
  }
}

error
freefile_push (disk_id id, uint32_t vol_addr, uint32_t inode){
  tfs_description tfs_d;
  error e;

  //read volume superblock
  if ((e = read_tfsdescription(id, vol_addr, &tfs_d))!=EXIT_SUCCESS) return e;
  else {
    tfs_ftent ftent;
    
    //fill appropriate TFS_NEXT_FREE_FILE_ENTRY_INDEX value in <ftent>
    if (tfs_d.freefile_count == 0) ftent.nextfreefile = inode;
    else ftent.nextfreefile = tfs_d.freefile_first;

    //write <ftent> to <inode> file table entry    
    if ((e = write_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;

    //update <tfs_d> and write it to disk
    tfs_d.freefile_first = inode;
    tfs_d.freefile_count++;
    return write_tfsdescription(id, vol_addr, &tfs_d);    
  }
}

error
freefile_pop (disk_id id, uint32_t vol_addr, uint32_t* inode){
  tfs_description tfs_d;
  error e;

  //read volume superblock
  if ((e = read_tfsdescription(id, vol_addr, &tfs_d))!=EXIT_SUCCESS) return e;
  
  //test if file table is full
  if (tfs_d.freefile_count == 0) return FT_FULL;
  else {
    tfs_ftent ftent;

    //read first free file table entry
    e = read_ftent(id, vol_addr, tfs_d.freefile_first, &ftent);
    if (e != EXIT_SUCCESS) return e;

    //store first freefile address in <inode>
    *inode = tfs_d.freefile_first;

    //test if we have reached the end of the list
    if (ftent.nextfreefile == *inode && tfs_d.freefile_count >1) return FT_FEL_CORRUPTED;
    else {
      //update <tfs_d>
      tfs_d.freeb_first = ftent.nextfreefile;
      tfs_d.freeb_count--;

      //update superblock on disk
      return write_tfsdescription(id, vol_addr, &tfs_d);
    }
  }
}

error
file_realloc (disk_id id, uint32_t vol_addr, uint32_t inode, uint32_t size){
  tfs_ftent ftent;
  error e;
  int32_t bsize;
  struct _index index;
  
  
  //read file table entry <inode>
  if ((e = read_ftent(id, vol_addr, inode, &ftent))!= EXIT_SUCCESS) return e;
  
  //compare new size to old size
  if (size == ftent.size) return EXIT_SUCCESS;
  
  //calculate file block size
  bsize = ((ftent.size-1) %TFS_VOLUME_BLOCK_SIZE)+1;
  
  //initialize index
  index_init(id, vol_addr, inode, &index);
  
  if (size > ftent.size){
    //file size increase
    while (bsize*TFS_VOLUME_BLOCK_SIZE<size){
      if (fileblock_add(id, vol_addr, inode, &index)!= EXIT_SUCCESS)
	break;
      else
	bsize++;
    }
    //update <ftent> size
    ftent.size = (bsize*TFS_VOLUME_BLOCK_SIZE>=size
		  ? size
		  : bsize*TFS_VOLUME_BLOCK_SIZE);
    //write <ftent> to disk
    return write_ftent(id, vol_addr, inode, &ftent);
  }
  else {
    //file size decrease
    while ((bsize-1)*TFS_VOLUME_BLOCK_SIZE>=size){
      if (fileblock_rm(id, vol_addr, inode, &index)!= EXIT_SUCCESS)
	break;
      else
	bsize--;
    }
    //update <ftent> size
    ftent.size = ((bsize-1)*TFS_VOLUME_BLOCK_SIZE<size
		  ? size
		  : (bsize-1)*TFS_VOLUME_BLOCK_SIZE);
    //write <ftent> to disk
    return write_ftent(id, vol_addr, inode, &ftent);
  }
}

error
find_addr(disk_id id, uint32_t vol, uint32_t inode,
	  uint32_t b_file_addr, uint32_t* b_addr){
  block b;
  tfs_ftent ftent;
  error e;
  
  //read <inode> file table entry
  if ((e = read_ftent(id, vol, inode, &ftent))!=EXIT_SUCCESS) return e;
  
  //test if block address is reasonable
  if (b_file_addr>((ftent.size)-1)/TFS_VOLUME_BLOCK_SIZE)
    return B_OUTOFBOUNDS;
  
  //from tfs_indirect
  if (b_file_addr<TFS_DIRECT_BLOCKS_NUMBER){
    //read b_addr from file table entry
    *b_addr = ftent.tfs_direct[b_file_addr];
    return EXIT_SUCCESS;
  }
  
  b = new_block();
  //from tfs_indirect1
  b_file_addr-=TFS_DIRECT_BLOCKS_NUMBER;
  if (b_file_addr<TFS_INDIRECT1_CAPACITY){
    //read tfs_indirect1 block
    if ((e = read_block(id, b, vol+ftent.tfs_indirect1))!=EXIT_SUCCESS)
      {free(b); return e;}
    //read b_addr from block
    rintle(b_addr, b, b_file_addr);
    free(b);
    return EXIT_SUCCESS;
  }

  //from tfs_indirect2
  b_file_addr-=TFS_INDIRECT1_CAPACITY;
  if (b_file_addr<TFS_INDIRECT2_CAPACITY){
    uint32_t indirect1_addr;
    //read tfs_indirect2 block
    if ((e = read_block(id, b, vol+ftent.tfs_indirect2))!=EXIT_SUCCESS)
      {free(b); return e;}
    //read tfs_indirect1 address from block
    rintle(&indirect1_addr, b, b_file_addr*(TFS_VOLUME_BLOCK_SIZE/INT_SIZE));
    //read tfs_indirect1 block
    if ((e = read_block(id, b, indirect1_addr))!=EXIT_SUCCESS)
      {free(b); return e;}
    //read b_address from block
    rintle(b_addr, b, b_file_addr%(TFS_VOLUME_BLOCK_SIZE/INT_SIZE));
    free(b);
    return EXIT_SUCCESS;    
  }
  free(b);
  return F_SIZE_CORRUPTED;
}

#define TFS_ERR_OPERATION 214
#define DIR_BLOCKFULL -1

error
directory_pushent (disk_id id, uint32_t vol_addr, uint32_t inode,
		   const struct dirent *entry){
  tfs_description desc;
  tfs_ftent ftent;
  block b = new_block();
  const uint32_t ft_inode_baddr = INO_FTBLOCK(inode);
  
  error e = read_tfsdescription(id, vol_addr, &desc);
  if (e != EXIT_SUCCESS)
    return e;

  //read <inode> file table entry
  if ((e = read_ftent(id, vol_addr, inode, &ftent))!=EXIT_SUCCESS) return e;
  
  // It might be useless to check the type, but for now there is a test
  if (ftent.type != TFS_DIRECTORY_TYPE) {
    free(b);
    return TFS_ERR_OPERATION;
  }
  // DIRECTORY FULL
  if (ftent.size == TFS_FILE_MAX_SIZE){
    free(b);
    return F_FULL;
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
	e = freeblock_pop(id, vol_addr, &b_addr);
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
	freeblock_pop(id, vol_addr, &b_addr);
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
    uint32_t b_indir1_pos = INDIRECT1_IDX(ftent.size);
    rintle(&b_indir2_addr, b_indir1, b_indir1_pos);
    // provide new indir2 block if needed
    if (b_indir2_addr == 0)
      {
	if ((e = freeblock_pop(id, vol_addr, &b_indir2_addr)) != EXIT_SUCCESS)
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
    uint32_t b_indir2_pos = INDIRECT2_IDX(ftent.size);
    rintle(&b_addr, b_indir2, b_indir2_pos);
    block b_data;
    // provide new data block if needed
    if (b_indir2_addr == 0) {
      if ((e = freeblock_pop(id, vol_addr, &b_addr)) != EXIT_SUCCESS) {
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


////////////////////////////////////////////////////////////////////////////////
// TRASH
////////////////////////////////////////////////////////////////////////////////
/*
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


static error
put_volume_addr(disk_id id, uint32_t vol, uint32_t inode,
		uint32_t b_file_addr, uint32_t b_addr){
  block b=new_block();;
  tfs_ftent ftent;
  error e;
  
  //read filetable block
  if ((e = read_block(id,b,vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS)
  //read file table entry from block
  read_ftent(b, INO_BPOS(inode), ftent);
  
  //to tfs_indirect
  if (b_file_addr<TFS_DIRECT_BLOCKS_NUMBER){
      {free(b); return e;}
    //write b_addr to file table entry
    ftent->tfs_direct[b_addr] = b_addr;
    //write file entry to block
    write_ftent(b, INO_BPOS(inode), ftent);
    //write block to disk
    if ((e = write_block(id,b,vol+INO_FTBLOCK(inode)))!=EXIT_SUCCESS)
      {free(b); return e;}
    free(b);
    return EXIT_SUCCESS;
  }
  
  //to tfs_indirect1
  b_file_addr-=TFS_DIRECT_BLOCKS_NUMBER;
  if (b_file_addr<TFS_INDIRECT1_CAPACITY){
    //read tfs_indirect1 block
    if ((e = read_block(id, b, vol+ftent->tfs_inditect1))!=EXIT_SUCCESS)
      {free(b); return e;}
    //write b_addr to block
    wintle(b_addr, b, b_file_addr);
    //write block to disk
    if ((e = write_block(id, b, vol+ftent->tfs_inditect1))!=EXIT_SUCCESS)
      {free(b); return e;}
    free(b);
    return EXIT_SUCCESS;
  }

  //to tfs_indirect2
  b_file_addr-=TFS_INDIRECT1_CAPACITY;
  if (b_file_addr<TFS_INDIRECT2_CAPACITY){
    uint32_t indirect1_addr;
    //read tfs_indirect2 block
    if ((e = read_block(id, b, vol+ftent->tfs_inditect2))!=EXIT_SUCCESS)
      {free(b); return e;}
    //read tfs_indirect1 address
    rintle(indirect1_addr, b, b_file_addr*(TFS_VOLUME_BLOCK_SIZE/INT_SIZE));
    //read tfs_indirect1 block
    if ((e = read_block(id, b, indirect1_addr))!=EXIT_SUCCESS)
      {free(b); return e;}
    //write b_addr to block
    wintle(b_addr, b, b_file_addr%(TFS_VOLUME_BLOCK_SIZE/INT_SIZE));
    //write block to disk
    if ((e = write_block(id, b, indirect1_addr))!=EXIT_SUCCESS)
      {free(b); return e;}
    return EXIT_SUCCESS;    
  }
  return WRONG_BLOCK_FILE_ADDRESS;
}

 */

//////////////////////////////////////////////////////////////////////
// $Log:$
//

