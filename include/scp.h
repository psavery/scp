// Author -- Patrick S. Avery -- 2015


#ifndef SCP_H
#define SCP_H

#include <libssh/libssh.h>
#include <stdbool.h>

#define LIBSSH_BUFFER_SIZE 16384

int scp_copyFromServer(ssh_session session, char* fileName, char* destination);

// scp needs to be completely set-up and the pull-request already made and
// accepted before this is called
bool _scp_copyFileFromServer(ssh_session session, ssh_scp scp,
                             const char* fileName, const char* destination,
                             bool singleFileRequest);

// scp needs to be completely set-up and the pull-request already made and
// accepted before this is called
bool _scp_copyDirFromServer(ssh_session session, ssh_scp scp,
                            const char* fileName, const char* destination);

bool _scp_handlePullRequest(ssh_session session, ssh_scp scp,
                            const char* fileName, const char* destination,
                            int rc, bool singleFileRequest);

#endif // SCP_H
