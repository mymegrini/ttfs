#include "ll.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

/**                                                                                                                                            
 * A structure containing informations about an opened disk                                                                                    
 */
typedef struct {
  char *name;      /**< name of the disk */
  int fd;          /**< file descriptor */
  uint32_t size;   /**< size of the disk */
  uint32_t npart;   /**< number of partitions */
  uint32_t part[D_PARTMAX];    /**< index of partitions, null at the creation. */
} disk_ent;

static disk_ent *_disk[DD_MAX];    /**< opened disks. A disk_id refers to an index in this array */




/**
 * Physical reading of a block in a disk.
 * Lowest level to read a block, 
 * are directly accessed
 *
 * \param id the id of the disk 
 * \param b the block to store the reading
 * \param num the number of the block
 * \return error 
 * \see D_WRONGID
 * \see B_OUT_OF_DISK
 * \see D_SEEK_ERR
 * \see D_READ_ERR
 */
error read_physical_block(disk_id id,block b,uint32_t num){

  // In the case of reading the block 0,
  // check if the disk is a new empty file
  // if not, return D_NEWDISK
  if ( num == 0 ) {
    int r = read(_disk[id]->fd, &b, B_SIZE);
    if ( r == -1 ) {
      // error message
      return D_READ_ERR;
    } else if ( r == 0 ){
      // error message
      return D_NEWDISK;
    }
  }
  else {
    if ( lseek(_disk[id]->fd, B_SIZE*num, SEEK_SET) == -1 ) {
      // error message
      return D_SEEK_ERR;
    }
    int r = read(_disk[id]->fd, &b, B_SIZE);
    if ( r == -1 ) {
      // error message
      return D_READ_ERR;
    }
  }  
  return EXIT_SUCCESS;
}

/**
 * Physical writing of a block in a disk.
 * Lowest level to read a block, 
 * are directly accessed
 *
 * \param id the id of the disk 
 * \param b the block to store the reading
 * \param num the number of the block
 * \return error 
 * \see D_WRONGID
 * \see B_OUT_OF_DISK
 * \see D_SEEK_ERR
 * \see D_READ_ERR
 */
error write_physical_block(disk_id id,block b,uint32_t num){

  if ( num > _disk[id]->size ) {
    // error message
    return B_OUT_OF_DISK;
  } else {
    if ( lseek(_disk[id]->fd, B_SIZE*num, SEEK_SET) == -1 ) {
      // error message
      return D_SEEK_ERR;
    }
    if ( write(_disk[id]->fd, &b,  B_SIZE) == -1 ) {
      // error message
      return D_WRITE_ERR;
    }
  }
  
  return EXIT_SUCCESS;
}


/**
 * Starting a disk
 * Attribute a dynamic id to this disk
 * Readding the block zero for starting disk
 *
 * \param name the name of the disk 
 * \param id the dynamic id attribute to the disk at start
 * \return error 
 */
error start_disk(char *name,disk_id *id){
  int i = 0;
  while((i<DD_MAX)||(_disk[i]!=NULL))
    i++;
  if(i == DD_MAX) {
    // error message
    return OD_FULL;
  }
  int new_fd = open(name,O_CREAT|S_IRUSR|S_IWUSR);
  if(new_fd == -1){
    // error message
    return D_OPEN_ERR;
  }
  block b_read;
  error err_read = read_physical_block(i,b_read,0);
  if(err_read != EXIT_SUCCESS)
    return err_read;
  _disk[i]=(disk_ent *)(malloc(sizeof(disk_ent *)));
  _disk[i]->name=(char *)(malloc(sizeof(char)*strlen(name)));
  strcpy(_disk[i]->name, name);    // better to copy the string, if not name could change...
  _disk[i]->fd=new_fd;
  uint32_t val_size;
  _readint(b_read,0,&val_size); //fonction de Nicolas
  _disk[i]->size = val_size;
  uint32_t val_npart;
  _readint(b_read,4,&val_npart);
  _disk[i]->npart = val_npart;
  int j = 0;
  for(j=0;j<val_npart;j++){
    int pos = j*4+8;
    uint32_t val_tmp;
    _readint(b_read,pos,&val_tmp);
    _disk[i]->part[j] = val_tmp;
  }
  *id = i;

  return EXIT_SUCCESS;
}


/**
 * @brief Read a block from disk.
 *        Use a cache memory to read.
 * 
 * @param id 
 * @param b 
 * @param num 
 * @return error could be D_WRONGID or EXIT_SUCCESS
 * @see D_WRONGID
 */
error read_block(disk_id id,block b,uint32_t num){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    // error message
    return D_WRONGID;;
  }
  return read_physical_block(id, b, num);
}



/**
 * @brief Write a block to a disk.
 *        Use a cache memory to write.e
 * 
 * @param id 
 * @param b 
 * @param num 
 * @return error could be D_WRONGID or EXIT_SUCCESS
 * @see D_WRONGID
 */
error write_block(disk_id id,block b,uint32_t num){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    // error message
    return D_WRONGID;
  }

  return write_physical_block(id, b, num);
}

/**
 * @brief Syncronize the disk.
 *        Do all operations in the cache.
 * 
 * @param id 
 * @return error
 */
error sync_disk(disk_id id){
  return EXIT_SUCCESS;
}


/**
 * @brief Free an opened disk.
 * 
 * @param id 
 * @return error is D_WRONGID when ID is wrong, if not EXIT_SUCCESS
 * @see D_WRONGID
 */
error stop_disk(disk_id id){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    // error message
    return D_WRONGID;
  }

  error e = sync_disk(id);
  if ( e != EXIT_SUCCESS )   // mmh... should we realy stop the disk in that case ?
    return e;
  
  free(_disk[id]);
  return EXIT_SUCCESS;;
}


