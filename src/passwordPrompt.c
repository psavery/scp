// Written by Patrick S. Avery -- 2015
// Hides the console input while the user inputs the password
// Returns the password

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
