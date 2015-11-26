/* Written by Patrick S. Avery -- 2015
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <passwordPrompt.h>
#include <scp.h>
#include <sshUtils.h>
#include <connectSSH.h>

int main(int argc, char* argv[])
{
  if (argc != 3) {
    printf("Usage: scp <from> <to>\n");
    return -1;
  }

  char* from = argv[1];
  char* to = argv[2];

  sshInfo fromInfo;
  sshInfo toInfo;

  psshInfo pfromInfo = &fromInfo;
  psshInfo ptoInfo = &toInfo;

  // Set all the info from the 'from' and 'to' character
  // arrays. It automatically determines if they are local or not
  // If the reading fails for some reason, they will return false
  if (!sshUtils_setSSHInfo(from, pfromInfo) ||
      !sshUtils_setSSHInfo(to, ptoInfo)) {
    fprintf(stderr, "Error reading arguments");
    return -1;
  }

  // If both are local, just perform a regular cp...
  // TODO: make it compatible with Windows
  if (fromInfo.isLocal && toInfo.isLocal) {
    size_t size = sizeof(char) * (strlen(from) + strlen(to)) + 5;
    char* cp = malloc(size);
    snprintf(cp, size, "cp %s %s", from, to);

    int ret = system(cp);

    free(cp);
    return ret;
  }

  // We are copying from the server...
  else if (!fromInfo.isLocal && toInfo.isLocal) {
    ssh_session session = connectSSH_getConnectedSession(pfromInfo);
    if (!session) {
      fprintf(stderr, "Error connecting the session: %s\n",
              ssh_get_error(session));
      return -1;
    }

    // Let's just turn recursive mode on...
    bool isRecursive = true;
    if (scp_copyFromServer(session, pfromInfo->filePath, to, isRecursive)
        != SSH_OK) {
      fprintf(stderr, "Error executing scp_copyFromServer()\n");
      connectSSH_disconnectSession(&session);
      return -1;
    }
    connectSSH_disconnectSession(&session);
  }

  // We are copying to the server...
  else if (fromInfo.isLocal && !toInfo.isLocal) {
    ssh_session session = connectSSH_getConnectedSession(ptoInfo);
    if (!session) {
      fprintf(stderr, "Error connecting the session: %s\n",
              ssh_get_error(session));
      return -1;
    }

    // Let's just turn recursive mode on...
    bool isRecursive = true;
    if (scp_copyToServer(session, pfromInfo->filePath, ptoInfo->filePath,
                        isRecursive) != SSH_OK) {
      fprintf(stderr, "Error executing scp_copyFromServer()\n");
      connectSSH_disconnectSession(&session);
      return -1;
    }
    connectSSH_disconnectSession(&session);
  }

  else {
    fprintf(stderr, "%s%s\n", "Copying from one remote location to another ",
            "remote location has not yet been set up");
    return -1;
  }

  fprintf(stdout, "scp complete!\n");
  return 0;
}

