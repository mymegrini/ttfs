#include <stdint.h>

#ifndef ERROR_H
#define ERROR_H

#define ERR_STRLEN 80

#define EXIT_SUCCESS 0

/**
 * A structure to represent an error id
 */
typedef uint8_t error;

/**
 * Prints an errror message on error output 
 */
void printerror(char *s, error err);

/**
 * @brief This function forces an exit if an error is encountered
 * @param[in] err error value
 * @param[in] msg error message header
 * @return void
 * 
 * @see printerror
 */
void testerror(char* msg, error err);

/**< Abbreviation list:
 * C  : Command
 * D  : Disk
 * OD : Open Disk Array
 * B  : Block
 * P  : Partition
 * V  : Volume
 * F  : File
 * FT : File Table
 * FBL: Free Block List
 * FEL: Free Entry List 
 * TFS: Toy File System
 >*/

#define EXIT_SUCCESS 0      /**< Successful execution >*/
#define C_FORMAT 1          /**< Bad command format >*/
#define D_WRONGID 2         /**< Wrong disk_id  >*/
#define B_OUT_OF_DISK 3     /**< Access impossible: block number out of the disk >*/
#define D_SEEK_ERR 4        /**< Error while seeking to a block >*/
#define D_READ_ERR 5        /**< Error while reading a block >*/
#define D_WRITE_ERR 6       /**< Error while writing a block >*/
#define OD_FULL 7           /**< Array _disk full >*/ 
#define D_OPEN_ERR 8        /**< Error while opening a disk >*/
#define D_NEWDISK 9         /**< If a disk is created >*/
#define B_WRONGIDX 10       /**< Wrong block index >*/
#define P_WRONGIDX 11       /**< Wrong partition index >*/
#define P_CORRUPTED 12      /**< Partition size does not match disk size >*/
#define B_WRONGNAME 13      /**< Wrong block name >*/
#define P_WRONGVALUE 14     /**< Wrong value of the partition (negative) >*/
#define V_FULL 15           /**< No free volume blocks remaining >*/
#define V_FBL_CORRUPTED 16  /**< Volume free block list corrupted >*/
#define F_FULL 17           /**< File reached maximum supported size >*/
#define FT_FULL 18          /**< File table full >*/
#define FT_FEL_CORRUPTED 19 /**< File table free entry list corrupted >*/
#define F_SIZE_CORRUPTED 20 /**< File size value corrupted >*/
#define F_EMPTY 21          /**< Empty file >*/
#define I_CORRUPTED 22      /**< Corrupted index structure >*/
#define B_OUTOFBOUNDS 23    /**< Block file address exceeds maximum file size >*/
#define D_STOP_FAIL 24      /**< Attempt to sync disk failed >*/
#define D_PERM_FAIL 25      /**< Setting permissions for disk failed >*/
#define S_WRONGTYPE 26      /**< Semaphore type unrecognized */
#define TFS_MAX_FILE 27     /**< Maximum number of open files reached */
#define TFS_ERR_BIGFILE 28
#define TFS_SYSDIR 29
#define TFS_ERR_OPERATION 30
#define DIR_BLOCKFULL 31
#define TFS_ERRPATH 32
#define TFS_ENTRY_NOTFOUND 33
#define TFS_ERRLOCK 34
#define TFS_FILENOTFOUN 35
#define TFS_ERRPATH_HOST 36
#define TFS_ERRPATH_PARTID 37
#define TFS_FILENOTFOUND 38
#define TFS_ERRPATH_NODISK 39
#define TFS_EXISTINGENTRY 40
#define TFS_BAD_FILDES 41  /**< Bad file descriptor */
#define TFS_LOCK_FAIL 42    /**< Failed to acquire lock on file */
#define TFS_UNLOCK_FAIL 43  /**< Failed to release lock on file */
#define ERR_TFS_READ 44

/**
 * @brief This variable can store an error
 *
 */

extern error errnum;

#endif
