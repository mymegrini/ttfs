#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>
#include <inttypes.h>

#define B_SIZE 1024    /***< block size */


typedef uint8_t byte;    /**< a byte in the disk */
typedef uint16_t ad_byt;    /**< byte address in a block */
typedef uint16_t addiff_byt;    /**< difference of add_byte */
#define MAX_BYTADDR 1023


/**
 * A structure to represent blocks
 */
typedef struct{
  byte data[B_SIZE]; /**< the block contents */
} block;


/**
 * @brief Return a pointer to a new empty.
          An empty block is full of bytes 0. 
 * 
 * @return block* a new block
 */
block * new_block( void );


/**
 * @brief Reads an integer in little-endian from a block
 * 
 *
 * @param[out] value pointer where read integer is stored 
 * @param[in] b the block from which integer is read
 * @param[in] idx index of integer in block
 * @return void
 */
void rintle(uint32_t* value, block b, ad_byt idx);
/**
 * Writes an integer in little-endian to a block
 * at a specified index
 *
 * @param[in] value of integer to be written 
 * @param[out] b the block to which integer is written
 * @param[in] idx index of integer in block
 * @return void
 */
void wintle(uint32_t value, block b, ad_byt idx);

#endif
