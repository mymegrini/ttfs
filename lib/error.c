#include "error.h"
#include <stdio.h>

/**
 * Initializing errnum value at 0
 */
error errnum = 0;

/**
 * Contains descriptions of all errors for
 * for the purpose of human readable error 
 * outputs
 */
char* _errlist[] = {
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
void printerror(char* s){
  fprintf(stderr, "%s: %s\n", s, _errlist[errnum]);
}
