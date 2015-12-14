#ifndef LL_H
#define LL_H
#include <stdint.h>
#include "error.h"
#include "block.h"

#define DD_MAX 100      /***< maximum number of open disk */
#define D_PARTMAX 10    /***< maximum number of partitions in a disk */
#define D_NAME_MAXLEN 79     /***< disk name maximum length */


/**
 * A type to represent an index of the DISK_ID array
 */
typedef uint8_t disk_id;

/**
 * A structure to store basic disk information
 */
typedef struct {
  char name[D_NAME_MAXLEN];      /**< name of the disk */
  uint32_t size;   /**< size of the disk */
  uint32_t npart;   /**< number of partitions */
  uint32_t part[D_PARTMAX];    /**< parition sizes */
} d_stat;

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
 * @brief This function returns basic disk information
 * @param[in] id Disk id
 * @param[out] stat where information is stored
 * @return Returns an error if encountered
 * @see D_WRONGID
 */
error disk_stat(disk_id id, d_stat* stat);

#endif
