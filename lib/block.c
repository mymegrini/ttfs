#include "block.h"
#include <stdlib.h>

/**
 * @brief  Return a new empty block, filled by 0. 
 * @return block
 */
block new_block( void ) { 
  return (block) calloc( B_SIZE, sizeof(byte) );
}


/**
 * Reads an integer in little-endian from a block
 * at a specified index idx and stores it in value
 * integer pointer 
 * 
 */
error rintle(uint32_t* value, block b, addr idx){
  if (idx<0 || idx>B_SIZE-4) return B_WRONGIDX;
  *value = ((uint32_t)b->data[idx])
    | (((uint32_t)b->data[idx+1])<<8)
    | (((uint32_t)b->data[idx+2])<<16)
    | (((uint32_t)b->data[idx+3])<<24);
  return EXIT_SUCCESS;
}

/**
 * Writes an integer in little-endian to a block
 * at a specified index idx and stores it in value
 * integer pointer
 *
 */
error wintle(uint32_t value, block b, addr idx){
  if (idx<0 || idx>B_SIZE-4) return B_WRONGIDX;
  b->data[idx] = (byte) value;
  b->data[idx+1] = (byte) (value>>8);
  b->data[idx+2] = (byte) (value>>16);
  b->data[idx+3] = (byte) (value>>24);
  return EXIT_SUCCESS;
}
