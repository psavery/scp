/**********************************************************************
  passwordPrompt.h - Header file for the password prompt functions.
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

#ifndef PASSWORD_PROMPT_H
#define PASSWORD_PROMPT_H

#define PASSWORD_LENGTH 32

/*
 * A password prompt that hides the user input as they type it into the console
 *
 * @param statement The request statement to be printed to the console when
 * asking for the password.
 *
 * @return Returns a pointer to a static character array that contains the
 * password of PASSWORD_LENGTH (defined in the macro above).
 */
inline char* passwordPrompt_getPassword(const char* statement);

#endif // PASSWORD_PROMPT_H
