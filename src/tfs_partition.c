#include "ll.h"
#include "error.h"
#include "block.h"
#include "block0.h"
#include "default.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>


#define OPTSTRING "afhs:n:"
#define OPT_SIZE 's'
#define OPT_LONG_SIZE "size"
#define OPT_NAME 'n'
#define OPT_LONG_NAME "name"
#define OPT_FORCE 'f'
#define OPT_LONG_FORCE "force"
#define OPT_HELP 'h'
#define OPT_LONG_HELP "help"
#define OPT_APPEND 'a'
#define OPT_LONG_APPEND "append"

#define MOD_SAFE 1
#define MOD_APPEND 2
#define MOD_FORCE 4
#define MOD_HELP 8
#define MOD_DEFAULT MOD_SAFE

error partition( char *name, uint32_t *partsize, int nb_part, long long unsigned totalsize, short mode );
error part_append( disk_id d_id, uint32_t *partsize, int nb_part, long long unsigned totalsize );
error part_force( disk_id d_id, uint32_t *partsize, int nb_part, long long unsigned totalsize );
error part_safe( disk_id d_id, uint32_t *partsize, int nb_part, long long unsigned totalsize );

void usage( char *argv0, FILE *stdout );
void help( char *argv0 );

#include <ctype.h>
#include <string.h>
#include <inttypes.h>

long long int
atou ( char *s ) {
  int size = strlen(s);
  uint32_t val = 0;
  int count = 0;
  uint32_t below_max = (UINT32_MAX - 1) / 10;
  while ( isdigit(*s) ) {
    if (val > below_max)
      return UINT32TOOBIG;
    val = val*10 + ( *s++ - '0' );
    ++count;
  }
  return (count==size)?val:STRBADCHAR;
}

#define YES 1
#define NO 0

/**
 * @brief This function waits for an answer from the user
 * @param[in] prompt message for the user
 * @return Returns 0 for [n/N] and 1 for [y/Y]
 *
 * This function displays a prompt message <prompt> and
 * waits for the user's answer and returns it
 */

int answer(char* prompt){
  printf("%s", prompt);
  switch(getchar()){
  case 'Y':	
  case 'y':
    return YES;
  case 'n':
  case 'N':
    return NO;
  default:
    while(getchar() != '\n');
    return answer(prompt);
  }
}

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
  char name[D_NAME_MAXLEN+1];
  name[0] = 0;
  uint32_t *partsize = (uint32_t *) malloc( argc * sizeof(uint32_t) );//malloc( (argc-1)/2 * sizeof(uint32_t) );
  struct option options[] = {
    { OPT_LONG_HELP, no_argument, 0, OPT_HELP },
    { OPT_LONG_APPEND, no_argument, 0, OPT_APPEND },
    { OPT_LONG_FORCE, no_argument, 0, OPT_FORCE },
    { OPT_LONG_SIZE, required_argument, 0, OPT_SIZE },
    { OPT_LONG_NAME, required_argument, 0, OPT_NAME },
    { 0, 0, 0, 0 }
  };
  unsigned short mode = MOD_DEFAULT;
  int nb_part = 0;
  char c;
  long long int size;
  long long unsigned totalsize = 0;
    
  while ((c = getopt_long( argc, argv, OPTSTRING, options, 0 )) != -1   ) {
    switch ( c ) {
    case OPT_HELP:
      help ( argv[0] );
      return EXIT_SUCCESS;
      break;
      
    case OPT_FORCE:
      mode |= MOD_FORCE;
    case OPT_APPEND:
      mode |= MOD_APPEND;
      break;
      
    case OPT_SIZE:
      size = atou(optarg);
      if ( size == UINT32TOOBIG ) {    // doesn't fit in 4 bytes
	fprintf(stderr, "Size %s too big.\n", optarg);
	exit(UINT32TOOBIG); 
      }
      else if (size == STRBADCHAR) {    // bad character in the string, ex : 100hello
	fprintf(stderr, "Bad character in size %s, integer value was attempted.\n", optarg);
	exit(STRBADCHAR);
      }
      else if ( size > 0 ) {    // check for positive size
	partsize[nb_part++] = size;
	totalsize += size;
      }
      break;

    case OPT_NAME:
      snprintf( name, D_NAME_MAXLEN, "%s", optarg );
      break;
    }
  }


  if ( nb_part == 0 ) {
    usage( argv[0], stderr );
    return -1;
  }      
  if ( name[0] == 0 )
    strncpy( name, DEF_FILENAME, D_NAME_MAXLEN );

  return partition( name, partsize, nb_part, totalsize, mode ); 
}

