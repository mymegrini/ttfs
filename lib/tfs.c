#include "ll.h"
#include "utils.h"
#include "block.h"
#include "tfs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>


#define TFS_ERRPATH 128

#define TFSPATH_STRSEP "/"
#define TFSPATH_CHARSEP '/'
#define TFSPATH_FPFX "FILE://"
#define TFSPATH_FPFXLEN 7
#define TFSPATH_ISFPFX(p) (strncmp(p, TFSPATH_FPFX, TFSPATH_FPFXLEN) == 0)
#define TFSPATH_HOSTTSTR "HOST"
#define TFSPATH_ISHOST(p) (strcmp(p, TFSPATH_FPFX, TFSPATH_FPFXLEN) == 0)



/**
 * pathnopfx modify return the unprefixed path
 * 
 * @param path 
 * @return error TFS_ERRPATH when path isn't prefixed
 */
error
pathnopfx ( char *path )  {
  if (TFSPATH_ISVALID(p)) {
    path += TFSPATH_FPFXLEN;
    return EXIT_SUCCESS;
  }
  else {
    return TFS_ERRPATH;
  }
}


int
tfs_mkdir(const char *cpath, mode_t mode) {
  // copy of the path to work on
  char * path = (char *) malloc(sizeof(cpath));
  memcpy(path, cpath, sizeof(cpath));
  error e;
  // path without prefix
  if ((e = pathnopfx(path)) != EXIT_SUCCESS) {
    return e;
  }
  char *device = strtok(NULL, TFSPATH_STRSEP);
  if (device == NULL) {
    return TFS_ERRPATH;
  }
  // We may let this to the command mkdir...
  if (TFSPATH_ISHOST(str)) {
    return mkdir(path + strlen(device), mode);
  }
  // Start device
  disk_id id;
  if ((e=start_disk(device, &id)) != EXIT_SUCCESS) {
    return e;
  }
  // recover partition id
  char *str_partid = strtok(NULL, TFSPATH_STRSEP);
  if (partid == NULL) {
    return TFS_ERRPATH;
  }
  long long int llpart_id = atou(partid);
  if (llpartid < 0) {
    return TFS_ERRPATH;
  }
  d_stat dstat;
  disk_stat(id, &dstat);
  if (dstat.npart < llpartid) {
    return TFS_ERRPATH;
  }
  // calculate partition address
  uint32_t part_add = 1;
  for (int i = 0; i < partid; i++)
    part_add += dstat.part[i];
  // recover partition block0
  block part_b0 = new_block();
  if ((e=read_block(id, block part_b0, part_add)) != EXIT_SUCCESS) {
    return e;
  }
  uint32_t b_size;
  rintle(&b_size, block part_b0, TFS_VOLUME_BLOCK_SIZE_INDEX);

  // navigate through arborescence
  char *entry = strtok(NULL, TFSPATH_STRSEP);
  if (entry == NULL) {
    return TFS_ERRPATH;
  }
  char *last_entry;
  do {
    last_token = entry;
    entry = strtok(NULL, TFSPATH_STRSEP);
    if (entry == NULL) {
      // create dir
    }
    else {
      // if token directory exists,
      // recover entries directories.
      // else raise UNREACHABLE_PATH
    }
  } 
  return 0;
}


