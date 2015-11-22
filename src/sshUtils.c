// Author -- Patrick S. Avery -- 2015

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sshUtils.h>

bool sshUtils_fileIsLocal(const char* path)
{
  char* p = strchr(path, ':');

  if (!p) return true;
  else return false;
}

// info->host will be NULL if no host was found...
bool sshUtils_setSSHInfo(char* input, psshInfo info)
{
  // First, check to see if it is local
  if(sshUtils_fileIsLocal(input)) {
    snprintf(info->filePath, FILE_PATH_SIZE, "%s", input);
    info->isLocal = true;
    if (!info->filePath) {
      fprintf(stderr, "Error reading input in sshUtils_getSCPInfo()\n");
      return false;
    }
    else return true;
  }

  // If it made it here, it is not local
  info->isLocal = false;

  // p points to NULL if no user was given
  char* p = strchr(input, '@');

  // No user found, but we are still remote
  if (!p) {
    // user will be the user's name
    getlogin_r(info->user, USER_SIZE);
    snprintf(info->host, HOST_SIZE, "%s", strtok(input, ":"));
  }
  else {
    snprintf(info->user, USER_SIZE, "%s", strtok(input, "@"));
    snprintf(info->host, HOST_SIZE, "%s", strtok(NULL, ":"));
  }

  snprintf(info->filePath, FILE_PATH_SIZE, "%s", strtok(NULL, ":"));

  // Default ssh port is 22
  info->port = 22;

  if (!info->filePath) {
    fprintf(stderr, "Error reading input in sshUtils_getSCPInfo()\n");
    return false;
  }
  return true;
}
