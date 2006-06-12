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

#include <SignatureGenerator.h>
#include <math.h>

SignatureGenerator::SignatureGenerator(const SignatureDatabase *sd) {
	pSigDB = sd;
	pCurrent = NULL;
}

SignatureGenerator::~SignatureGenerator() {
	if (pCurrent != NULL)
		delete pCurrent;
}

void SignatureGenerator::init() {
	if (pCurrent != NULL)
		delete pCurrent;
	pCurrent = NULL;
}

void SignatureGenerator::addComponent(const Signature *pSig) {
	if (pCurrent == NULL) 
		pCurrent = new Signature(*pSig);
	else 
		pCurrent->mergeWith(*pSig);
}

void SignatureGenerator::addComponentByName(string name) {
	const Signature *pSig = pSigDB->getSignatureByName(name);
	return addComponent(pSig);
}

Signature *SignatureGenerator::getResultSignature() {
 	return new Signature(*pCurrent);
}

Signature *SignatureGenerator::generateSignatureFromSymbol(const Symbol &symbol) {
	vector<NameQuantityPair *> compound = symbol.getCompound();
	init();
	for (vector<NameQuantityPair *>::iterator it = compound.begin();
		it != compound.end(); it++) {
			NameQuantityPair *p = *it;
			for (int i=0; i < p->quantity; i++) 
				addComponentByName(p->eleName);
		}
	pCurrent->setName(symbol.getName());

	const CoinPackedVector *cpv = pCurrent->getNormalized();
	const int *oi = cpv->getIndices();
	const double *oe = cpv->getElements();
	
	int ev = symbol.getEV();
	int size = cpv->getNumElements();
	int *index = new int[size];
	double *value = new double[size];
	int nsize = 0;
	for (int i=0; i < size; i++) {
	  int nind = int(rint(double(oi[i]) / ev));
	  int j;
	  for (j=0; j < nsize; j++) 
		if (index[j] == nind)
		  break;

	  if (j == nsize) {
		index[j] = nind;
		value[j] = oe[i];
		nsize++;
	  } else {
		value[j] += oe[i];
	  }
	}

	Signature * newSig = new Signature(nsize, index, value);
	newSig->setName(pCurrent->getName());
	delete pCurrent;
	pCurrent = newSig;
	
	return new Signature(*pCurrent);
}
