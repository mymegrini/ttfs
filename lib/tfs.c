#include "tfs.h"
#include "tfsll.h"
#include "ll.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////

#define DIRECTORY_SIZEMIN (2*TFS_DIRECTORY_ENTRY_SIZE)
#define ISFILDES(fildes) ((fildes)<TFS_FILE_MAX && _filedes[fildes])

#ifndef B_SIZE
   #define B_SIZE 1024
#endif
////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

struct _DIR {
  int             fd;
  uint32_t        b_offset;
  uint32_t        b_size;
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

static int dir_isempty(DIR *dir)
{
  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL)
    {
      if (strncmp(ent->d_name, ".", TFS_NAME_MAX) != 0 &&
	  strncmp(ent->d_name, "..", TFS_NAME_MAX) != 0)
	return 0;
    }
  return 1;  // empty directory
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
	 (block)&(_filedes[parent->fd]->inode), 0);
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
  DIR *dir = opendir(path);
  if (!dir_isempty(dir)) {
    free(parent_path);
    closedir(dir);
    return TFS_DIRNOTEMPTY;
  }
  
  // ino for file suppression
  uint32_t rmino    = _filedes[dir->fd]->inode;
  disk_id  d_id     = _filedes[dir->fd]->id;
  uint32_t vol_addr = _filedes[dir->fd]->vol_addr;
  closedir(dir);
  uint32_t parentino;
  // find parent inode before trying to delete file
  if ((e = find_inode(parent_path, &parentino)) != EXIT_SUCCESS) {
    free(parent_path);
    return e;
  }
  // remove file content
  if ((e = file_realloc(d_id, vol_addr, rmino, 0)) != EXIT_SUCCESS) {
    free(parent_path);
    return e;
  }
  // liberate inode
  if ((e = freefile_push(d_id, vol_addr, rmino)) != EXIT_SUCCESS) {
    free(parent_path);
    return e;
  }
  // remove parent entry
  e = directory_rment(d_id, vol_addr, parentino, last_el);
  free(parent_path);
  return e;
}

/**
 * 
 * 
 * 
 * 
 */
int tfs_rename(const char *path, const char *newname){
  
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
  int fd = tfs_open(filename, O_RDWR);
  if (fd == -1) {
    errnum = TFS_ERROPEN;
    return NULL;
  }
  DIR *dir = (DIR *) malloc(sizeof(DIR));
  dir->fd       = fd;
  dir->b_offset = 0;
  dir->b_size   = 0;
  for (int i = 0; i < TFS_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
    {
      char entry_buf[TFS_DIRECTORY_ENTRY_SIZE];
      if (tfs_read(fd, entry_buf, TFS_DIRECTORY_ENTRY_SIZE) == 0)
	return dir;
      else {
	// fill buffer
	rintle(&dir->buf[i].d_ino, (block)entry_buf, 0);
	strncpy(dir->buf[i].d_name, &entry_buf[INT_SIZE], TFS_NAME_MAX);
	dir->b_size++;
      }      
    }
  return dir;
}

/**
 * 
 * 
 * 
 */
struct dirent *readdir(DIR *dir){
  // end of the buffer
  if (dir->b_offset == dir->b_size) {
    char entry_buff[TFS_DIRECTORY_ENTRY_SIZE];
    // END OF THE DIRECTORY
    if (tfs_read(dir->fd, entry_buff, TFS_DIRECTORY_ENTRY_SIZE) == 0)
      return NULL;
    // NOT THE END, LOAD BUFFER
    dir->b_size   = 0;
    dir->b_offset = 0;
    do
      {
	rintle(&dir->buf[dir->b_size].d_ino, (block)entry_buff, 0);
	strncpy(dir->buf[dir->b_size].d_name, &entry_buff[INT_SIZE], TFS_NAME_MAX);
	++dir->b_size;
      }
    while (tfs_read(dir->fd, entry_buff, TFS_DIRECTORY_ENTRY_SIZE) != 0);
  }
  return &dir->buf[dir->b_offset++];
}

/**
 * 
 * 
 * 
 */
void rewinddir(DIR *dir){
  tfs_lseek(dir->fd, 0, SEEK_SET);
  dir->b_offset = 0;
  dir->b_size   = 0;
  for (int i = 0; i < TFS_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
    {
      char entry_buf[TFS_DIRECTORY_ENTRY_SIZE];
      if (tfs_read(dir->fd, entry_buf, TFS_DIRECTORY_ENTRY_SIZE) == 0)
	return ;
      else {
	// fill buffer
	rintle(&dir->buf[i].d_ino, (block)entry_buf, 0);
	strncpy(dir->buf[i].d_name, &entry_buf[INT_SIZE], TFS_NAME_MAX);
	dir->b_size++;
      }      
    }
  return ;
}

/**
 * 
 * 
 * 
 */
int closedir(DIR *dir){
  return tfs_close(dir->fd);
}
