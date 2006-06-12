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

#include <SignatureDatabase.h>

// Initialize the content of a SignatureDatabase;
SignatureDatabase::SignatureDatabase(int size) : CoinPackedMatrix() {
  numSigs = 0;
  numNonEmptySigs = 0;
  sizeSig = size;
  index = new int[size];
  map.clear();
  sigs.clear();
  CoinPackedMatrix::setDimensions(sizeSig, 0);
}

SignatureDatabase::~SignatureDatabase() {
  if (index != NULL)
	delete [] index;
	
  for (vector<const Signature *>::iterator it = sigs.begin();
  	it != sigs.end(); it++) {
  		delete (*it);
  	}
}

void SignatureDatabase::appendSignature(const Signature& sig) {
  //cout << sig.getNormalized()->getNumElements() << endl;
  //if (sig.getNormalized()->getNumElements() != 0) {
	const Signature *newSig = new Signature(sig);
	sigs.push_back(newSig);

	index[numNonEmptySigs] = numSigs;
	//	cout << sig.getNormalized()->getNumElements() << endl;
	map[numSigs] = numNonEmptySigs;
	
	numNonEmptySigs++;
	numSigs++;

	//sig.dump();

	CoinPackedMatrix::appendCol(*(sig.getNormalized()));
  //} else {
  //	map[numSigs] = -1;
  //	numSigs++;
  //}
}

const Signature *SignatureDatabase::getSignatureByName(string name) const{
	for (vector<const Signature *>::const_iterator it = sigs.begin();
		it != sigs.end(); it++) {
			const Signature *s = (*it);
			if (name == s->getName())
				return s;
			}
	return NULL;
}

// The index starts from zero
const Signature *SignatureDatabase::getSignatureByIndex(int i) const {
  if (i >= numNonEmptySigs) 
	return NULL;
  else {
	vector<const Signature *>::const_iterator it = sigs.begin();
	return it[i];
  }
}
