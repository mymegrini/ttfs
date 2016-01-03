// tfsll.h
//
// last-edit-by: <nscott32> 
//
// Description: TFS low level library functions
//
//////////////////////////////////////////////////////////////////////

#ifndef TFSLL_H
#define TFSLL_H 1

#include <inttypes.h>



////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Directory entry
 *
 * A directory entry contains a file number and a name.
 */
struct dirent {
    uint32_t       d_ino;       /**< file number    */
    char           d_name[28];  /**< entry name     */
};



/**
 * @brief Directory stream
 *
 * A directory is an abstract vision of a directory.
 */
typedef struct {
  dirent           entry[16];     /**< current entries                */
  uint32_t         next_entries;  /**< block adress for next entries  */
  uint32_t         first_entries; /**< block adress for dirst entries */
  uint32_t         last_entries;  /**< block adress for last entries  */
} DIR;



////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Puch the block at b_addr to free blocks list
 * 
 * @param b_addr 
 * @return error EXIT_SUCCESS, TFS_ERRBLOCK if address is not valid
 */
error
freeblock_push ( const uint32_t b_addr );



/**
 * @brief Remove the block at b_addr of the free blocks list
 * 
 * @param b_addr 
 * @return error EXIT_SUCCESS, TFS_ERRADDR if address is not valid
 */
error
freeblock_rm ( const uint32_t b_addr );



/**
 * @brief Push the directory entry to the directory
 * 
 * @param dir 
 * @param ent
 * @return error EXIT_SUCCESS, TFS_FULL if the volume is full
 */
error
directory_pushent ( DIR directory, const struct dirent *restrict entry  );



/**
 * @brief Remove a directory entry
 *
 *  
 * @param inode 
 * @return error EXIT_SUCCESS
 */
error
directory_rment ( DIR directory, const struct dirent *restrict entry );



/**
 * @brief Add a data block to a file
 *
 * Add the block at b_addr to the file of inode file number 
 * @param inode 
 * @param b_addr 
 * @return error EXIT_SUCCESS, TFS_ERRBLOCK if adress is not valid
 *         TFS_ERRINODE if the inode is not valid
 */
error
file_pushblock ( uint32_t inode, uint32_t b_addr );



/**
 * @brief Remove a data block from a file
 *
 * Remove the data block at b_adrr to the file of inode file number
 * @param inode 
 * @param b_addr 
 * @return error EXIT_SUCCESS, TFS_ERRBLOCK if adress is not valid,
 *         TFS_ERRINODE if the inode is not valid
 */
error
file_rmblock( uint32_t inode, uint32_t b_addr );



/**
 * @brief Free all data blocks of a file
 *
 * Free all data block of the file of inode file number
 * @param inode 
 * @param b_addr 
 * @return error EXIT_SUCCESS, TFS_ERRINODE if the inode is not valid
 */
error
file_freeblocks ( uint32_t inode );



/**
 * @brief Follow a path
 *
 * Follow the path path.
 * After execution, entry contains the next entry
 * from the path, and path has been modified
 * containing the path after entry.
 * @param path 
 * @param entry 
 * @return error TFS_ERRPATH if path is not valid
 */
error
path_follow ( char * path, char * entry );



#endif // TFSLL_H
//////////////////////////////////////////////////////////////////////
// $Log:$
//
