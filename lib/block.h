#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>
#include <inttypes.h>
#include "error.h"

#define D_BLOCK_SIZE 1024    /***< block size */
#define INT_SIZE 4

#define B0_ADD_DSIZE 0    /**< Address of disk size */
#define B0_ADD_NPART (INT_SIZE*1)    /**< Address of partition number */
#define B0_ADD_FSTPART (B0_ADD_NPART+INT_SIZE)    /**< Address of first partition size */

typedef uint8_t byte;    /**< a byte in the disk */
typedef int16_t addr;    /**< byte address in a block */
//typedef uint16_t addiff_byt;    /**< difference of add_byte */
//#define MAX_BYTADDR 1023


/**
 * A structure to represent blocks
 */
struct block {
  byte data[D_BLOCK_SIZE]; /**< the block contents */
};

typedef struct block* block;   /**< a block is a pointer to a struct block */


/**
 * @brief Return a pointer to a new empty.
          An empty block is full of bytes 0. 
 * 
 * @return block a new block
 */
block new_block( void );

/**
 * @brief Reads an integer in little-endian from a block
 * 
 *
 * @param[out] value pointer where read integer is stored 
 * @param[in] b the block from which integer is read
 * @param[in] idx index of integer in block
 * @return error if index out of bound
 * @see B_WRONGIDX
 */
error rintle(uint32_t* value, block b, addr idx);

/**
 * Writes an integer in little-endian to a block
 * at a specified index
 *
 * @param[in] value of integer to be written 
 * @param[out] b the block to which integer is written
 * @param[in] idx index of integer in block
 * @return error if index out of bound
 * @see B_WRONGIDX
 */
error wintle(uint32_t value, block b, addr idx);
#endif
