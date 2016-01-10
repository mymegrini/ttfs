#include "tfs.h"
#include "tfsll.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// ERRORS
////////////////////////////////////////////////////////////////////////////////
#define TFS_ERRPATH_NODISK  215
#define TFS_ERRPATH_HOST    216
#define TFS_ERRPATH_PARTID  217
#define TFS_ERRLOCK         300
#define TFS_ERRPATH         201
////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


/**
 * 
 * 
 * 
 * 
 */
int tfs_mkdir(const char *path, mode_t mode){
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
 */
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





error
tfs_fileno (char *path, uint32_t *ino)
{
  char *last_el;
  error e = path_split(path, &last_el);
  if (e != EXIT_SUCCESS)
    return e;
  char *token;
  // token : disk
  if ((path_follow(NULL, &token)) != EXIT_SUCCESS)
    return TFS_ERRPATH_NODISK;
  if (ISHOST(token))
    return TFS_ERRPATH_HOST;
  disk_id id;
  if ((e = start_disk(token, &id)) != EXIT_SUCCESS)
    return e;
  // token : part index
 if ((e = path_follow(NULL, &token)) != EXIT_SUCCESS) {
    stop_disk(id);
    return e;
  }
  // conversion to uint32_t
  long long int partid = atou(token);
  if (partid < 0) {
    stop_disk(id);
    return TFS_ERRPATH_PARTID;
  }
  // recover volume adress
  uint32_t vol_addr;
  if ((e = p_index(id, partid, &vol_addr)) != EXIT_SUCCESS) {
    stop_disk(id);
    return e;
  }
  DIR *parent = opendir(path);
  if (parent == NULL) {
    stop_disk(id);
    return TFS_ERRPATH;
  }
  struct dirent *ent;
  // look for the last element
  while ((ent = readdir(parent)) != NULL)
    // found
    if (strcmp(ent->d_name, last_el) == 0) {
      *ino = ent->d_ino;
      closedir(parent);
      stop_disk(id);
      return EXIT_SUCCESS;
    }
  // Not found
  closedir(parent);
  stop_disk(id);
  return TFS_ERRPATH;  
}
