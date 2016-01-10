#include "tfs.h"
#include "tfsll.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////

#define DIRECTORY_SIZEMIN (2*TFS_DIRECTORY_ENTRY_SIZE)
#define ISFILDES(fildes) ((fildes)<TFS_FILE_MAX && _filedes[fildes])

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

static struct dirent*
find_entry (DIR *dir, char *name)
{
  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL)
    if (strncmp(ent->d_name, name, TFS_NAME_MAX) == 0)
      return ent;
  return NULL;    
}

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
  // TEST HOST
  char *disk;
  if ((e = path_follow(NULL, &disk)) == TFS_PATHLEAF) {
    free(parent_path);
    return TFS_ERRPATH;
  }
  // HOST CASE
  if (ISHOST(disk)) {
    e = mkdir(&path[PATH_FPFXLEN+4], mode);
    free(parent_path);
    return e;
  }
  // TFS CASE
  // open directory
  DIR *parent = opendir(parent_path);
  if (parent == NULL) {
    free(parent_path);
    return TFS_ERRPATH;
  }
  // look for a possible existing entry
  if (find_entry(parent, last_el) != NULL) {
    free(parent_path);
    closedir(parent);
    return TFS_EXISTINGENTRY;
  }
  // create directory content
  const disk_id  id       = _filedes[parent->fd]->id;
  const uint32_t vol_addr = _filedes[parent->fd]->vol_addr;
  int fd = file_open(id, vol_addr, 0, O_CREAT, TFS_DIRECTORY_TYPE, 0);
  if (fd == -1) {
    free(parent_path);
    return TFS_ERROPEN;
  }
  char new_entry_buf[TFS_DIRECTORY_ENTRY_SIZE*2];
  // fill entry with 0
  memset(new_entry_buf, 0, TFS_DIRECTORY_ENTRY_SIZE*2);
  
  //////////////////////////////////////////////////////////////////////////////
  // NEED A CHECK
  rintle((uint32_t *)new_entry_buf, (block)&(_filedes[fd]->inode), 0);
  rintle((uint32_t *)&new_entry_buf[TFS_DIRECTORY_ENTRY_SIZE],
	 (block)&(filedes[parent->ino]), 0);
  //////////////////////////////////////////////////////////////////////////////
  new_entry_buf[INT_SIZE                               ] = '.';
  new_entry_buf[TFS_DIRECTORY_ENTRY_SIZE + INT_SIZE    ] = '.';
  new_entry_buf[TFS_DIRECTORY_ENTRY_SIZE + 1 + INT_SIZE] = '.';
  e = tfs_write(fd, new_entry_buf, TFS_DIRECTORY_ENTRY_SIZE);
  // upon error, remove created file
  if (e != EXIT_SUCCESS) {
    freefile_push(id, vol_addr, _filedes[fd]->inode);
    tfs_close(fd);
    closedir(parent);
    free(parent_path);
    return e;
  }
  close(fd);
  // add entry to parent directory
  // fill name part with 0
  memset(&new_entry_buf[INT_SIZE], 0, TFS_NAME_MAX);
  // copy entry name
  strncpy(&new_entry_buf[INT_SIZE], last_el, TFS_NAME_MAX);
  e = tfs_write(fd, new_entry_buf, TFS_DIRECTORY_ENTRY_SIZE);
  // upon error, remove created file
  if (e != EXIT_SUCCESS)
    freefile_push(id, vol_addr, _filedes[fd]->inode);
  // close
  tfs_close(fd);
  closedir(parent);
  free(parent_path);
  return e;
}

/**
 * 
 * 
 * 
 */
int tfs_rmdir(const char *path){
  error e;
  char *parent_path = strdup(path);
  char *last_el;
  if ((e = path_split(parent_path, &last_el)) != EXIT_SUCCESS) {
    free(parent_path);
    return e;
  }
  // TEST HOST
  char *disk;
  if ((e = path_follow(NULL, &disk)) == TFS_PATHLEAF) {
    free(parent_path);
    return TFS_ERRPATH;
  }
  // HOST CASE
  if (ISHOST(disk)) {
    e = rmdir(&path[PATH_FPFXLEN+4]);
    free(parent_path);
    return e;
  }
  // TFS CASE
  // open directory
  DIR *parent = opendir(parent_path);
  if (parent == NULL) {
    free(parent_path);
    return TFS_ERRPATH;
  }
  // look for the existing entry
  struct dirent *ent;
  if ((ent = find_entry(parent, last_el)) == NULL) {
    free(parent_path);
    closedir(parent);
    return TFS_NOENTRY;
  }
  else directory_pushent(const disk_id id, const uint32_t vol_addr, const uint32_t inode, const struct dirent *entry)
  
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
 */
int
tfs_lock (int fildes){
  if (ISFILDES(fildes)) {
    if (sem_wait(_filedes[fildes]->sem)) return TFS_LOCK_FAIL;
    else return EXIT_SUCCESS;
  } else return TFS_BAD_FILDES;
}

/**
 * 
 *
 * 
 * 
 */
int
tfs_unlock (int fildes){
  if (ISFILDES(fildes)) {
    if (sem_post(_filedes[fildes]->sem)) return TFS_UNLOCK_FAIL;
    else return EXIT_SUCCESS;
  } else return TFS_BAD_FILDES;
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





