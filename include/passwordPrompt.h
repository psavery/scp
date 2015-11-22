// Written by Patrick S. Avery -- 2015
// Hides the console input while the user inputs the password
// Returns the password

#ifndef PASSWORD_PROMPT_H
#define PASSWORD_PROMPT_H

#define PASSWORD_LENGTH 32

// The statement prints to the screen the password request
// Returns a static char* of max length 32 with the password in it
inline char* passwordPrompt_getPassword(const char* statement);

#endif // PASSWORD_PROMPT_H
