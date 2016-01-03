// tfsll.h
//
// last-edit-by: <nscott32> 
//
// Description: TFS low level library functions
//
//////////////////////////////////////////////////////////////////////

#ifndef TFSLL_H
#define TFSLL_H 1

//////////////////////////////////////////////////////////////////////
// HEADERS
//////////////////////////////////////////////////////////////////////
#include <stdint.h>



//////////////////////////////////////////////////////////////////////
// TYPES
//////////////////////////////////////////////////////////////////////

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



//////////////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////////////

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
path_follow ( const char * path, char ** entry );



#endif // TFSLL_H
//////////////////////////////////////////////////////////////////////
// $Log:$
//
