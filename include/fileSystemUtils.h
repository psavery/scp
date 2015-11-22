// Author -- Patrick S. Avery -- 2015

#ifndef FILE_SYSTEM_UTILS_H
#define FILE_SYSTEM_UTILS_H

#include <stdbool.h>

enum identify_file_type_e {
  FILE_IS_REG = 0,
  FILE_IS_DIR,
  UNKNOWN_FILE_TYPE,
  READ_ERROR,
  DOES_NOT_EXIST
};

int fileSystemUtils_getFileType(const char* path);

bool fileSystemUtils_mkdirIfNeeded(const char* path);

#endif