error
part_force ( disk_id d_id, uint32_t *partsize, int nb_part, long long unsigned totalsize ) {
  error e;
  d_stat dstat;
  disk_stat (d_id, &dstat);
    
  if ( dstat.size - 1 < totalsize ) {
    fprintf (stderr,
	     "Not enough space available on the disk.\n"
	     "Demanded\t: %llu\n"
	     "Available\t: %u\n",
	     totalsize, dstat.size - 1);
    exit(-2);
  }
  block b;
  b = new_block();
  read_block (d_id, b, 0);
  wintle( nb_part, b, B0_ADD_NPART);
  for ( int i = 0; i < nb_part; i++ )
    wintle(partsize[i], b, B0_ADD_FSTPART+i*SIZEOF_INT);
  e = write_block( d_id, b, 0 );
  stop_disk( d_id );
  return e;
}

error
part_safe ( disk_id d_id, uint32_t *partsize, int nb_part, long long unsigned totalsize ) {
  error e;
  d_stat dstat;
  disk_stat (d_id, &dstat);
  uint32_t freespace = dstat.size - 1;
  
  if ( freespace < totalsize ) {
    fprintf (stderr,
	     "Not enough space available on the disk.\n"
	     "Demanded\t: %20llu block%s\n"
	     "Available\t: %20u block%s\n",
	     totalsize, (totalsize>1)?"s":"", freespace, (freespace>1)?"s":"");
    exit(-2);
  }
  if ( dstat.npart > 0 ) {
    printf("Existing partition table found on %s.\n", dstat.name);
    if ( answer("Overwrite partition table ? ") == NO )
      exit(EXIT_SUCCESS);
  }

  block b;      
  b = new_block();
  read_block (d_id, b, 0);
  wintle( nb_part, b, B0_ADD_NPART);
  for ( int i = 0; i < nb_part; i++ )
    wintle(partsize[i], b, B0_ADD_FSTPART+i*SIZEOF_INT);
  e = write_block( d_id, b, 0 );
  stop_disk( d_id );
  return e;
}

error
part_append(disk_id d_id, uint32_t *partsize, int nb_part, long long unsigned totalsize) {

  error e;
  d_stat dstat;
  disk_stat (d_id, &dstat);
  uint32_t usedspace = 1;

  for ( int i = 0; i < dstat.npart; i++ )
    usedspace += dstat.part[i];

  uint32_t freespace = dstat.size - usedspace;
  
  if ( freespace < totalsize ) {
    fprintf (stderr,
	     "Not enough free space available on the disk.\n"
	     "Demanded\t: %20llu block%s\n"
	     "Available\t: %20u block%s\n",
	     totalsize, (totalsize>1)?"s":"", freespace, (freespace>1)?"s":"");
    exit(-2);
  }

  block b;
  b = new_block();
  read_block (d_id, b, 0);
  
  wintle( dstat.npart + nb_part, b, B0_ADD_NPART);
  for ( int i = 0; i < nb_part; i++ )
    wintle( partsize[i], b, B0_ADD_FSTPART + ((dstat.npart+i)*SIZEOF_INT) );
  e = write_block( d_id, b, 0 );
  stop_disk( d_id );

  return e;
}

error
partition( char *name, uint32_t *partsize, int nb_part, long long unsigned totalsize, short mode ) {

  error e;
  disk_id d_id;

  e = start_disk( name, &d_id );
  switch ( e ) {

  case EXIT_SUCCESS :
    if ( mode & MOD_APPEND )
      return part_append (d_id, partsize, nb_part, totalsize);
    else if ( mode & MOD_FORCE )
      return part_force (d_id, partsize, nb_part, totalsize);
    else
      return part_safe (d_id, partsize, nb_part, totalsize);

  case D_OPEN_ERR:
    printerror("Error while opening the disk.\n"
	       "Please make sure the disk exists", e);
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
  fprintf( out, "Usage:\t%s -s <size> [-%c <SIZE>...] [-%c <DISK>]\n", argv0, OPT_SIZE, OPT_NAME );
}

#define HELP_OPTION(OPT_LONG, OPT_SHORT, DESCRIPTION) printf("  -%c, --%s\t %s\n", OPT_LONG, OPT_SHORT, DESCRIPTION)

void help( char *argv0 ) {
  usage ( argv0, stdout );
  puts( "Write the partition table of DISK (the disk.tfs in the current directory by default).\n\n");

  
  HELP_OPTION(OPT_FORCE, OPT_LONG_FORCE, "overwrite the partition table if it exists");
  HELP_OPTION(OPT_HELP, OPT_LONG_HELP, "print this message");
  HELP_OPTION(OPT_NAME, OPT_LONG_NAME, "choose the DISK");
  HELP_OPTION(OPT_SIZE, OPT_LONG_SIZE, "add a partition to the partition table. Size must be specified");
  
}
