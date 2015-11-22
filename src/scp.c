// Author -- Patrick S. Avery -- 2015

#include <linux/limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <scp.h>
#include <loadBar.h>
#include <fileSystemUtils.h>

// Define this macro to produce more debug output
// #define SCP_DEBUG

// Make sure there is enough space in newDestination before putting it into this function
// Easy way is to just make newDestination of size PATH_MAX...
void scp_createModifiedDestination(const char* from, const char* destination, char* newDestination)
{
  snprintf(newDestination, PATH_MAX, "%s", destination);

  int rc = fileSystemUtils_getFileType(destination);
  if (rc == FILE_IS_REG) return;
  else if (rc == FILE_IS_DIR) {
#ifdef _WIN32
    if (destination[strlen(destination) - 1] != '\\' || // User did not add the final slash
        strlen(destination) == 1 && destination[0] == '.' || // Dot
        strlen(destination) == 2 && destination[0] == '.' && destination[1] == '.') // Dot dot
       strcat(newDestination, "\\");
    char* last = strrchr(from, '\\');
#else
    if (destination[strlen(destination) - 1] != '/' || // User did not add the final slash
        strlen(destination) == 1 && destination[0] == '.' || // Dot
        strlen(destination) == 2 && destination[0] == '.' && destination[1] == '.') // Dot dot
      strcat(newDestination, "/");
    char* last = strrchr(from, '/');
#endif

    if (!last) strcat(newDestination, from);
    else strcat(newDestination, last);
    return;
  }
}

int scp_copyFromServer(ssh_session session, char* fileName, char* destination)
{
  // First, just make the initial preparations for scp...
  ssh_scp scp;
  int rc;

  // SSH_SCP_RECURSIVE allows us to detect if it's a directory
  scp = ssh_scp_new(session, SSH_SCP_READ | SSH_SCP_RECURSIVE, fileName);

  if (scp == NULL)
  {
    fprintf(stderr, "Error allocating scp session: %s\n",
            ssh_get_error(session));
    return SSH_ERROR;
  }
  rc = ssh_scp_init(scp);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error initializing scp session: %s\n",
            ssh_get_error(session));
    ssh_scp_free(scp);
    return rc;
  }

  rc = ssh_scp_pull_request(scp);

  if (!_scp_handlePullRequest(session, scp, fileName, destination, rc, true))
    return SSH_ERROR;

  ssh_scp_close(scp);
  ssh_scp_free(scp);
  return SSH_OK;
}

// rc is the return from the pull request
bool _scp_handlePullRequest(ssh_session session, ssh_scp scp,
                            const char* fromName, const char* toName,
                            int rc, bool singleFileRequest)
{
  bool success = false;

  switch (rc) {
    // Requested a directory!
    case SSH_SCP_REQUEST_NEWDIR:
#ifdef SCP_DEBUG
      printf("in _scp_handlePullRequest(), rc is SSH_SCP_REQUEST_NEWDIR\n");
#endif
      if (ssh_scp_accept_request(scp) != SSH_OK)
        fprintf(stderr, "Error accepting scp request: %s\n", ssh_get_error(session));
      else {
        char newToName[PATH_MAX];
        snprintf(newToName, PATH_MAX, "%s/%s", toName, ssh_scp_request_get_filename(scp));

        // Make the local directory if needed
        if (!fileSystemUtils_mkdirIfNeeded(newToName)) {
          fprintf(stderr, "fileSystemUtils_mkdirIfNeeded() failed.\n");
          break;
        }

        success = _scp_copyDirFromServer(session, scp, fromName, newToName);
      }
      break;
    // Requested a file!
    case SSH_SCP_REQUEST_NEWFILE:
#ifdef SCP_DEBUG
      printf("in _scp_handlePullRequest(), rc is SSH_SCP_REQUEST_NEWFILE\n");
#endif
      if (ssh_scp_accept_request(scp) != SSH_OK) {
        fprintf(stderr, "Error accepting scp request: ");
        fprintf(stderr, "%s\n", ssh_get_error(session));
      }
      else {
        char newToName[PATH_MAX];
        int type = fileSystemUtils_getFileType(toName);

        // If the toName is a dir, set the newToName to be dir/fileName
        if (type == FILE_IS_DIR)
          snprintf(newToName, PATH_MAX, "%s/%s", toName, ssh_scp_request_get_filename(scp));

        // If the toName is anything else, just set newToName to be the same
        else if (type == FILE_IS_REG || type == DOES_NOT_EXIST)
          snprintf(newToName, PATH_MAX, "%s", toName);
        else {
          fprintf(stderr, "Error determining type of file for: %s\n", toName);
          return false;
        }

        success = _scp_copyFileFromServer(session, scp, fromName, newToName, singleFileRequest);
      }
      break;
    // A warning was returned
    case SSH_SCP_REQUEST_WARNING:
      fprintf(stderr, "%s", "ssh_scp_pull_request() returned with a warning: ");
      fprintf(stderr, "%s\n", ssh_get_error(session));
      break;
    // An error was returned
    case SSH_ERROR:
      fprintf(stderr, "%s", "ssh_scp_pull_request() returned with an error: ");
      fprintf(stderr, "%s\n", ssh_get_error(session));
      break;
    // An unexpected value was returned
    default:
      fprintf(stderr, "%s%s\n",
              "ssh_scp_pull_request() returned an unexpected value.\n",
              "Please contact the development team to resolve this.\n");
      break;
  }

  if (!success) return false;

  return true;
}

bool _scp_copyFileFromServer(ssh_session session, ssh_scp scp,
                             const char* fileName, const char* destination,
                             bool singleFileRequest)
{
#ifdef SCP_DEBUG
  printf("in _scp_copyFileFromServer(): \n");
  printf("fileName is %s\n", fileName);
  printf("destination is %s\n", destination);
#endif
  int rc = SSH_ERROR;
  size_t fileSize = ssh_scp_request_get_size(scp);
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
    rc = ssh_scp_read(scp, buffer, sizeof(buffer));
    fclose(fp);
    return true;
  }

  do {
    rc = ssh_scp_read(scp, buffer, sizeof(buffer));
    if (rc == SSH_ERROR) {
      fprintf(stderr, "Error reading file: %s\n", ssh_get_error(session));
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

  // If a single file is being requested, the pull request will return
  // SSH_SCP_REQUEST_EOF at the end
  if (singleFileRequest) {
    rc = ssh_scp_pull_request(scp);

    if (rc != SSH_SCP_REQUEST_EOF) {
      fprintf(stderr, "Unexpected request: %s\n",
              ssh_get_error(session));
      return false;
    }
  }

  return true;
}

// This is recursive if more directories exist
// the SSH_SCP_REQUEST_NEWDIR return from ssh_scp_pull_request()
// should have already been received and accepted before calling this function
bool _scp_copyDirFromServer(ssh_session session, ssh_scp scp,
                            const char* fromName, const char* toName)
{
#ifdef SCP_DEBUG
  printf("in _scp_copyDirFromServer(): \n");
  printf("fromName is %s\n", fromName);
  printf("toName is %s\n", toName);
#endif
  int rc = ssh_scp_pull_request(scp);

  while (rc != SSH_SCP_REQUEST_ENDDIR) {
    if (!_scp_handlePullRequest(session, scp, fromName, toName, rc, false))
      return false;
    rc = ssh_scp_pull_request(scp);
  }

  return true;
}

