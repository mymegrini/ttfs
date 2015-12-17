#include "error.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * Contains descriptions of all errors for
 * for the purpose of human readable error 
 * outputs
 */

static char* _errlist[] = {
  "SUCCESS",
  "Bad command format",
  "Wrong disk id",
  "Access impossible: block number out of the disk",
  "Error while seeking to a block",
  "Error while reading a block",
  "Error while writing a block",
  "Array _disk full",
  "Error while opening a disk",
  "New empty disk",
  "Wrong block index",
  "Wrong partition index",
  "Partition size does not match disk size"
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
  fprintf(stderr, "%s: %s\n", s, _errlist[err]);
}


/**
 * This function prints an error message and terminates the
 * program if <err> is not EXIT_SUCCESS
 */
void testerror(char* msg, error err){    
  if(err!=EXIT_SUCCESS){
    printerror(msg, err);
    exit(err);
  }
}
