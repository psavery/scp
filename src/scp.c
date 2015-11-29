/**********************************************************************
  scp.c - Source code for the scp functions

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include <linux/limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <scp.h>
#include <loadBar.h>
#include <fileSystemUtils.h>

#define LIBSSH_BUFFER_SIZE 16384

// Define this macro to produce more debug output
//#define SCP_DEBUG

int scp_copyFromServer(ssh_session session, char* from,
                       char* destination, bool isRecursive)
{
  // First, just make the initial preparations for scp...
  ssh_scp scp;
  int rc;

  // SSH_SCP_RECURSIVE allows us to detect if it's a directory
  scp = ssh_scp_new(session, SSH_SCP_READ | SSH_SCP_RECURSIVE, from);

  if (scp == NULL) {
    fprintf(stderr, "Error allocating scp session: %s\n",
            ssh_get_error(session));
    return SSH_ERROR;
  }
  rc = ssh_scp_init(scp);
  if (rc != SSH_OK) {
    fprintf(stderr, "Error initializing scp session: %s\n",
            ssh_get_error(session));
    ssh_scp_free(scp);
    return rc;
  }

  rc = ssh_scp_pull_request(scp);

  // Set up the scp info
  scpInfo scpinfo;
  scpinfo.session = session;
  scpinfo.scp = scp;
  snprintf(scpinfo.from, PATH_MAX, "%s", from);
  scpinfo.isRecursive = isRecursive;

  // A single file was requested...
  if (rc == SSH_SCP_REQUEST_NEWFILE) scpinfo.isRecursive = false;

  // Create the pointer to be passed around
  pscpInfo scp_info = &scpinfo;

  if (!_scp_handlePullRequest(scp_info, destination, rc)) {
    fprintf(stderr, "Error in %s: _scp_hanldePullRequest() failed!\n",
            __FUNCTION__);
    return SSH_ERROR;
  }

  ssh_scp_close(scp);
  ssh_scp_free(scp);
  return SSH_OK;
}

// rc is the return from the pull request
bool _scp_handlePullRequest(pscpInfo scp_info, const char* destination,
                            int pullRequestRet)
{
  bool success = false;

  switch (pullRequestRet) {
    // Requested a directory!
    case SSH_SCP_REQUEST_NEWDIR:
#ifdef SCP_DEBUG
      printf("in _scp_handlePullRequest(), rc is SSH_SCP_REQUEST_NEWDIR\n");
#endif
      // If recursive mode is turned off, just return false...
      if (!scp_info->isRecursive) {
        fprintf(stderr, "%s is a directory!\n", scp_info->from);
        return false;
      }

      if (ssh_scp_accept_request(scp_info->scp) != SSH_OK)
        fprintf(stderr, "Error accepting scp request: %s\n",
                ssh_get_error(scp_info->session));
      else {
        char newDest[PATH_MAX];
        snprintf(newDest, PATH_MAX, "%s/%s",
                 destination, ssh_scp_request_get_filename(scp_info->scp));

        // Make the local directory if needed
        if (!fileSystemUtils_mkdirIfNeeded(newDest)) {
          fprintf(stderr, "fileSystemUtils_mkdirIfNeeded() failed.\n");
          break;
        }

        success = _scp_copyDirFromServer(scp_info, newDest);
      }
      break;
    // Requested a file!
    case SSH_SCP_REQUEST_NEWFILE:
#ifdef SCP_DEBUG
      printf("in _scp_handlePullRequest(), rc is SSH_SCP_REQUEST_NEWFILE\n");
#endif
      if (ssh_scp_accept_request(scp_info->scp) != SSH_OK) {
        fprintf(stderr, "Error accepting scp request: ");
        fprintf(stderr, "%s\n", ssh_get_error(scp_info->session));
      }
      else {
        char newDest[PATH_MAX];
        int type = fileSystemUtils_getFileType(destination);

        // If the destination is a dir, set the newDest to be dir/fileName
        if (type == FILE_IS_DIR)
          snprintf(newDest, PATH_MAX, "%s/%s",
                   destination, ssh_scp_request_get_filename(scp_info->scp));

        // If the destination is anything else, just set newDest to be the same
        else if (type == FILE_IS_REG || type == DOES_NOT_EXIST)
          snprintf(newDest, PATH_MAX, "%s", destination);
        else {
          fprintf(stderr, "Error determining type of file for: %s\n",
                  destination);
          return false;
        }

        success = _scp_copyFileFromServer(scp_info, newDest);
      }
      break;
    // A warning was returned
    case SSH_SCP_REQUEST_WARNING:
      fprintf(stderr, "%s", "ssh_scp_pull_request() returned with a warning: ");
      fprintf(stderr, "%s\n", ssh_get_error(scp_info->session));
      break;
    // An error was returned
    case SSH_ERROR:
      fprintf(stderr, "%s", "ssh_scp_pull_request() returned with an error: ");
      fprintf(stderr, "%s\n", ssh_get_error(scp_info->session));
      break;
    // An unexpected value was returned
    default:
      fprintf(stderr, "%s%s\n",
              "ssh_scp_pull_request() returned an unexpected value.\n",
              "Please contact the development team to resolve this.\n");
      break;
  }

  return success;
}

bool _scp_copyFileFromServer(pscpInfo scp_info, const char* destination)
{
#ifdef SCP_DEBUG
  printf("in _scp_copyFileFromServer(): \n");
  printf("scp_info->from is %s\n", scp_info->from);
  printf("destination is %s\n", destination);
#endif
  int rc = SSH_ERROR;
  size_t fileSize = ssh_scp_request_get_size(scp_info->scp);
  size_t bytesRead = 0;
  char buffer[LIBSSH_BUFFER_SIZE];

  FILE *fp = fopen(destination, "w+");

  if (fp == NULL) {
    fprintf(stderr, "Error opening %s for writing\n", destination);
    return -1;
  }

  // If the fileSize is zero, just return true. No copying needed
  if (fileSize == 0) {
    // This is needed to refresh the state of the scp
    rc = ssh_scp_read(scp_info->scp, buffer, sizeof(buffer));
    fclose(fp);
    return true;
  }

  do {
    rc = ssh_scp_read(scp_info->scp, buffer, sizeof(buffer));
    if (rc == SSH_ERROR) {
      fprintf(stderr, "Error reading file: %s\n",
              ssh_get_error(scp_info->session));
      return -1;
    }

    // rc is equal to the number of bytes read if it is not an error...
    bytesRead += rc;

    // We want the loadBar() to print every time it is called, so we set
    // the resolution to be the fileSize
    loadBar_loadBar(bytesRead, fileSize, fileSize, 20, destination);

    // rc is set to the number of bytes transferred
    // With the way this is set up, we will print exactly rc bytes
    // from the buffer to the file pointer
    fprintf(fp, "%.*s", rc, buffer);
  }
  while (bytesRead < fileSize);

  fclose(fp);

  return true;
}

// This is recursive if more directories exist
// the SSH_SCP_REQUEST_NEWDIR return from ssh_scp_pull_request()
// should have already been received and accepted before calling this function
bool _scp_copyDirFromServer(pscpInfo scp_info, const char* destination)
{
#ifdef SCP_DEBUG
  printf("in _scp_copyDirFromServer(): \n");
  printf("scp_info->from is %s\n", scp_info->from);
  printf("destination is %s\n", destination);
#endif
  int rc = ssh_scp_pull_request(scp_info->scp);

  // Keep looping through the contents of the directory until we reach
  // the end of the directory
  while (rc != SSH_SCP_REQUEST_ENDDIR) {
    if (!_scp_handlePullRequest(scp_info, destination, rc)) return false;
    rc = ssh_scp_pull_request(scp_info->scp);
  }

  return true;
}

int scp_copyToServer(ssh_session session, char* from,
                     char* to, bool isRecursive)
{
#ifdef SCP_DEBUG
  printf("scp_copyToServer() called with from = '%s' and to = '%s'\n",
         from, to);
#endif
  // If to ends in '/', this causes confusion for the server, so just replace it
  if (from[strlen(from) - 1] == '/') from[strlen(from) - 1] = '\0';

  // If we're not in recursive mode and it's a directory, return with an error
  if (!isRecursive && fileSystemUtils_getFileType(from) == FILE_IS_DIR) {
    fprintf(stderr, "%s is a directory!\n", from);
    return SSH_ERROR;
  }
  // If we are in recursive mode and it's a file, just turn it off
  else if (isRecursive && fileSystemUtils_getFileType(from) == FILE_IS_REG)
    isRecursive = false;

  // Make the initial preparations for scp...
  ssh_scp scp;
  int rc;

  // SSH_SCP_RECURSIVE allows us to detect if it's a directory
  scp = ssh_scp_new(session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, to);

  if (scp == NULL) {
    fprintf(stderr, "Error allocating scp session: %s\n",
            ssh_get_error(session));
    return SSH_ERROR;
  }

  rc = ssh_scp_init(scp);
  if (rc != SSH_OK) {
    fprintf(stderr, "Error initializing scp session: %s\n",
            ssh_get_error(session));
    ssh_scp_free(scp);
    return rc;
  }

  scpInfo scpinfo;
  scpinfo.session = session;
  scpinfo.scp = scp;

  snprintf(scpinfo.from, PATH_MAX, "%s", from);

  scpinfo.isRecursive = isRecursive;

  pscpInfo scp_info = &scpinfo;

  if (fileSystemUtils_getFileType(from) == FILE_IS_REG) {
    if (!_scp_copyFileToServer(scp_info, from)) return SSH_ERROR;
  }
  else if (fileSystemUtils_getFileType(from) == FILE_IS_DIR) {
    if (!_scp_copyDirToServer(scp_info, from)) return SSH_ERROR;
  }

  ssh_scp_close(scp);
  ssh_scp_free(scp);
  return SSH_OK;
}

// Push files to server
bool _scp_copyFileToServer(pscpInfo scp_info, const char* file)
{
#ifdef SCP_DEBUG
  printf("scp_copyFileToServer() called with file = '%s'\n", file);
#endif
  int rc;

  size_t size = fileSystemUtils_getFileSize(file);

  FILE* fp = fopen(file, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error while opening %s for reading\n", file);
    return false;
  }

  // Use the same permissions for the server as for local
  int permissions = fileSystemUtils_getFilePermissions(file);

  rc = ssh_scp_push_file(scp_info->scp, file, size, permissions);
  if (rc != SSH_OK) {
    fprintf(stderr, "Can't open remote file: %s\n",
            ssh_get_error(scp_info->session));
    fclose(fp);
    return false;
  }

  size_t bytesRead = 0;
  // TODO: it'd be nice to find a way to copy over the file in segments
  // to allow for a loading bar and to prevent having really large files
  // being copied into memory
  char* buffer = malloc(sizeof(char) * size);

  fread(buffer, size, 1, fp);

  rc = ssh_scp_write(scp_info->scp, buffer, size);

  if (rc != SSH_OK) {
    fprintf(stderr, "Can't write to remote file: %s\n",
            ssh_get_error(scp_info->session));
    free(buffer);
    fclose(fp);
    return false;
  }
  bytesRead += LIBSSH_BUFFER_SIZE;
  free(buffer);
  fclose(fp);

  return true;
}

bool _scp_copyDirToServer(pscpInfo scp_info, const char* dirName)
{
#ifdef SCP_DEBUG
  printf("_scp_copyDirToServer() was called for %s\n", dirName);
#endif
  // Use the same permissions for the remote file as for the local file
  int permissions = fileSystemUtils_getFilePermissions(dirName);

  int rc = ssh_scp_push_directory(scp_info->scp, dirName, permissions);
  if (rc != SSH_OK) {
    fprintf(stderr, "Can't create remote directory: %s\n",
            ssh_get_error(scp_info->session));
    return false;
  }
  int type = FILE_IS_REG;
  char* fileList = fileSystemUtils_D_getDelimitedFileList(dirName);

  // Save the cwd and restore it later
  char oldPath[PATH_MAX];
  fileSystemUtils_getCWD(oldPath);

  // Move the process's working directory to the directory of interest
  fileSystemUtils_chdir(dirName);

  // This list is delimited by commas, so tokenize it with commas
  // This function is recursive, so use strtok_r()
  char* saveptr;
  char* tok = strtok_r(fileList, ",", &saveptr);
  bool success = true;
  while (tok != NULL) {
    // Skip . and ..
    if (strcmp(tok, "..") == 0 || strcmp(tok, ".") == 0) {
      tok = strtok_r(NULL, ",", &saveptr);
      continue;
    }
    // Get the file type
    type = fileSystemUtils_getFileType(tok);

    if (type == FILE_IS_DIR) {
      if (!_scp_copyDirToServer(scp_info, tok)) {
        success = false;
        break;
      }
    }
    else if (type == FILE_IS_REG) {
      if (!_scp_copyFileToServer(scp_info, tok)) {
        success = false;
        break;
      }
    }
    else fprintf(stderr, "Warning: %s is not a regular file or directory\n",
                 tok);

    tok = strtok_r(NULL, ",", &saveptr);
  }
  // fileSystemUtils_D_getDelimitedFileList() returns a character array that
  // must be freed
  free(fileList);

  // restore old cwd
  fileSystemUtils_chdir(oldPath);
  ssh_scp_leave_directory(scp_info->scp);

  return success;
}
