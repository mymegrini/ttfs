#include "error.h"
#include "stdio.h"

/**
 * Contains descriptions of all errors for
 * for the purpose of human readable error 
 * outputs
 */
static char* _errlist[] = {
  "SUCCESS",
  "Wrong disk id",
  "Access impossible: block number out of the disk",
  "Error while seeking to a block",
  "Error while reading a block",
  "Error while writing a block",
  "Array _disk full",
  "Error while opening a disk",
  "If a disk is created"
};

/**
 * Prints an errror message on error output
 * using the format <s: error message>
 *
 *\param s[in] sting to be added at beginning
 * of error output
 *\return void 
 */
void printerror(char* s, error err){
  fprintf(stderr, "%s: %s", s, _errlist[err]);
}
