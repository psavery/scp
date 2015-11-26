// Author -- Patrick S. Avery -- 2015

#ifndef FILE_SYSTEM_UTILS_H
#define FILE_SYSTEM_UTILS_H

#define DEFAULT_MODE_T 0755

#include <stdbool.h>

enum identify_file_type_e {
  FILE_IS_REG = 0,
  FILE_IS_DIR,
  UNKNOWN_FILE_TYPE,
  READ_ERROR,
  DOES_NOT_EXIST
};

int fileSystemUtils_getFileType(const char* path);

int fileSystemUtils_getFilePermissions(const char* path);

size_t fileSystemUtils_getFileSize(const char* path);

bool fileSystemUtils_mkdirIfNeeded(const char* path);

// Returns a dynamically allocated character array that contains a
// comma delimited list of the files in a directory
// The returned character array needs to be freed
char* fileSystemUtils_D_getDelimitedFileList(const char* dirName);

#endif
