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

int scp_copyFromServer(ssh_session session, char* from,
                       char* destination, bool isRecursive)
{
  // First, just make the initial preparations for scp...
  ssh_scp scp;
  int rc;

  // SSH_SCP_RECURSIVE allows us to detect if it's a directory
  scp = ssh_scp_new(session, SSH_SCP_READ | SSH_SCP_RECURSIVE, from);

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

  // Set up the scp info
  scpInfo scpinfo;
  scpinfo.session = session;
  scpinfo.scp = scp;
  snprintf(scpinfo.from, PATH_MAX, "%s", from);
  scpinfo.isRecursive = isRecursive;

  // A single file was requested...
  if (rc == SSH_SCP_REQUEST_NEWFILE) scpinfo.isRecursive = false;

  pscpInfo scp_info = &scpinfo;

  if (!_scp_handlePullRequest(scp_info, destination, rc))
    return SSH_ERROR;

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

  if (!success) return false;

  return true;
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

  // If a single file is being requested, the pull request will return
  // SSH_SCP_REQUEST_EOF at the end
  if (!scp_info->isRecursive) {
    rc = ssh_scp_pull_request(scp_info->scp);

    if (rc != SSH_SCP_REQUEST_EOF) {
      fprintf(stderr, "Unexpected request: %s\n",
              ssh_get_error(scp_info->session));
      return false;
    }
  }

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

