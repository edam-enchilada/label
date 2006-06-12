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

#include <SubSpaceSet.h>

#include <string.h>

#include <iostream>
using namespace std;

void SubSpaceSet::clear() {
  if (root != NULL)
	clearNode(root);
}

void SubSpaceSet::clearNode(SSNode *node) {
  for (int i=0; i < tileSize; i++) 
	if (node->child[i] != NULL) 
	  clearNode(node->child[i]);
  delete [] node->child;
  delete node;
}

void SubSpaceSet::set(const int *spaceVector) {
  //  cout << "vectorSize " << vectorSize << ", tileSize " << tileSize << endl;

  setNode(root, 0, spaceVector);
}

void SubSpaceSet::setNode(SSNode *&node, int level, const int *spaceVector) {
  if (level < vectorSize) {
	if (node == NULL) {
	  node = new SSNode;
	  node->child = new SSNode*[tileSize];
	  memset(node->child, 0, sizeof(SSNode*) * tileSize);
	} 
	setNode(node->child[spaceVector[level]], level+1, spaceVector);
  }
}

bool SubSpaceSet::get(const int *spaceVector) {
  return getNode(root, 0, spaceVector);
}

bool SubSpaceSet::getNode(SSNode *node, int level, const int* spaceVector) {
  if (level == vectorSize)
	return 1;
  else 
	if (node == NULL)
	  return 0;
	else 
	  return getNode(node->child[spaceVector[level]], level+1, spaceVector);
}
