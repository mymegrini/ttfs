#include "tfs.h"
#include "tfsll.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>


////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////
#define DIRECTORY_SIZEMIN (2*TFS_DIRECTORY_ENTRY_SIZE)
////////////////////////////////////////////////////////////////////////////////
// ERRORS
////////////////////////////////////////////////////////////////////////////////
#define TFS_ERRPATH_NODISK  215
#define TFS_ERRPATH_HOST    216
#define TFS_ERRPATH_PARTID  217
#define TFS_ERRLOCK         300
#define TFS_ERRPATH         201


////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////
struct _DIR {
  int             fd;
  uint32_t        ino;
  uint32_t        f_offset;
  uint32_t        b_offset;
  struct dirent   buf[32];
};
////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


/**
 * 
 * 
 * 
 * 
 */
int tfs_mkdir(const char *path, mode_t mode)
{
  error e;
  char *parent_path = strdup(path);
  char *last_el;
  if ((e = path_split(parent_path, &last_el)) != EXIT_SUCCESS) {
    free(parent_path);
    return e;
  }
  DIR *parent = opendir(parent_path);
  if (parent == NULL) {
    free(parent_path);
    return TFS_ERRPATH;
  }
  const disk_id  id       = _filedes[parent->fd].id;
  const uint32_t vol_addr = _filedes[parent->fd].vol_addr;
  uint32_t ino_new;
  // Get a new inode
  e = freefile_pop(id, vol_addr, &ino_new);
  if (e != EXIT_SUCCESS) {
    closedir(parent);
    free(parent_path);
    return e;
  }
  uint32_t datablock_addr;
  // Get a data block
  e = freeblock_pop(id, vol_addr,
		    &datablock_addr);
  if (e != EXIT_SUCCESS) {
    closedir(parent);
    freefile_push(id, vol_addr, ino_new);
    free(parent_path);
    return e;
  }
  block datablock = new_block();
  // Write entry "."
  wintle(ino_new, datablock, 0);
  datablock->data[INT_SIZE] = '.';
  // entry ".."
  wintle(_filedes[parent->fd].ino, datablock, TFS_DIRECTORY_ENTRY_SIZE);
  datablock->data[TFS_DIRECTORY_ENTRY_SIZE + INT_SIZE] = '.';
  datablock->data[TFS_DIRECTORY_ENTRY_SIZE + INT_SIZE + 1] = '.';
  // write data block
  e = write_block(id, datablock, vol_addr + datablock_addr);
  if (e != EXIT_SUCCESS) {
    closedir(parent);
    freeblock_push(id, vol_addr, datablock_addr);
    freefile_push(id, vol_addr, ino_new);
    free(parent_path);
  }
  /* TODO:  */
  return 0;
}

/**
 * 
 * 
 * 
 */
int tfs_rmdir(const char *path){
  return 0;
}

/**
 * 
 * 
 * 
 * 
 */
int tfs_rename(const char *old, const char *new){
  return 0;
}

/**
 * 
 * 
 * 
 * 
 * 
 */
int tfs_open(const char *name,int oflag, ...){
  return 0;
}

/**
 * 
 * 
 * 
 * 
 * 
 * 
 */uint32_t 
ssize_t tfs_read(int fildes,void *buf,size_t nbytes){
  return 0;
}

/**
 * 
 * 
 * 
 * 
 * 
 * 
 */
ssize_t tfs_write(int fildes,void *buf,size_t nbytes){
  return 0;
}

/**
 * 
 * 
 * 
 */
int tfs_close(int fildes){
  return 0;
}

/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
off_t tfs_lseek(int fildes,off_t offset,int whence){
  return 0;
}

/**
 * 
 * 
 * 
 */
DIR *opendir(const char *filename){
  return NULL;
}

/**
 * 
 * 
 * 
 */
struct dirent *readdir(DIR *dirp){
  return NULL;
}

/**
 * 
 * 
 * 
 */
void rewinddir(DIR *dirp){
  
}

/**
 * 
 * 
 * 
 */
int closedir(DIR *dirp){
  return 0;
}





