/**********************************************************************
  fileSystemUtils.h - Header file for file system utilities for the scp

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef FILE_SYSTEM_UTILS_H
#define FILE_SYSTEM_UTILS_H

#define DEFAULT_MODE_T 0755

#include <stdbool.h>

// A basic enum for file types
enum identify_file_type_e {
  FILE_IS_REG = 0,
  FILE_IS_DIR,
  UNKNOWN_FILE_TYPE,
  READ_ERROR,
  DOES_NOT_EXIST
};

/*
 * Discovers and returns the type of a given file.
 *
 * @param path The path to the file to be investigated
 *
 * @return Returns an identify_file_type_e enum with the type of file.
 */
int fileSystemUtils_getFileType(const char* path);

/*
 * Discovers and returns the mode type of a given file.
 *
 * @param path The path of the file to be investigated
 *
 * @return Returns the mode of a file as a decimal integer.
 */
int fileSystemUtils_getFilePermissions(const char* path);

/*
 * Discovers and returns the size of a given file.
 *
 * @param path The path of the file to be investigated
 *
 * @return Returns the size of the file as a size_t
 */
size_t fileSystemUtils_getFileSize(const char* path);

/*
 * Makes a directory of a given path if one does not already exist.
 * Will also create any parent directories if they do not exist either.
 *
 * @param path The path of the directory to be created
 *
 * @return Returns true if it succeeded and false if it failed
 */
bool fileSystemUtils_mkdirIfNeeded(const char* path);

/*
 * Changes the programs running directory to be in a specified directory.
 *
 * @param path The path of the location to change the directory.
 *
 * @return Returns true if it succeeded and false if it failed
 */
bool fileSystemUtils_chdir(const char* path);

/*
 * Writes the current working directory to the character array that is input.
 * Make sure the parameter is of size PATH_MAX before inputting it into this
 * function.
 *
 * @param string The string to be written to with the current working directory
 * path. Make sure it is of size PATH_MAX before passing it.
 *
 * @return Returns true if it succeeded and false if it failed
 */
// Make sure string is of size PATH_MAX before passing it here
bool fileSystemUtils_getCWD(char* string);

/*
 * Returns a dynamically allocated character array that contains a
 * comma delimited list of the files in a directory
 * The returned character array needs to be freed by calling free()
 * The function currently DOES include "." and ".." in the return.
 *
 * @param dirName The name of the directory from which to obtain the list of
 * files and directories that it obtains.
 *
 * @return A pointer to the new dynamically allocated character array that
 * contains the comma-delimited list of files. Be sure to free this when
 * finished using it.
 */

char* fileSystemUtils_D_getDelimitedFileList(const char* dirName);

#endif
