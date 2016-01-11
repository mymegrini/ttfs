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

#define EXIT_SUCCESS 0        /**< Successful execution >*/
#define C_FORMAT 1            /**< Bad command format >*/
#define D_WRONGID 2           /**< Wrong disk_id  >*/
#define B_OUT_OF_DISK 3       /**< Access impossible: block number out of the disk >*/
#define D_SEEK_ERR 4          /**< Error while seeking to a block >*/
#define D_READ_ERR 5          /**< Error while reading a block >*/
#define D_WRITE_ERR 6         /**< Error while writing a block >*/
#define OD_FULL 7             /**< Array _disk full >*/ 
#define D_OPEN_ERR 8          /**< Error while opening a disk >*/
#define D_NEWDISK 9           /**< If a disk is created >*/
#define B_WRONGIDX 10         /**< Wrong block index >*/
#define P_WRONGIDX 11         /**< Wrong partition index >*/
#define P_CORRUPTED 12        /**< Partition size does not match disk size >*/
#define B_WRONGNAME 13        /**< Wrong block name >*/
#define P_WRONGVALUE 14       /**< Wrong value of the partition (negative) >*/
#define V_FULL 15             /**< No free volume blocks remaining >*/
#define V_FBL_CORRUPTED 16    /**< Volume free block list corrupted >*/
#define F_FULL 17             /**< File reached maximum supported size >*/
#define FT_FULL 18            /**< File table full >*/
#define FT_FEL_CORRUPTED 19   /**< File table free entry list corrupted >*/
#define F_SIZE_CORRUPTED 20   /**< File size value corrupted >*/
#define F_EMPTY 21            /**< Empty file >*/
#define I_CORRUPTED 22        /**< Corrupted index structure >*/
#define B_OUTOFBOUNDS 23      /**< Block file address exceeds maximum file size >*/
#define D_STOP_FAIL 24        /**< Attempt to sync disk failed >*/
#define D_PERM_FAIL 25        /**< Setting permissions for disk failed >*/
#define S_WRONGTYPE 26        /**< Semaphore type unrecognized */
#define TFS_MAX_FILE 27       /**< Maximum number of open files reached */
#define TFS_ERR_BIGFILE 28    /**< Big file error */
#define TFS_SYSDIR 29         /**< System directory */
#define TFS_ERR_OPERATION 30  /**< Operation error */
#define DIR_BLOCKFULL 31      /**< Directory block full */
#define TFS_ERRPATH 32        /**< Path error */
#define TFS_ENTRY_NOTFOUND 33 /**< Entry not found */
#define TFS_ERRLOCK 34        /**< Lock error */
#define TFS_F_NOTFOUND 35     /**< File not found */
#define TFS_ERRPATH_HOST 36   /**< Path error : HOST */
#define TFS_ERRPATH_PARTID 37 /**< Volume id missing from path */
#define TFS_FILENOTFOUND 38   /**< File not found */
#define TFS_ERRPATH_NODISK 39 /**< Disk missing from path */
#define TFS_EXISTINGENTRY 40  /**< Entry already exists */
#define TFS_BAD_FILDES 41     /**< Bad file descriptor */
#define TFS_LOCK_FAIL 42      /**< Failed to acquire lock on file */
#define TFS_UNLOCK_FAIL 43    /**< Failed to release lock on file */
#define ERR_TFS_READ 44       /**< Error while reading a file */
#define TFS_ERROPEN 45        /**< Open error */
#define TFS_NOENTRY 46        /**< entry doesn't exist in the directory */
#define TFS_DIRNOTEMPTY 47    /**< directory not empty */
#define TFS_O_NOACCESS 48     /**< Access mode bits not found */
#define TFS_ERRWRITE 49       /**< Error while writing a file */
#define D_LOCK  50            /**< Can't lock disk when starting */
#define D_UNLOCK 51           /**< Can't unlock disk at closing */
#define D_GETWD_FAIL 52       /**< Failed to read current working directoryn */
#define TFS_NOTSUPPORTED 53   /**< Operation not supported yet */
#define TFS_SEG_FAULT 54      /**< Segmentation fault >*/
#define TFS_WRONGSUBTYPE 55   /**< Unrecognized subtype */
#define TFS_WRONGTYPE 56      /**< Unrecognized file type */
#define TFS_INVALIDSEEK 57    /**< Invalid seek mode */
#define TFS_ERRPATH_NOPATH 58

/**
 * @brief This variable can store an error
 *
 */

extern error errnum;

#endif
