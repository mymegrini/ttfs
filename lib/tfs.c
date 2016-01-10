#include "tfs.h"
#include "tfsll.h"
#include "ll.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>

////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////

#define DIRECTORY_SIZEMIN (2*TFS_DIRECTORY_ENTRY_SIZE)
#define ISFILDES(fildes) ((fildes)<TFS_FILE_MAX && _filedes[fildes])
#define B_FILE_ADDR(offset) ((offset)/TFS_VOLUME_BLOCK_SIZE)

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

static struct entry*
find_entry (DIR *dir, char *name)
{
  struct entry* ent;
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
  // create entry
  const disk_id  id       = _filedes[parent->fd]->id;
  const uint32_t vol_addr = _filedes[parent->fd]->vol_addr;
  uint32_t ino_new;
  int fd = file_open(id, vol_addr, 0, O_CREAT, TFS_DIRECTORY_TYPE, 0);
  if (fd == -1) {
    return 
  }
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
  wintle(_filedes[parent->fd]->inode, datablock, TFS_DIRECTORY_ENTRY_SIZE);
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
 * @param oflag O_RDONLY|O_WRONLY|O_RDWR [O_CREAT|O_APPEND|O_TRUNC]
 * 
 */
int tfs_open(const char *name, int oflag, ...){
  if ((oflag&O_RDONLY)||(oflag&O_WRONLY)||(oflag&O_RDWR)){
    uint32_t inode;
    char* path = strdup(name);
    char* fname;
    char* elem;
    disk_id id;
    uint32_t vol_addr;
    int fd;
    
    //find file inode
    errnum = find_inode(path, &inode);
    
    //case 1: create new file
    if (errnum == TFS_F_NOTFOUND && (oflag|O_CREAT)){
      uint32_t dino;
      struct dirent entry;
      
      //obtaining name and parent path
      if ((errnum = path_split(path, &fname))!=EXIT_SUCCESS)
	{free(path);return -1;}
      
      //test for failure
      if ((errnum= find_inode(path, &dino))!=EXIT_SUCCESS)
	{free(path);return -1;}
      
      //get disk id
      if ((errnum = path_follow(NULL, &elem))!=EXIT_SUCCESS)
	{free(path);return -1;}
      strcat(elem, DEF_EXT);
      if ((errnum = start_disk(elem, &id))!=EXIT_SUCCESS)
	{free(path);return -1;}      
      
      //get volume address
      if ((errnum = path_follow(NULL, &elem))!=EXIT_SUCCESS)
	{free(path);return -1;}      
      if (atou(elem)<0)
	{free(path);return -1;}
      if ((errnum = p_index(id, atou(elem), &vol_addr))!=EXIT_SUCCESS)
	{free(path);return -1;}
      
      //open new file
      if ((fd = file_open(id, vol_addr, 0, oflag, TFS_REGULAR_TYPE, 0))==-1)
	{free(path);return -1;}
      
      //store new file entry in <dino> directory
      entry.d_ino = _filedes[fd]->inode;
      if (strlen(fname)>TFS_NAME_MAX) fname[TFS_NAME_MAX]=0;
      strcpy(entry.d_name, fname);
      if ((errnum = directory_pushent(id, vol_addr, dino, &entry))!=EXIT_SUCCESS)
	{free(path);return -1;}
      
      //return file descriptor
      return fd;      
    }
    
    //case 2: file does not exist
    if (errnum == TFS_F_NOTFOUND) {free(path);return -1;}
    
    //case 3: file exists
    if (errnum == EXIT_SUCCESS){      
      //obtaining name and parent path
      if ((errnum = path_split(path, &fname))!=EXIT_SUCCESS)
	{free(path);return -1;}
      
      //get disk id
      if ((errnum = path_follow(NULL, &elem))!=EXIT_SUCCESS)
	{free(path);return -1;}
      strcat(elem, DEF_EXT);
      if ((errnum = start_disk(elem, &id))!=EXIT_SUCCESS)
	{free(path);return -1;}      
      
      //get volume address
      if ((errnum = path_follow(NULL, &elem))!=EXIT_SUCCESS)
	{free(path);return -1;}      
      if (atou(elem)<0)
	{free(path);return -1;}
      if ((errnum = p_index(id, atou(elem), &vol_addr))!=EXIT_SUCCESS)
	{free(path);return -1;}  
      
      //open file      
      if ((fd = file_open(id, vol_addr, inode, oflag, 0, 0))==-1)
	{free(path);return -1;}
      
      //truncate file if necessary
      if (((oflag&O_WRONLY)||(oflag&O_RDWR))&&(oflag&O_TRUNC)){
	if ((errnum = file_realloc(id, vol_addr, inode, 0))!=EXIT_SUCCESS)
	  {free(path);return -1;}
      }
      
      //return file descriptor
      return fd;      
    }
    return -1;
  } else {
    errnum = TFS_O_NOACCESS;
    return -1;
  }
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
  //test fildes
  if (!ISFILDES(fildes)) return TFS_BAD_FILDES;
  else {
    File* fd = _filedes+fildes;
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
  //test fildes
  if (!ISFILDES(fildes)) return TFS_BAD_FILDES;
  else {
  
  ssize_t s;
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

 
