/**********************************************************************
  connectSSH.h - Header file for the ssh connection function

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef CONNECT_SSH_H
#define CONNECT_SSH_H

#include <libssh/libssh.h>

#include <sshUtils.h>

/*
 * Connects a session using the information in an sshInfo struct.
 * See sshUtils.h for the definition of an sshInfo struct.
 *
 * @param info A pointer to an sshInfo struct that contains the info for setting
 * up an ssh connection.
 *
 * @return A session that has been connected. Returns NULL if the connection
 * failed.
 */
ssh_session connectSSH_getConnectedSession(psshInfo info);


/*
 * Disconnects an ssh session. It requires a pointer to the ssh_session so that
 * it may set the ssh_session to be NULL
 *
 * @param session A pointer to an ssh_session to be disconnected
 */
inline void connectSSH_disconnectSession(ssh_session* session);

#endif
