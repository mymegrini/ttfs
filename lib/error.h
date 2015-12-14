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
extern void printerror(char *s, error err);

#define EXIT_SUCCESS 0    /**< Successful execution */
#define C_FORMAT 1    /**< Bad command format */
#define D_WRONGID 2    /**< Wrong disk_id  */
#define B_OUT_OF_DISK 3    /**< Access impossible: block number out of the disk */
#define D_SEEK_ERR 4    /**< Error while seeking to a block */
#define D_READ_ERR 5    /**< Error while reading a block */
#define D_WRITE_ERR 6    /**< Error while writing a block */
#define OD_FULL 7      /**< Array _disk full */ 
#define D_OPEN_ERR 8   /**< Error while opening a disk */
#define D_NEWDISK 9    /**< If a disk is created */
#define B_WRONGIDX 10 /**< Wrong block index */
#define D_WRONGNAME 11 /**< Wrong disk name */

#endif
