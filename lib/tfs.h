#ifndef TFS_H
#define TFS_H

#define INT_SIZE 4                                /***< Integer size */

#define TFS_MAGIC_NUMBER 0x31534654               /***< TFS version identifier "TFS1" */
#define TFS_MAGIC_NUMBER_INDEX 0                  /***< TFS version identifier index in volume superblock */
#define TFS_VOLUME_BLOCK_SIZE 1024                /***< TFS block size*/
#define TFS_VOLUME_BLOCK_SIZE_INDEX 4             /***< TFS block size index in volume superblock */
#define TFS_VOLUME_BLOCK_COUNT_INDEX 8            /***< TFS bllock count index in volume superblock */
#define TFS_VOLUME_FREE_BLOCK_COUNT_INDEX 12      /***< TFS free block count index in volume superblock */
#define TFS_VOLUME_FIRST_FREE_BLOCK_INDEX 16      /***< TFS first free block index in volume superblock */
#define TFS_VOLUME_MAX_FILE_COUNT_INDEX 20        /***< TFS max file count index in volume superblock */
#define TFS_VOLUME_FREE_FILE_COUNT_INDEX 24       /***< TFS free file count index in volume superblock */
#define TFS_VOLUME_FIRST_FREE_FILE_INDEX 28       /***< TFS first free file index in volume superblock */

#define TFS_FILE_TABLE_ENTRY_SIZE 64              /***< TFS file table entry size */
#define TFS_FILE_SIZE_INDEX 0                     /***< TFS file size index in file table entry */
#define TFS_FILE_TYPE_INDEX 4                     /***< TFS file type index in file table entry */
#define TFS_REGULAR_TYPE 0                        /***< TFS regular file table entry type */
#define TFS_DIRECTORY_TYPE 1                      /***< TFS directory file table entry type */
#define TFS_PSEUDO_TYPE 2                         /***< TFS pseudo file table entry type */
#define TFS_FILE_SUBTYPE_INDEX 8                  /***< TFS file pseudo-type index in file table entry */
#define TFS_DATE_SUBTYPE 0                        /***< TFS date file table entry subtype */
#define TFS_DISK_SUBTYPE 1                        /***< TFS disk file table entry subtype */
#define TFS_DIRECT_INDEX 12                       /***< TFS direct data blocks file table entry index */
#define TFS_DIRECT_BLOCKS_NUMBER 10               /***< TFS direct data blocks number in file table entry */
#define TFS_INDIRECT1_INDEX 52                    /***< TFS indirect1 block index in file table entry */
#define TFS_INDIRECT2_INDEX 56                    /***< TFS indirect2 block index in file table entry */
#define TFS_NEXT_FREE_FILE_ENTRY_INDEX 60         /***< TFS next free file entry index in file table entry */

#define TFS_VOLUME_NEXT_FREE_BLOCK_INDEX 1020     /***< TFS volume next free block index */

#endif
