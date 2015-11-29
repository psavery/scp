/**********************************************************************
  loadBar.h - Header file for the loading bar function

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef LOAD_BAR_H
#define LOAD_BAR_H

/*
 * A loading bar that will be displayed and updated on the console as
 * progress is made.
 *
 * @param x The progress that has been made
 * @param n The total progress to be made
 * @param r The resolution of the loading bar (i. e., how many times to update)
 * @param w The width of the loading bar in characters
 * @param fileName The file name to be displayed
 *
 */
inline void loadBar_loadBar(int x, int n, int r, int w, const char* fileName);

#endif // LOAD_BAR_H
