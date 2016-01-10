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
#include "error.h"
#include "ll.h"
#include <semaphore.h>
//#include "block.h"

////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////
#define INT_SIZE 4                                /***< Integer size */
#define INTX(i) (i*INT_SIZE)                      /***< Integer size multiples */

#define TFS_MAGIC_NUMBER 0x31534654               /***< TFS version identifier "TFS1" */
#define TFS_MAGIC_NUMBER_INDEX INTX(0)            /***< TFS version identifier index in volume superblock */
#define TFS_VOLUME_BLOCK_SIZE D_BLOCK_SIZE        /***< TFS block size*/
#define TFS_VOLUME_BLOCK_SIZE_INDEX INTX(1)       /***< TFS block size index in volume superblock */
#define TFS_VOLUME_BLOCK_COUNT_INDEX INTX(2)      /***< TFS bllock count index in volume superblock */
#define TFS_VOLUME_FREE_BLOCK_COUNT_INDEX INTX(3) /***< TFS free block count index in volume superblock */
#define TFS_VOLUME_FIRST_FREE_BLOCK_INDEX INTX(4) /***< TFS first free block index in volume superblock */
#define TFS_VOLUME_MAX_FILE_COUNT_INDEX INTX(5)   /***< TFS max file count index in volume superblock */
#define TFS_VOLUME_FREE_FILE_COUNT_INDEX INTX(6)  /***< TFS free file count index in volume superblock */
#define TFS_VOLUME_FIRST_FREE_FILE_INDEX INTX(7)  /***< TFS first free file index in volume superblock */

#define INT_PER_BLOCK (TFS_VOLUME_BLOCK_SIZE/INT_SIZE) /***< Number of integers per block*/
#define TFS_FILE_TABLE_INDEX 1                    /***< TFS index of the file table */
#define TFS_DIRECT_BLOCKS_NUMBER 10               /***< TFS direct data blocks number in file table entry */
#define TFS_INDIRECT1_CAPACITY INT_PER_BLOCK      /***< TFS file's maximum number of indirect1 blocks */
#define TFS_INDIRECT2_CAPACITY (INT_PER_BLOCK*INT_PER_BLOCK) /***< TFS file's maximum number of indirect2 blocks */
#define TFS_FILE_TABLE_ENTRY_SIZE INTX(6+TFS_DIRECT_BLOCKS_NUMBER) /***< TFS file table entry size */
#define TFS_FILE_SIZE_INDEX INTX(0)               /***< TFS file size index in file table entry */
#define TFS_FILE_TYPE_INDEX INTX(1)               /***< TFS file type index in file table entry */
#define TFS_REGULAR_TYPE 0                        /***< TFS regular file table entry type */
#define TFS_DIRECTORY_TYPE 1                      /***< TFS directory file table entry type */
#define TFS_PSEUDO_TYPE 2                         /***< TFS pseudo file table entry type */
#define TFS_FILE_SUBTYPE_INDEX INTX(2)            /***< TFS file pseudo-type index in file table entry */
#define TFS_DATE_SUBTYPE 0                        /***< TFS date file table entry subtype */
#define TFS_DISK_SUBTYPE 1                        /***< TFS disk file table entry subtype */
#define TFS_DIRECT_INDEX(i) INTX(3+(i))           /***< TFS direct data block <i> file table entry index */
#define TFS_INDIRECT1_INDEX INTX(3+TFS_DIRECT_BLOCKS_NUMBER) /***< TFS indirect1 block index in file table entry */
#define TFS_INDIRECT2_INDEX INTX(4+TFS_DIRECT_BLOCKS_NUMBER) /***< TFS indirect2 block index in file table entry */
#define TFS_NEXT_FREE_FILE_ENTRY_INDEX INTX(5+TFS_DIRECT_BLOCKS_NUMBER) /***< TFS next free file entry index */

#define TFS_VOLUME_NEXT_FREE_BLOCK_INDEX (TFS_VOLUME_BLOCK_SIZE-INT_SIZE) /***< TFS volume next free block index */

#define TFS_DIRECTORY_ENTRIES_PER_BLOCK			\
  (TFS_VOLUME_BLOCK_SIZE/TFS_DIRECTORY_ENTRY_SIZE)/*** Number of directory entries per block */

#define TFS_NAME_MAX 28                           /** TFS directory entry name maximum length */
#define TFS_DIRECTORY_ENTRY_SIZE (INTX(1)+TFS_NAME_MAX) /** TFS directory entry size */
#define TFS_DIRECTORY_ENTRY_INDEX(i) ((i)*TFS_DIRECTORY_ENTRY_SIZE) /** TFS directory entry file index */

