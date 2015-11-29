/**********************************************************************
  passwordPrompt.c - Source code for the password prompt functions.
                     Hides the console input while the user inputs the password
                     Returns a pointer to a 32-byte length static char array
                     That contains the password.

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

// Include Windows header for Windows
#ifdef _WIN32
#include <windows.h>
#else // Include Unix headers for Unix
#include <termios.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>

#include <passwordPrompt.h>

inline char* passwordPrompt_getPassword(const char* statement)
{
// For Windows
#ifdef _WIN32
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(hStdin, &mode);
  SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
#else // For Unix
  struct termios oldt;
  tcgetattr(STDIN_FILENO, &oldt);
  struct termios newt = oldt;
  newt.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif

  static char passwd[PASSWORD_LENGTH];

  if (statement) printf("%s", statement);
  else printf("Enter password: ");

  fgets(passwd, PASSWORD_LENGTH, stdin);

  // Do a rudimentary removal of \n at the end
  char *p;
  if ((p=strchr(passwd, '\n')) != NULL) *p = '\0';

  // Just for cleanliness
  printf("\n");

// Cleanup
#ifdef _WIN32
  SetConsoleMode(hStdin, mode);
#endif
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return passwd;
}
