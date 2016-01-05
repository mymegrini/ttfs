#ifndef TFS_H
#define TFS_H
#include <sys/types.h>
#include "tfsll.h"
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