#define TFS_FILE_MAX_SIZE (TFS_VOLUME_BLOCK_SIZE			\
			   *(TFS_DIRECT_BLOCKS_NUMBER			\
			     +(TFS_VOLUME_BLOCK_SIZE/INT_SIZE)		\
			     *(1 + (TFS_VOLUME_BLOCK_SIZE/INT_SIZE))	\
			     )						\
			   )

#define TFS_FILE_MAX 400                          /** TFS directory maximum number of open files */

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
  char           d_name[TFS_NAME_MAX];  /**< entry name     */
};

/**
 * @brief File index tree
 *
 * A structure which represents a file's data block tree
 */
typedef struct _index* _index;

typedef struct file* file;

////////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////////

file _filedes[TFS_FILE_MAX];

////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Push the block <b_addr> to the free blocks list
 * 
 * @param b_addr block index on the volume
 * @param id disk id
 * @param vol partition number
 * @return error EXIT_SUCCESS, TFS_ERRBLOCK if address is not valid
 */
error
freeblock_push (disk_id id, uint32_t vol_addr, uint32_t b_addr);


/**
 * @brief Remove the block at b_addr of the free blocks list
 * 
 * @param id disk id
 * @param vol partition number
 * @param b_addr block volume address
 * @return error EXIT_SUCCESS, TFS_ERRADDR if address is not valid
 */
error
freeblock_pop (disk_id id, uint32_t vol_addr, uint32_t* b_addr);


/**
 * @brief Push the file entry <inode> to the free file entry list
 * 
 * @param inode file inode
 * @param id disk id
 * @param vol partition number
 * @return error
 */
error
freefile_push (disk_id id, uint32_t vol_addr, uint32_t inode);


/**
 * @brief Remove the file entry <inode> from the free file entry list
 * 
 * @param id disk id
 * @param vol partition number
 * @param inode file inode number
 * @return error 
 */
error
freefile_pop (disk_id id, uint32_t vol_addr, uint32_t* inode);


/**
 * @brief Add a data block to a file
 *
 * Add the block at b_addr to the file of inode file number 
 *
 * @param inode file inode
 * @param b_addr block index on the volume
 * @return error EXIT_SUCCESS, TFS_ERRBLOCK if adress is not valid
 *         TFS_ERRINODE if the inode is not valid
 */
error
fileblock_add (disk_id id, uint32_t vol_addr, uint32_t inode, _index index);


/**
 * @brief Remove a data block from a file
 *
 * Remove the data block <b_adrr> from the file <inode>
 *
 * @param inode file inode
 * @param b_addr block index in the file
 * @return error EXIT_SUCCESS, TFS_ERRBLOCK if adress is not valid,
 *         TFS_ERRINODE if the inode is not valid
 */
error
fileblock_rm (disk_id id, uint32_t vol, uint32_t inode, _index index);


/**
 * @brief Change size of file <inode> to size <size>
 *
 * @param id disk id
 * @param vol partition number
 * @param inode file inode number
 * @param size new file size
 */
error
file_realloc (disk_id id, uint32_t vol_addr, uint32_t inode, uint32_t size);


/**
 * @brief Push the directory entry to the directory
 * 
 * @param dir 
 * @param ent
 * @return error EXIT_SUCCESS, TFS_FULL if the volume is full
 */
error
directory_pushent (const disk_id id, const uint32_t vol_addr,
		   const uint32_t inode, const struct dirent *entry );


/**
 * @brief Remove a directory entry
 *
 *  
 * @param inode 
 * @return error EXIT_SUCCESS
 */
error
directory_rment (disk_id id, uint32_t vol, const struct dirent *restrict entry);


/**
 * @brief Free all data blocks of a file
 *
 * Free all data blocks of the <inode> file
 *
 * @param inode file inode
 * @param vol partition index
 * @param id disk id number 
 * @return error EXIT_SUCCESS, TFS_ERRINODE if the inode is not valid
 */
error
file_freeblocks (disk_id id, uint32_t vol, uint32_t inode);

/**
 * @brief Finds a file block's volume address
 *
 * @param inode file inode
 * @param vol partition index
 * @param id disk id number 
 * @param[in] b_file_addr block's file number
 * @param[out] b_addr block's volume address
 */
error
find_addr(disk_id id, uint32_t vol, uint32_t inode,
	  uint32_t b_file_addr, uint32_t* b_addr);


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
