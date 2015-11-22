#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
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
    if (!_mkdir(path, 0755)) return false;

  return true;
}
