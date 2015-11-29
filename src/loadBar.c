/**********************************************************************
  loadBar.c - Source code for the loading bar function

  Copyright (C) 2015 by Patrick S. Avery

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include <stdio.h>

#include <loadBar.h>

// Process has done x out of n rounds,
// and we want a bar of width w and resolution r.
inline void loadBar_loadBar(int x, int n, int r, int w, const char* fileName)
{
  // Only update r times.
  if ( x % (n / r) != 0 ) return;
  // Calculate the ratio of complete-to-incomplete.
  float ratio = x / (float)n;

  // We shouldn't really have a ratio greater than 1...
  if (ratio > 1) ratio = 1.0;
  int   c     = ratio * w;

  // Print the percentage complete.
  printf("%3d%% [", (int)(ratio*100) );

  // Print the load bar.
  int i;
  for (i = 0; i < c; i++) printf("=");
  for (i = c; i < w; i++) printf(" ");

  // ANSI Control codes to go back to the
  // previous line and clear it if it is not complete
  if (x < n) printf("]  %s\n\033[F\033[J", fileName);

  // Another valid way to return to the previous line
  //if (x < n) printf("]  %s\r", fileName);

  // If we reach the end, just end the bracket and move to the next line
  else printf("]  %s\n", fileName);
}

