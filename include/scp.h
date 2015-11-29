/**********************************************************************
  scp.h - Header file for the scp functions

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef SCP_H
#define SCP_H

#include <libssh/libssh.h>
#include <stdbool.h>
#include <linux/limits.h>

// A helper struct that contains scp info
typedef struct {
  ssh_session session;
  ssh_scp scp;
  char from[PATH_MAX];
  bool isRecursive;
} scpInfo;

// The pointer to be passed around
typedef scpInfo* pscpInfo;

/*
 * Primary function to copy a file/dir from a remote computer to a local machine
 *
 * @param session A session that has already been connected to the server.
 * @param from The path to the file or directory to be copied on the server.
 * @param to The path to the local destination for the copied file or dir.
 * @param isRecursive Set this false if you do not want directories to be copied
 *
 * @return Returns SSH_OK if it succeeded and something else if it failed
 * (potentially SSH_ERROR)
 */
int scp_copyFromServer(ssh_session session, char* from,
                       char* to, bool isRecursive);

/*
 * Primary function to copy a file/dir from a local machine to a remote computer
 *
 * @param session A session that has already been connected to the server.
 * @param from The path to the file or directory to be copied on the local
 * machine.
 * @param to The path to the remote destination for the copied file or dir.
 * @param isRecursive Set this false if you do not want directories to be copied
 *
 * @return Returns SSH_OK if it succeeded and something else if it failed
 * (potentially SSH_ERROR)
 */

int scp_copyToServer(ssh_session session, char* from,
                     char* to, bool isRecursive);

// Disable doxygen parsing
/// \cond

/*
 * Function to copy a file from a server using an already set-up ssh_session and
 * ssh_scp. The scp needs to be completely set-up and the pull-request already
 * made and accepted before this function is called.
 *
 */
static bool _scp_copyFileFromServer(pscpInfo scp_info, const char* destination);

/*
 * Function to copy a directory from a server using an already set-up
 * ssh_session and ssh_scp. The scp needs to be completely set-up and the
 * pull-request already made and accepted before this function is called.
 *
 */
static bool _scp_copyDirFromServer(pscpInfo scp_info, const char* destination);

/*
 * Function to handle a pull request once the libssh pull request function
 * has been called. It will then decide whether to call
 * _scp_copyFileFromServer() or _scp_copyDirFromServer()
 * It is only used with scp_copyFromServer() - not scp_copyToServer()
 *
 */
static bool _scp_handlePullRequest(pscpInfo scp_info, const char* destination,
                                   int pullRequestRet);

/*
 * Function called to copy a file to a server from a specific location.
 * The reason scp_info->from is not used is because the "from" location
 * may need to change in the case that it is recursive.
 *
 */
static bool _scp_copyFileToServer(pscpInfo scp_info, const char* from);

/*
 * Function called to copy a dir to a server from a specific location.
 * The reason scp_info->from is not used is because the "from" location
 * may need to change in the case that it is recursive.
 * This function will be called recursively for directories inside directories.
 *
 */
static bool _scp_copyDirToServer(pscpInfo scp_info, const char* from);

// Resume doxygen parsing
/// \endcond

#endif // SCP_H
