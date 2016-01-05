#ifndef TFS_H
#define TFS_H

#define INT_SIZE 4                                /***< Integer size */

#define  0x31534654               /***< TFS version identifier "TFS1" */
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
#define TFS_DIRECT_INDEX(i) (12+i*INT_SIZE)       /***< TFS direct data block <i> file table entry index */
#define TFS_DIRECT_BLOCKS_NUMBER 10               /***< TFS direct data blocks number in file table entry */
#define TFS_INDIRECT1_INDEX 52                    /***< TFS indirect1 block index in file table entry */
#define TFS_INDIRECT2_INDEX 56                    /***< TFS indirect2 block index in file table entry */
#define TFS_NEXT_FREE_FILE_ENTRY_INDEX 60         /***< TFS next free file entry index in file table entry */

#define TFS_VOLUME_NEXT_FREE_BLOCK_INDEX 1020     /***< TFS volume next free block index */

#define TFS_DIRECTORY_ENTRY_SIZE 32               /***< TFS directory entry size */
#define TFS_DIRECTORY_ENTRY_INDEX(i) (32*i)       /***< TFS directory entry file index */
#define TFS_DIRECTORY_ENTRY_MAX_NAME_LENGTH 28    /***< TFS directory entry name maximum length */

#include <sys/types.h>
#include <dirent.h>

/**
 * @brief This function attempts to create a directory named <path>
 * @param[in] path specifies name and location of directory
 * @param[in] mode specifies the permissions to use
 * @return Returns 0 on success or -1 if an error occured
 */
int tfs_mkdir(const char *path, mode_t mode);

/**
 * @brief This function deletes a directory, which must be empty
 * @param[in] path specifies name and location of directory
 * @return Returns 0 on success or -1 if an error occured
 */
int tfs_rmdir(const char *path);

/**
 * @brief This function renames a file, moving it between directories if required.
 * @param[in] old specifies old pathname
 * @param[in] new specifies new pathname
 * @return Returns 0 on success or -1 if an error occured
 */
int tfs_rename(const char *old, const char *new);

/**
 * @brief This function returns a file descriptor for use in subsequent system calls
 * @param[in] name of file to be opened
 * @param[in] oflag specifies access modes
 * @param[in] mode specifies permissions to use for O_CREAT
 * @return Returns 0 on success or -1 if an error occured
 */
int tfs_open(const char *name,int oflag, ...);

/**
 * @brief This function attempts to read up to <nbytes> bytes from file descriptor
 * <fildes> into the buffer starting at <buf>
 * @param[in] fildes file desctiptor
 * @param[in] buf buffer
 * @param[in] nbytes maximum number of bytes to be read
 * @return Returns, on success, number of bytes read (0 indicates EOF) and -1 on error 
 */
ssize_t tfs_read(int fildes,void *buf,size_t nbytes);

/**
 * @brief This function writes up to <nbytes> bytes from the buffer pointed to by <buf>
 * to the file reffered to by the file descriptor <fildes>
 * @param[in] fildes file desctiptor
 * @param[out] buf buffer
 * @param[in] nbytes maximum number of bytes to be written
 * @return Returns, on success, number of bytes written and -1 on error 
 */
ssize_t tfs_write(int fildes,void *buf,size_t nbytes);

/**
 * @brief This function closes a file descriptor
 * @param[in] fildes file descriptor
 * @return Returns 0 on success or -1 if an error occured
 */
int tfs_close(int fildes);

/**
 * @brief This function repositions the offset of the open file associated with the file
 * descriptor <fildes> to the argument <offset> according to the directive <whence>
 * @param[in] fildes file descriptor
 * @param[in] offset number of bytes
 * @param[in] whence directive
 * @return Returns the resulting offset location measured from the beginning of the file,
 * on success, and -1, on error
 */
off_t tfs_lseek(int fildes,off_t offset,int whence);

/**
 * @brief This function opens a directory stream corresponding to the directory <filename>
 * @param[in] filename directory name
 * @return Returns a pointer to the directory stream, on success, NULL, on error
 */
DIR *opendir(const char *filename);

/**
 * @brief This function returns the next directory entry in the directory stream <dirp>
 * @param[in] dirp directory stream
 * @return Returns a pointer to a <dirent> structure, on success, NULL, on error
 */
struct dirent *readdir(DIR *dirp);

/**
 * @brief This function resets the position of the directory stream <dirp> to its beginning
 * @param[in] dirp directory stream
 * @return Returns no value
 */
void rewinddir(DIR *dirp);

/**
 * @brief This function closes the directory stream associated with <dirp>
 * @param[in] dirp directory stream
 * @return Returns 0 on success or -1 if an error occured
 */
int closedir(DIR *dirp);

#endif
