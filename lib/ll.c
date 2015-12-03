#include "ll.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>

char _error[ERR_STRLEN+1];

/**                                                                                                                                            
 * A structure containing informations about an opened disk                                                                                    
 */
typedef struct {
  char *name;      /**< name of the disk */
  int fd;          /**< file descriptor */
  uint32_t size;   /**< size of the disk */
  uint8_t npart;   /**< number of partitions */
  uint32_t part[D_PARTMAX];    /**< index of partitions, null at the creation. */
} disk_ent;

static disk_ent *_disk[DD_MAX];    /**< opened disks. A disk_id refers to an index in this array */

/**
 * Physical reading of a block.
 * Lowest level to read a block, 
 * are directly accessed
 *
 * \param id the id of the disk 
 * \param b the block to store the reading
 * \param num the number of the block
 * \return error 
 * \see D_UNAVAILABLE
 * \see B_OUT_OF_DISK
 * \see D_SEEK_ERR
 * \see D_READ_ERR
 */
error read_physical_block(disk_id id,block b,uint32_t num){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    snprintf(_error, ERR_STRLEN, "disk_id %d unavailable\n", id);
    // error message
    return D_UNAVAILABLE;
  } else if ( num < _disk[id]->size ) {
    // error message
    return B_OUT_OF_DISK;
  } else {
    if ( lseek(_disk[id]->fd, B_SIZE*num, SEEK_SET) == -1 ) {
      // error message
      return D_SEEK_ERR;
    }
    int r = read(_disk[id]->fd, &b, B_SIZE);
    if ( r == -1 ) {
      // error message
      return D_READ_ERR;
    } else if ( r == 0 ){
      // error message
      return D_NEWDISK;
    }
  }
  
  return EXIT_SUCCESS;
}

/**
 * 
 * 
 * 
 * 
 * 
 */
error write_physical_block(disk_id id,block b,uint32_t num){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    // error message
    return D_UNAVAILABLE;
  } else if ( num > _disk[id]->size ) {
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
 * 
 * 
 * 
 * 
 */
error start_disk(char *name,disk_id *id){
  return EXIT_SUCCESS;
}
 
/**
 * 
 * 
 * 
 * 
 * 
 */
error read_block(disk_id id,block b,uint32_t num){
  return EXIT_SUCCESS;
}
 
/**
 * 
 * 
 * 
 * 
 * 
 */
error write_block(disk_id id,block b,uint32_t num){
  return EXIT_SUCCESS;
}

/**
 * 
 * 
 * 
 */
error sync_disk(disk_id id){
  return EXIT_SUCCESS;
}

/**
 * 
 * 
 * 
 */
error stop_disk(disk_id id){
  return EXIT_SUCCESS;
}
