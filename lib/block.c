#include "block.h"
#include <string.h> // for memset call


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
void rintle(uint32_t* value, block b, ad_byt idx){
    *value = b->data[idx]
    +256*(b->data[idx+1]
	  +256*(b->data[idx+2]
		+256*b->data[idx+3]
		)
	  );
}

/**
 * Writes an integer in little-endian to a block
 * at a specified index idx and stores it in value
 * integer pointer
 *
 */
void wintle(uint32_t value, block b, ad_byt idx){
  b->data[idx] = value % 256;
  value = value / 256;
  b->data[idx+1] = value % 256;
  value = value / 256;
  b->data[idx+2] = value % 256;
  value = value / 256;
  b->data[idx+3] = value % 256;
}
