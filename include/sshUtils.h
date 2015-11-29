/**********************************************************************
  sshUtils.h - Header file for some ssh utility functions

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef SSH_UTILS_H
#define SSH_UTILS_H

#include <stdbool.h>

#define USER_SIZE 32
#define HOST_SIZE 32
#define FILE_PATH_SIZE 128
#define PASS_SIZE 32

// Struct that contains the ssh info
typedef struct {
  char user[USER_SIZE];
  char host[HOST_SIZE];
  char filePath[FILE_PATH_SIZE];
  char pass[PASS_SIZE];
  int port;
  bool isLocal;
} sshInfo;

typedef sshInfo* psshInfo;

/*
 * Checks to see if the file is local. Currently just checks to see if the
 * path contains a ":" and returns true if it does.
 *
 * @param path The path to check if it is local.
 *
 * @return Returns true if it is local and false if it is remote.
 */
bool sshUtils_fileIsLocal(const char* path);

// function reads the input and sets the struct to be the values given.
/*
 * Reads the input from the character array and sets up the sshInfo struct
 * based upon it.
 *
 * @param input The character array that contains the information to be read
 * into the struct pointed to by psshInfo.
 * @param info A pointer to an sshInfo struct whose members will be set.
 *
 * @return Returns true if the read was successful and false if the read
 * failed.
 */
bool sshUtils_setSCPInfo(char* input, psshInfo info);

#endif
