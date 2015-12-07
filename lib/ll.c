#include "ll.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

/**
 * A structure containing informations about an opened disk                                                        
 */
typedef struct {
  char name[D_NAME_MAXLEN+1];      /**< name of the disk */
  int fd;          /**< file descriptor */
  uint32_t size;   /**< size of the disk */
  uint32_t npart;   /**< number of partitions */
  uint32_t part[D_PARTMAX];    /**< index of partitions, null at the creation. */
} disk_ent;

static disk_ent* _disk[DD_MAX];    /**< opened disks. A disk_id refers to an index in this array */




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
 * Reads an integer in little-endian from a block
 * at a specified index
 *
 * \param value pointer where read integer is stored 
 * \param b the block from which integer is read
 * \param idx index of integer in block
 * \return void
 */
void _readint(uint32_t* value, block b, uint8_t idx){
  *value = b.data[idx*4]
    +256*(b.data[idx*4+1]
	  +256*(b.data[idx*4+2]
		+256*b.data[idx*4+3]
		)
	  );
}

/**
 * Writes an integer in little-endian to a block
 * at a specified index
 *
 * \param value of integer to be written 
 * \param b the block to which integer is written
 * \param idx index of integer in block
 * \return void
 */
void _writeint(uint32_t value, block b, uint8_t idx){
  b.data[idx*4] = value % 256;
  value = value / 256;
  b.data[idx*4+1] = value % 256;
  value = value / 256;
  b.data[idx*4+2] = value % 256;
  value = value / 256;
  b.data[idx*4+3] = value % 256;
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
  if(i == DD_MAX) {
    errnum = OD_FULL;
    return OD_FULL;
  }
  int new_fd = open(name,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
  if(new_fd == -1){
  }
  disk_ent dent;
  block b_read;
  error err_read = read_physical_block(i,b_read,0);
  
  if(err_read != EXIT_SUCCESS)
    return err_read;
  
  strncpy(dent.name, name, D_NAME_MAXLEN+1);
  dent.fd=new_fd;
  _readint(&dent.size,b_read,0);
  _readint(&dent.npart,b_read,1);
  
  int j = 0;
  for(j=0;j<dent.npart && j<D_PARTMAX;j++){
    _readint(dent.part+j, b_read, 2+j);
  }
  
  _disk[i]=&dent;

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
  
  free(_disk[id]);
  return EXIT_SUCCESS;;
}

/**
 * This function creates and initializes a new disk.
 * It allocates enough memory for size blocks
 * and puts size in the block 0
 *
 *
 */
error init_disk(char* name, int size){
  disk_id id = 0;
  
  while((id<DD_MAX)&&(_disk[id]!=NULL))
    id++;
  if(id == DD_MAX) {
    errnum = OD_FULL;
    return OD_FULL;
  } else {
    int fd;
    
    if ((fd = open(name,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR)) == -1){
      errnum = D_OPEN_ERR;
      return D_OPEN_ERR;      
    } else {    
      disk_ent dent;
      block b;
      error r;

      strncpy(dent.name, name, D_NAME_MAXLEN+1);
      dent.fd = fd;
      dent.size =size;
      dent.npart =0;
      _disk[id] = &dent;
      
      if ((r = write_physical_block(id,b,size))!=EXIT_SUCCESS){
	printerror("init_disk");
	return r;
      } else {
	_writeint(0, b, 0);
	_writeint(size, b, 0);
	if ((r = write_physical_block(id,b,0))!=EXIT_SUCCESS){	  
	  printerror("init_disk");
	  return r;
	} else {   
	  return stop_disk(id);
	}
      } 
    }
  }
}
