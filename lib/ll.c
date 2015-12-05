#include "ll.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types>
#include <unistd.h>

/**                                                                                                                                            
 * A structure containing informations about an opened disk                                                                                    
 */
typedef struct disk_ent =
  {
    char *name;      /**< name of the disk */
    int fd;          /**< file descriptor */
    uint32_t size;   /**< size of the disk */
    uint8_t npart;   /**< number of partitions */
    uint32_t part[D_PARTMAX]    /**< index of partitions, null at the creation. */
  } disk_ent;

extern disk_ent __o_disk[DD_MAX];    /**< opened disks. A disk_id refers to an index in this array */

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

  if ( id >= DD_MAX || __o_disk[id] == NULL ) {
    // error message
    return D_UNAVAILABLE
  } else if ( num < b.size ) {
    // error message
    return B_OUT_OF_DISK;
  } else {
    if ( lseek(__o_disk[id].fd, B_SIZE*num, SEEK_SET) == -1 ) {
      // error message
      return D_SEEK_ERR;
    }
    if ( read(__o_disk[id], b, B_SIZE) == -1 ) {
      // error message
      return D_READ_ERR;
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

  if ( id >= DD_MAX || __o_disk[id] == NULL ) {
    // error message
    return D_UNAVAILABLE
  } else if ( num < b.size ) {
    // error message
    return B_OUT_OF_DISK;
  } else {
    if ( lseek(__o_disk[id].fd, B_SIZE*num, SEEK_SET) == -1 ) {
      // error message
      return D_SEEK_ERR;
    }
    if ( write(__o_disk[id], b, B_SIZE) == -1 ) {
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
  if(i==DD_MAX) {
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
  _disk[i]->name=name;
  _disk[i]->fd=new_fd;

  *id = i;

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


