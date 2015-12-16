#include "ll.h"
#include "error.h"
#include "block.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define SIZE_INT 4

#define B0_IDX_DSIZE 0    /**< index of disk size */
#define B0_IDX_PRTCOUNT 4    /**< index of number of partition */
#define B0_IDX_PRTABLE 8    /**< index of partition table */

typedef uint32_t ad_b;    /**< address for blocks in the disk */

/**
 * A structure containing informations about an opened disk                    
 */
typedef struct {
  char name[D_NAME_MAXLEN+1];      /**< name of the disk */
  int fd;          /**< file descriptor */
  uint32_t size;   /**< size of the disk */
  uint32_t npart;   /**< number of partitions */
  uint32_t part[D_PARTMAX];    /**< size of partitions, null at the creation. */
} disk_ent;

static disk_ent* _disk[DD_MAX];    /**< opened disks. A disk_id refers to an index in this array */


/**
 * Physical reading of a block in a disk.
 * Lowest level to read a block, 
 * are directly accessed.
 * This function is private
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
  
  if ( lseek(_disk[id]->fd, B_SIZE*num, SEEK_SET) == -1 ) {
    // error message
    return D_SEEK_ERR;
    }
  int r = read(_disk[id]->fd, b, B_SIZE);
  if ( r == -1 ) {
    // error message
    return D_READ_ERR;
  }
    
  return EXIT_SUCCESS;
}

/**
 * Physical writing of a block in a disk.
 * Lowest level to read a block, 
 * are directly accessed.
 * This function is private
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
    if ( write(_disk[id]->fd, b,  B_SIZE) == -1 ) {
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
  while((i<DD_MAX)&&(_disk[i]!=NULL))
    i++;
  if(i == DD_MAX) 
    return OD_FULL;
  
  int new_fd = open(name,O_RDWR);
  if(new_fd == -1){
    return D_OPEN_ERR;
  }

  *id = i;

  disk_ent* dent = (disk_ent*) malloc(sizeof(disk_ent));
  dent->name[D_NAME_MAXLEN]=0;
  strncpy(dent->name, name, D_NAME_MAXLEN);
  dent->fd=new_fd;

  _disk[i] = dent;

  block b_read = new_block();
  error err_read = read_physical_block(i,b_read,0);
  
  if(err_read != EXIT_SUCCESS){
    free( b_read );
    free(dent);
    _disk[i]=NULL;
    return err_read;
  }

  rintle(&dent->size,b_read,0*SIZE_INT);
  rintle(&dent->npart,b_read,1*SIZE_INT);
  
  int j = 0;
  for(j=0;j<dent->npart && j<D_PARTMAX;j++){
    rintle(dent->part+j, b_read, B0_IDX_PRTABLE+j*SIZE_INT);
  }
  free(b_read);
  
  return EXIT_SUCCESS;
}


/**
 * Read a block from disk.
 * Use a cache memory to read.
 * 
 * 
 */
error read_block(disk_id id,block b,uint32_t num){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    // error message
    return D_WRONGID;;
  }
  return read_physical_block(id, b, num);
}



/**
 * Write a block to a disk.
 * Use a cache memory to write.e
 * 
 * 
 * 
 */
error write_block(disk_id id,block b,uint32_t num){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    // error message
    return D_WRONGID;
  }

  return write_physical_block(id, b, num);
}

/**
 * Syncronize the disk.
 * Flush  the cache.
 * 
 * 
 * 
 */
error sync_disk(disk_id id){
  return EXIT_SUCCESS;
}


/**
 * Closes an opened disk.
 * Frees all associated memory
 * 
 * 
 * 
 */
error stop_disk(disk_id id){

  if ( id >= DD_MAX || _disk[id] == NULL ) {
    // error message
    return D_WRONGID;
  }

  error e = sync_disk(id);
  if ( e != EXIT_SUCCESS )   // mmh... should we realy stop the disk in that case ?
    return e;

  //free(_disk[id]->name);
  free(_disk[id]);
  return EXIT_SUCCESS;;
}


/**
 * Returns disk information
 * 
 * 
 */
error disk_stat(disk_id id, d_stat* stat){
  
  if ( id >= DD_MAX || _disk[id] == NULL ) {
    return D_WRONGID;
  } else {
    int n;
    strncpy(stat->name, _disk[id]->name, D_NAME_MAXLEN);
    stat->size = _disk[id]->size;
    stat->npart = _disk[id]->npart;
    for (n=0; n<stat->npart; n++){
      stat->part[n] = _disk[id]->part[n];
    }
    return EXIT_SUCCESS;
  }
}

