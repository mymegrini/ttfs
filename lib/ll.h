#ifndef LL_H
#define LL_H
#include <stdint.h>
#include "error.h"

#define B_SIZE 1024    /***< block size */
#define DD_MAX 50      /***< maximum number of open disk */
#define D_PARTMAX 3    /***< maximum number of partitions in a disk */
#define D_NAME_MAXLEN 79     /***< disk name maximum length */

/**
 * A structure to represent blocks
 */
typedef struct{
  char data[B_SIZE]; /**< the block contents */
} block;

/**
 * A structure to represent an index of the DISK_ID array
 */
typedef int disk_id;

/**
 * @brief This function associates an id number to a disk
 * @param[in] name Name of the disk
 * @param[out] id Pointer to receive the id
 * @return Returns an error if encountered
 */
error start_disk(char *name,disk_id *id);
 
/**
 * @brief This function reads a data block from the disk
 * @param[in] id Disk id
 * @param[out] b Block to receive data
 * @param[in] num Physical block number
 * @return Returns an error if encountered
 */
error read_block(disk_id id,block b,uint32_t num);
 
/**
 * @brief This function writes a data block from the disk
 * @param[in] id Disk id
 * @param[in] b Block to receive data
 * @param[in] num Physical block number
 * @return Returns an error if encountered
 */
error write_block(disk_id id,block b,uint32_t num);

/**
 * @brief This function flushes te buffer
 * @param[in] id Disk id
 * @return Returns an error if encountered
 */
error sync_disk(disk_id id);

/**
 * @brief This function ends the session corresponding to id
 * @param[in] id Disk id
 * @return Returns an error if encountered
 */
error stop_disk(disk_id id);

/**
 * @brief This function creates and initializes a new disk
 * @param[in] name disk name
 * @param[in] size disk size
 * @param[out] id wheere disk id is stored
 * @return Returns an error if encountered
 */
error init_disk(char* name, int size);
#endif
