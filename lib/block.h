#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

#define B_SIZE 1024    /***< block size */

/**
 * A structure to represent blocks
 */
typedef struct{
  char data[B_SIZE]; /**< the block contents */
} block;

/**
 * @brief Reads an integer in little-endian from a block
 * 
 *
 * @param[out] value pointer where read integer is stored 
 * @param[in] b the block from which integer is read
 * @param[in] idx index of integer in block
 * @return void
 */
void rintle(uint32_t* value, block b, uint8_t idx);
/**
 * Writes an integer in little-endian to a block
 * at a specified index
 *
 * @param[in] value of integer to be written 
 * @param[out] b the block to which integer is written
 * @param[in] idx index of integer in block
 * @return void
 */
void wintle(uint32_t value, block b, uint8_t idx);

#endif
