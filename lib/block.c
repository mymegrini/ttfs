#include "block.h"

/**
 * Reads an integer in little-endian from a block
 * at a specified index idx and stores it in value
 * integer pointer 
 * 
 */
void rintle(uint32_t* value, block b, uint8_t idx){
  *value = b.data[idx*4]
    +256*(b.data[idx*4+1]
	  +256*(b.data[idx*4+2]
		+256*b.data[idx*4+3]
		)
	  );
}

/**
 * Writes an integer in little-endian to a block
 * at a specified index idx and stores it in value
 * integer pointer
 *
 */
void wintle(uint32_t value, block b, uint8_t idx){
  b.data[idx*4] = value % 256;
  value = value / 256;
  b.data[idx*4+1] = value % 256;
  value = value / 256;
  b.data[idx*4+2] = value % 256;
  value = value / 256;
  b.data[idx*4+3] = value % 256;
}
