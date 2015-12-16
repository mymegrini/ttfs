#include "ll.h"
#include "error.h"
#include "block.h"
#include "block0.h"
#include "default.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>


#define OPTSTRING "s:n:"
#define OPT_SIZE 's'
#define OPT_NAME 'n'
#define MOD_PART 0


error partition( uint32_t *partsize, int nb_part, char *name );
void usage( char *argv0, FILE *stdout );

/**
 * @brief Partitionate the disk.
 *        Write the partition table of a disk.
 * 
 * @param argc 
 * @param argv -p size (one ore more)
 * @param[optionnal] name
 * @return int
 */
int
main ( int argc, char *argv[] )
{
  char name[D_NAME_MAXLEN];
  name[0] = 0;
  uint32_t *partsize = (uint32_t *) malloc( argc * sizeof(uint32_t) );//malloc( (argc-1)/2 * sizeof(uint32_t) );
  struct option options[] = {
    { "size", required_argument, 0, OPT_SIZE },
    { "name", required_argument, 0, OPT_NAME },
    { 0, 0, 0, 0 }
  };
  unsigned short mode = MOD_PART;
  int nb_part = 0;
  char c;

  
  while ((c = getopt_long( argc, argv, OPTSTRING, options, 0 )) != -1   ) {
    switch ( c ) {
    case OPT_SIZE:
      partsize[nb_part++] = atoll(optarg);
      break;
    case OPT_NAME:
      snprintf( name, D_NAME_MAXLEN, "%s", optarg );
      break;
    }
  }

  switch ( mode ) {
  case MOD_PART:    // partition creation mode _ default
    if ( nb_part == 0 ) {
      usage( argv[0], stderr );
      return -1;
    }      
    if ( name[0] == 0 )
      strncpy( name, DEF_FILENAME, D_NAME_MAXLEN );
    return partition( partsize, nb_part, name );
    break;
  }
  
  return EXIT_SUCCESS;
}



error partition( uint32_t *partsize, int nb_part, char *name ) {

  error e;
  disk_id d_id;
  block b;
  e = start_disk( name, &d_id );

  switch ( e ) {
    
  case EXIT_SUCCESS :
    b = new_block();
    read_block( d_id, b, 0 );
    wintle(nb_part, b, B0_ADD_NPART);
    for ( int i = 0; i < nb_part; i++ )
      wintle(partsize[i], b, B0_ADD_FSTPART+i*SIZEOF_INT);
    e = write_block( d_id, b, 0 );
    stop_disk( d_id );
    break;
  case D_OPEN_ERR:
    printerror("Error while opening the disk.\n"
	       "Please make sure %s exists", e);
    return e;
    break;
  default:
    printerror("A system error occured while accessing the disk.\n"
	       "Please try another time", e);
    break;    
  }
  return e;
}


/**
 * @brief Print a message about usage of the command
 * 
 * @param argv0 name of the command
 */
void usage( char *argv0, FILE *out ) {
  fprintf( out, "Usage:\t%s -s <size> [-%c <size>...] [-%c <name>]\n", argv0, OPT_SIZE, OPT_NAME );
}











