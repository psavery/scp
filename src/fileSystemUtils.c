// Author -- Patrick S. Avery -- 2015

#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fileSystemUtils.h>

int fileSystemUtils_getFileType(const char* path)
{
  struct stat path_stat;

  // This will return zero if the read is successful
  if (stat(path, &path_stat) != 0) {
    if (errno == ENOENT) return DOES_NOT_EXIST;
    else return READ_ERROR;
  }

  // Is the file regular?
  else if (S_ISREG(path_stat.st_mode)) return FILE_IS_REG;

  // Is the file a directory?
  else if (S_ISDIR(path_stat.st_mode)) return FILE_IS_DIR;

  return UNKNOWN_FILE_TYPE;
}

int fileSystemUtils_getFilePermissions(const char* path)
{
  struct stat st;
  stat(path, &st);

  int statchmod = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
  return statchmod;
}

// Returns the size of a file as type size_t
size_t fileSystemUtils_getFileSize(const char* path)
{
  struct stat st;
  stat(path, &st);
  return st.st_size;
}

static bool _mkdir(const char* dir, mode_t mode) {

  char tmp[PATH_MAX];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", dir);

  len = strlen(tmp);

  if (tmp[len - 1] == '/')
    tmp[len - 1] = 0;

  // Will call 'mkdir' for every directory to the path
  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = 0;
      // This may return false sometimes. But only the
      // final mkdir at the end really matters...
      mkdir(tmp, mode);
      *p = '/';
    }
  }

  // Create the final directory...
  if (mkdir(tmp, mode) != 0) return false;
  return true;
}

bool fileSystemUtils_mkdirIfNeeded(const char* path)
{
  struct stat st = {0};

  if (fileSystemUtils_getFileType(path) == DOES_NOT_EXIST)
    // the 0755 mode allows read-write-execute for user and
    // only read and execute for everyone else
    if (!_mkdir(path, DEFAULT_MODE_T)) return false;

  return true;
}

bool fileSystemUtils_getCWD(char* string)
{
  getcwd(string, PATH_MAX);
  if (string == NULL) {
    fprintf(stderr, "Error: getcwd() failed in %s\n", __FUNCTION__);
    fprintf(stderr, "errnor is %i\n", errno);
    return false;
  }
  return true;
}

bool fileSystemUtils_chdir(const char* path)
{
  int ret = chdir(path);
  if (ret != 0) {
    fprintf(stderr, "Error: cannot change directory to %s in %s\n", path,
            __FUNCTION__);
    fprintf(stderr, "errno is %i\n", errno);
    return false;
  }
  return true;
}

// The _D_ stands for "dynamic". It is there to emphasize
// that the array that is returned is dynamically allocated and needs to be
// freed
// This returns a character array of every file/directory in a directory
// It is delimited by commas. It also returns '.' and '..'
char* fileSystemUtils_D_getDelimitedFileList(const char* dirName)
{
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir(dirName)) != NULL) {
    // First loop will be to get the size to malloc
    size_t size = 0;
    while ((ent = readdir(dir)) != NULL) {
      // +1 is for the comma or the \0 at the end
      size += strlen(ent->d_name) + 1;
    }
    closedir(dir);
    // Second loop is to copy over the list
    if ((dir = opendir(dirName)) != NULL) {
      char* fileList = malloc(sizeof(char) * size);

      // Set the first value to be the NULL terminator
      *fileList = '\0';

      // Now catenate each file from the list
      while ((ent = readdir(dir)) != NULL) {
        strcat(fileList, ent->d_name);
        // Add a comma unless we reached the end
        if (strlen(fileList) + 1 < size) strcat(fileList, ",");
      }
      closedir(dir);
      return fileList;
    }
  }

  // If we made it this far, an error occurred
  fprintf(stderr, "Error opening %s for reading\n", dirName);
  return NULL;
}
