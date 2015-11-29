/**********************************************************************
  connectSSH.c - Source code for the ssh connection function

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <connectSSH.h>
#include <passwordPrompt.h>

// Even though ssh_session is already a pointer to a struct, the pointer gets
// altered by ssh_new(). So we need to pass ssh_session into connectSession()
// as a pointer.
ssh_session connectSSH_getConnectedSession(psshInfo info)
{
  // Create session
  ssh_session session = ssh_new();
  if (!session) {
    return NULL;
  }

  if (!info->host) {
    fprintf(stderr, "Error in connectSSH_getConnectedSession(). Host was not set.\n");
    return NULL;
  }

  // Set options
  int verbosity = SSH_LOG_NOLOG;
  //int verbosity = SSH_LOG_PROTOCOL;
  //int verbosity = SSH_LOG_PACKET;
  int timeout = 15; // timeout in sec

  ssh_options_set(session, SSH_OPTIONS_HOST, info->host);
  ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
  ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);

  if (info->user) {
    ssh_options_set(session, SSH_OPTIONS_USER, info->user);
  }
  ssh_options_set(session, SSH_OPTIONS_PORT, &info->port);

  // Connect
  if (ssh_connect(session) != SSH_OK) {
    printf("SSH error: %s", ssh_get_error(session));
    return NULL;
  }

  // Verify that host is known
  int state = ssh_is_server_known(session);
  switch (state) {
  case SSH_SERVER_KNOWN_OK:
    break;
  case SSH_SERVER_KNOWN_CHANGED:
  case SSH_SERVER_FOUND_OTHER:
  case SSH_SERVER_FILE_NOT_FOUND:
  case SSH_SERVER_NOT_KNOWN: {
    int hlen;
    unsigned char *hash = 0;
    char *hexa;
    printf("Error. Host is not known.");
    // hlen = ssh_get_pubkey_hash(session, &hash);
    // hexa = ssh_get_hexa(hash, hlen);
    return NULL;
  }
  case SSH_SERVER_ERROR:
    printf("SSH error: %s", ssh_get_error(session));
    return NULL;
  }

  // Authenticate
  int rc;
  int method;

  // Try to authenticate
  rc = ssh_userauth_none(session, NULL);
  if (rc == SSH_AUTH_ERROR) {
    printf("SSH error: %s", ssh_get_error(session));
    return NULL;
  }

  method = ssh_auth_list(session);
  // while loop here is only so break will work. If execution gets
  // to the end of the loop, the function returns false.
  while (rc != SSH_AUTH_SUCCESS) {

    // Try to authenticate with public key first
    if (method & SSH_AUTH_METHOD_PUBLICKEY) {
      rc = ssh_userauth_autopubkey(session, info->user);
      if (rc == SSH_AUTH_ERROR) {
        printf("Error during auth (pubkey)");
        printf("Error: %s", ssh_get_error(session));
        return NULL;
      } else if (rc == SSH_AUTH_SUCCESS) {
        break;
      }
    }

    // Try to authenticate with password
    if (method & SSH_AUTH_METHOD_PASSWORD) {
      char request[sizeof(char) * (34 + strlen(info->user) + strlen(info->host))];
      snprintf(request, sizeof(request), "Please enter the password for %s@%s ", info->user, info->host);
      snprintf(info->pass, PASS_SIZE, "%s", passwordPrompt_getPassword(request));

      rc = ssh_userauth_password(session,
                                 info->user,
                                 info->pass);
      if (rc == SSH_AUTH_ERROR) {
        printf("Error during auth (passwd)");
        printf("Error: %s", ssh_get_error(session));
        return NULL;
      } else if (rc == SSH_AUTH_DENIED) {
        printf("Error. Authentication denied with passwd!\n");
        return NULL;
      } else if (rc == SSH_AUTH_SUCCESS) {
        break;
      }
    }

    return NULL;
  }
  return session;
}

inline void connectSSH_disconnectSession(ssh_session* session)
{
  if (session) ssh_free(*session);
  *session = 0;
}
