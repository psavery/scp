// Author -- Patrick S. Avery -- 2015


#ifndef CONNECT_SSH_H
#define CONNECT_SSH_H

#include <libssh/libssh.h>

#include <sshUtils.h>

ssh_session connectSSH_getConnectedSession(psshInfo info);

inline void connectSSH_disconnectSession(ssh_session* session);

#endif
