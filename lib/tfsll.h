// tfsll.h
//
// last-edit-by: <nscott32> 
//
// Description: TFS low level library functions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef TFSLL_H
#define TFSLL_H 1

////////////////////////////////////////////////////////////////////////////////
// HEADERS
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>



////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////
#define INT_SIZE 4                                  /***< Integer size */
#define INTX(i) (i*INT_SIZE)                        /***< Integer size multiples */

#define TFS_MAGIC_NUMBER 0x31534654                 /***< TFS version identifier "TFS1" */
#define TFS_MAGIC_NUMBER_INDEX INTX(0)              /***< TFS version identifier index in volume superblock */
#define TFS_VOLUME_BLOCK_SIZE 1024                  /***< TFS block size*/
#define TFS_VOLUME_BLOCK_SIZE_INDEX INTX(1)         /***< TFS block size index in volume superblock */
#define TFS_VOLUME_BLOCK_COUNT_INDEX INTX(2)        /***< TFS bllock count index in volume superblock */
#define TFS_VOLUME_FREE_BLOCK_COUNT_INDEX INTX(3)   /***< TFS free block count index in volume superblock */
#define TFS_VOLUME_FIRST_FREE_BLOCK_INDEX INTX(4)   /***< TFS first free block index in volume superblock */
#define TFS_VOLUME_MAX_FILE_COUNT_INDEX INTX(5)     /***< TFS max file count index in volume superblock */
#define TFS_VOLUME_FREE_FILE_COUNT_INDEX INTX(6)    /***< TFS free file count index in volume superblock */
#define TFS_VOLUME_FIRST_FREE_FILE_INDEX INTX(7)    /***< TFS first free file index in volume superblock */

#define TFS_FILE_TABLE_INDEX 1                      /***< TFS index of the file table */
#define TFS_FILE_TABLE_ENTRY_SIZE INTX(16)          /***< TFS file table entry size */
#define TFS_FILE_SIZE_INDEX INTX(0)                 /***< TFS file size index in file table entry */
#define TFS_FILE_TYPE_INDEX INTX(1)                 /***< TFS file type index in file table entry */
#define TFS_REGULAR_TYPE 0                          /***< TFS regular file table entry type */
#define TFS_DIRECTORY_TYPE 1                        /***< TFS directory file table entry type */
#define TFS_PSEUDO_TYPE 2                           /***< TFS pseudo file table entry type */
#define TFS_FILE_SUBTYPE_INDEX INTX(2)              /***< TFS file pseudo-type index in file table entry */
#define TFS_DATE_SUBTYPE 0                          /***< TFS date file table entry subtype */
#define TFS_DISK_SUBTYPE 1                          /***< TFS disk file table entry subtype */
#define TFS_DIRECT_INDEX(i) INTX(3+i)               /***< TFS direct data block <i> file table entry index */
#define TFS_DIRECT_BLOCKS_NUMBER 10                 /***< TFS direct data blocks number in file table entry */
#define TFS_INDIRECT1_INDEX INTX(13)                /***< TFS indirect1 block index in file table entry */
#define TFS_INDIRECT2_INDEX INTX(14)                /***< TFS indirect2 block index in file table entry */
#define TFS_NEXT_FREE_FILE_ENTRY_INDEX INTX(15)     /***< TFS next free file entry index in file table entry */

#define TFS_VOLUME_NEXT_FREE_BLOCK_INDEX INTX(255)  /***< TFS volume next free block index */

#define TFS_DIRECTORY_ENTRY_SIZE INTX(4)            /***< TFS directory entry size */
#define TFS_DIRECTORY_ENTRY_INDEX(i) INTX(4*i)      /***< TFS directory entry file index */
#define TFS_DIRECTORY_ENTRY_MAX_NAME_LENGTH INTX(7) /***< TFS directory entry name maximum length */


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
  char           d_name[TFS_DIRECTORY_ENTRY_MAX_NAME_LENGTH];  /**< entry name     */
};



