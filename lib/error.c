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
  "Partition size does not match disk size",
  "Wrong block name",
  "Wrong value of the partition (negative)",
  "No free volume blocks remaining",
  "Volume free block list corrupted",
  "File reached maximum supported size",
  "File table full",
  "File table free entry list corrupted",
  "File size value corrupted",
  "Empty file",
  "Corrupted index structure",
  "Block file address out of bounds",
  "Attempt to stop disk failed",
  "Setting permissions for disk failed",
  "Semaphore type unrecognized",
  "Maximum number of open files reached",
  "TFS_ERR_BIGFILE",
  "TFS_SYSDIR",
  "TFS_ERR_OPERATION",
  "DIR_BLOCKFULL",
  "TFS_ERRPATH",
  "TFS_ENTRY_NOTFOUND",
  "TFS_ERRLOCK",
  "File not found",
  "TFS_ERRPATH_HOST",
  "TFS_ERRPATH_PARTID",
  "TFS_FILENOTFOUND",
  "TFS_ERRPATH_NODISK",
  "TFS_EXISTINGENTRY",
  "Bad file descriptor",
  "Failed to acquire lock on file",
  "Failed to release lock on file",
  "ERR_TFS_READ",
  "TFS_ERROPEN",
  "TFS_NOENTRY",
  "TFS_DIRNOTEMPTY",
  "Access mode bits not found",
  "Error while writing a file",
  "Can't lock disk when starting",
  "Can't unlock disk at closing",
  "Failed to read current working directory"
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

/**
 * This variable can be used to store an error in functions
 * which do not return <error>
 *
 */

error errnum = EXIT_SUCCESS;
