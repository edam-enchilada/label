/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
  *
 * The Initial Developer of the Original Code is
 * The EDAM Project at University of Wisconsin, Madison
 *
 * Contributor(s):
 *  Lei Chen    lchen5@gmail.com
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef SUBSPACE_SET_H
#define SUBSPACE_SET_H

#include <stdlib.h>

struct SubSpaceNode {
  int count;
  struct SubSpaceNode ** child;
};

typedef struct SubSpaceNode SSNode;

class SubSpaceSet {
  public :
	SubSpaceSet(int ts, int vs) : root(NULL),tileSize(ts),vectorSize(vs) {};
  ~SubSpaceSet() { clear(); }
  void set(const int *spaceVector);
  bool get(const int *spaceVector);
  void clear();

  private :
	SSNode * root;
  int tileSize;
  int vectorSize;

  void clearNode(SSNode *node);
  bool getNode(SSNode *node, int level, const int *spaceVector);
  void setNode(SSNode *&node, int level, const int *spaceVector);
};

#endif
