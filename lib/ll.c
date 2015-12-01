#include "ll.h"
#include "error.h"



/**                                                                                                                                            
 * A structure containing informations about an opened disk                                                                                    
 */
typedef struct disk_ent =
  {
    char *name;      /**< name of the disk */
    int fd;          /**< file descriptor */
    uint32_t size;   /**< size of the disk */

  } disk_ent;

static disk_ent __o_disk[DD_MAX];    /**< opened disks. A disk_id refers to an index in this array */


/**
 * 
 * 
 * 
 * 
 * 
 */
error read_physical_block(disk_id id,block b,uint32_t num){
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