/**
 * @brief Directory stream
 *
 * A directory stream is an abstract vision of a directory.
 */
typedef struct {
  dirent           entry[16];     /**< current entries                */
  uint8_t          offset;        /**< position of the current entry  */
  uint32_t         next_block;    /**< block adress for next entries  */
  uint32_t         first_block; /**< block adress for dirst entries */
  uint32_t         last_block;  /**< block adress for last entries  */
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
freeblock_push (const disk_id id, const uint32_t vol_addr, const uint32_t b_addr);



/**
 * @brief Remove the block at b_addr of the free blocks list
 * 
 * @param b_addr 
 * @return error EXIT_SUCCESS, TFS_ERRADDR if address is not valid
 */
error
freeblock_rm (disk_id id, uint32_t vol, const uint32_t b_addr);



/**
 * @brief Push the directory entry to the directory
 * 
 * @param dir 
 * @param ent
 * @return error EXIT_SUCCESS, TFS_FULL if the volume is full
 */
error
directory_pushent (disk_id id, uint32_t vol, uint32_t inode, struct dirent *restrict entry );



/**
 * @brief Remove a directory entry
 *
 *  
 * @param inode 
 * @return error EXIT_SUCCESS
 */
error
directory_rment (disk_id id, uint32_t vol, uint32_t inode, const struct dirent *restrict entry);



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
file_pushblock (disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr);



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
file_rmblock(disk_id id, uint32_t vol, uint32_t inode, uint32_t b_addr);



/**
 * @brief Free all data blocks of a file
 *
 * Free all data block of the file of inode file number
 * @param inode 
 * @param b_addr 
 * @return error EXIT_SUCCESS, TFS_ERRINODE if the inode is not valid
 */
error
file_freeblocks (disk_id id, uint32_t vol, uint32_t inode);



#define TFS_PATHLEAF 1
#define TFS_ERRPATH_NOPFX 104
#define TFS_ERRPATH_NOWORKINGPATH 200
/**
 * @brief Follow a path
 *
 * Follow the path path by filling entry with the next entry.
 * entry should be the adress of a char *.
 *
 * Usage:
 *
 * First, call path_follow with the full path, i.e. prefixed
 * by FILE://, and the adress of an unitialized char*.
 * For the first call, the pointer *entry is not modified
 * so you can pass NULL.
 * If the path is not prefixed, TFS_ERRPATH_NOPFX is returned.
 * Example:
 *    char path[] = "FILE://disk/vol/entry1/entry2"
 *    if (path_follow(path, NULL) == TFS_ERRPATH) ...;
 * 
 * To follow the the previous path, you should now call
 * follow_path with a null path and the adress of an unitialized 
 * char*.
 * If the last call has reached a leaf, then TFS_PATHLEAF is returned
 * and entry stay unchanged.
 * If you try calling path_follow(NULL,NULL) you will get a
 * segmentation fault. 
 * Example:
 *     char *entry;
 *     while (path_follow(NULL, &entry) != TFS_PATHLEAF)
 *        {
 *          printf("Entry : %s\n", entry);
 *        }
 *     printf("Last entry %s was a leaf.\n");
 *  
 * If the working path has not been initialized by a first call
 * and you follow a path (with something likefollow_path(NULL, &entry))$
 * then error TFS_ERRPATH_NOWORKINGPATH is returned.
 *
 * About memory allocation:
 * An internal static char* is used to work on, it is automatically
 * released after reaching a leaf or after
 * 
 * @param[in] path 
 * @param[out] entry 
 * @return error EXIT_SUCCESS, TFS_PATHLEAF, TFS_ERRPATH_NOPFX, 
 *               TFS_ERRPATH_NOWORKINGPATH
 *               
 */
error
path_follow (const char * path, char ** entry);



#endif // TFSLL_H
////////////////////////////////////////////////////////////////////////////////
// $Log:$
//
