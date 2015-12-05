#ifndef ERROR_H
#define ERROR_H 1

#define ERR_STRLEN 80

#define EXIT_SUCCESS 0

/**
 * A structure to represent an error id
 */
typedef int  error;


/**
 * Contain the last error
 */
extern int errnum;



extern void printerror(char *s);
	    
#define D_WRONGID 1    /**< Wrong disk_id  */
#define B_OUT_OF_DISK 2    /**< Access impossible: block number out of the disk */
#define D_SEEK_ERR 3    /**< Error while seeking to a block */
#define D_READ_ERR 4    /**< Error while reading a block */
#define D_WRITE_ERR 5    /**< Error while writing a block */
#define OD_FULL 6      /**< Array _disk full */ 
#define D_OPEN_ERR 7   /**< Error while opening a disk */
#define D_NEWDISK 8    /**< If a disk is created */

#endif
