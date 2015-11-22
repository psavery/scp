// Author -- Patrick S. Avery -- 2015

#ifndef SSH_UTILS_H
#define SSH_UTILS_H

#include <stdbool.h>

#define USER_SIZE 32
#define HOST_SIZE 32
#define FILE_PATH_SIZE 128
#define PASS_SIZE 32

typedef struct {
  char user[USER_SIZE];
  char host[HOST_SIZE];
  char filePath[FILE_PATH_SIZE];
  char pass[PASS_SIZE];
  int port;
  bool isLocal;
} sshInfo;

typedef sshInfo* psshInfo;

// Simply checks to see if the path contains ":"
// If it does, it assumes the file is not local
bool sshUtils_fileIsLocal(const char* path);

// function reads the input and sets the struct to be the values given
bool sshUtils_setSCPInfo(char* input, psshInfo info);

#endif
