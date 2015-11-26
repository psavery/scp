// Author -- Patrick S. Avery -- 2015


#ifndef SCP_H
#define SCP_H

#include <libssh/libssh.h>
#include <stdbool.h>
#include <linux/limits.h>

#define LIBSSH_BUFFER_SIZE 16384

typedef struct {
  ssh_session session;
  ssh_scp scp;
  char from[PATH_MAX];
  bool isRecursive;
} scpInfo;

typedef scpInfo* pscpInfo;

int scp_copyFromServer(ssh_session session, char* from,
                       char* to, bool isRecursive);
int scp_copyToServer(ssh_session session, char* from,
                     char* to, bool isRecursive);

// scp needs to be completely set-up and the pull-request already made and
// accepted before this is called
static bool _scp_copyFileFromServer(pscpInfo scp_info, const char* destination);

// scp needs to be completely set-up and the pull-request already made and
// accepted before this is called
static bool _scp_copyDirFromServer(pscpInfo scp_info, const char* destination);

static bool _scp_handlePullRequest(pscpInfo scp_info, const char* destination,
                                   int pullRequestRet);

static bool _scp_copyFileToServer(pscpInfo scp_info, const char* from);
static bool _scp_copyDirToServer(pscpInfo scp_info, const char* from);

#endif // SCP_H
