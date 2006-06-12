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

#include <SimpleGroupLabelFinder.h>
#include <BranchAndBoundLabelFinder.h>

SimpleGroupLabelFinder::~SimpleGroupLabelFinder() {
  clearCandidateSet();  
  delete pLabeler;
  delete minor;
  pLabeler = NULL;
  qLabeler= NULL;
}

void SimpleGroupLabelFinder::findMinor() {
  int spcNum = spectra.size();
  int sigNum = pSigDB->numNonEmptySignatures();

  // The minor array serves as a prefilter. if in the spectrum 
  // the ith m/z has a very small value but for signature k. the 
  // ith m/z has large value, that implies that the quantity of 
  // signature j has an upper bound. If the upper bound is
  // less than tileData[1], then the discretized quantity for signature
  // j will always be 0.
  if (minor != NULL)
	delete [] minor;
  minor = new int[spcNum * sigNum];
  memset(minor, 0, sizeof(int) * spcNum * sigNum);

  const double *tileData = pTile->getData();

  for (int i=0; i < sigNum; i++) {
	const Signature *pSig = pSigDB->getSignatureByIndex(i);
	const CoinPackedVector *v = pSig->getNormalized();
	const int * indices = v->getIndices();
	const double * elements = v->getElements();
	int nElem = v->getNumElements();

	int count = 0;
	// For each spectrum, check if the quanity of signautre i
	// is possible to exceed the minimial requirement
	// This optimization is used only for Mahatten Distance
	for (vector<Spectrum *>::iterator it = spectra.begin();
		 it != spectra.end(); it++, count++) {
	  Spectrum *pSpc = *it;
		
	  double *data  = pSpc->getUnpackedNormalized();
	  int spcSize = pSpc->getMaxiumIndex() + 1;
	  
	  double cError = 0;

	  for (int j=0; j < nElem; j++) {
		double pv = elements[j] * tileData[1];
		if (indices[j] >= spcSize)
		  cError += pv;
		else
		  if (pv > data[indices[j]])
			cError += pv - data[indices[j]];

		if (cError > error) {
		  minor[i * spcNum + count] = 1;
		  break;
		}
	  }
	  
	  delete [] data;
	}
  }
}

// the init() method contains two steps
// In the first, step we find the feasible labels of the first
// m specta, where m=(1-support)*N. Every group label should be 
// one of them.
// In the second setp, we check the candidate label with the 
// remaining spectra.
void SimpleGroupLabelFinder::init() {
  const double *tileData = pTile->getData();
  int setSize = spectra.size();

  pLabeler->setTile(pTile);
  pLabeler->setErrorBound(error);  

  findMinor();

  int maxFail = (int)((1 - support) * setSize + 1);
  //  cout << "SGLF : maxFail : " << maxFail << endl;
  int procCnt = 0;
  int sc = 0;
  // Generate the feasible discrete label for the first spectrum
  vector<Spectrum *>::iterator it = spectra.begin();
  while ((procCnt < maxFail) && (it != spectra.end())) {
	//	cout << "SGLF : " << procCnt << endl;

	Spectrum *sp = *it;
	/*	cout << "SGLF - spectrum" << endl;
		sp->dump();*/

	pLabeler->setSpectrum(sp);	
	
	// Here is an optimization trick, we first add the fail count
	// for each candidate labels by one. Later if the candidate label
	// turns out to be the true label of current spectrum, we will minus
	// it. The benifit is that it save LP calls
	for (CandidateIterator cit = candidates.begin();
		 cit != candidates.end(); cit++) 
		   (*cit).second++;

	// Find all the feasible label for the current spectrum
	// and checkout whether i has already been discovered before
	// if not, add new candidate label into the set.
	pLabeler->init();
	DiscreteLabel *label = pLabeler->getNext();
	while (label != NULL) {
	  CandidateIterator cit = candidates.find(label);
	  if (cit == candidates.end()) 
		// Since here is the first time label shows up, that lebel
		// should be infeasible for the previous spectra
		candidates[label] = procCnt;
	  else
		// This is a label, recover the fail count
		(*cit).second--;
	  
	  //	  label->dump();
	  //	  cout << endl;
	  label = pLabeler->getNext();
	}

	procCnt++;
	it++;
	sc++;
	//	cout << sc << endl;
  }
		 
  
  // For the remaining spectra, check them with the found labels
  while (it != spectra.end()) {
	qLabeler->setSpectrum(*it);
	  
	CandidateIterator cit = candidates.begin();

	while (cit != candidates.end()) {
	  DiscreteLabel *dLabel = (*cit).first;
	  int *data = dLabel->getData();
	  int size = dLabel->getSize();

	  bool ok = true;
	  for (int i=0; i < size; i++) {
		if ((data[i] != 0) && (minor[i * setSize + sc] == 1)) {
		  ok = false;
		  break;
		}
		qLabeler->setRange(i, tileData[data[i]], tileData[data[i]+1]);
	  }
		
	  if (ok) {
		QuantityLabel *ql = qLabeler->solve();
		
		if ((ql == NULL) || (ql->getError() >= error)) 
		  (*cit).second++;  // increase the count

		delete ql;
	  } else {
		compensateCounter++;
		(*cit).second++;
	  }

	  // If the count is bigger than the threshold
	  // remove the discrete label from the candidate set
	  if ((*cit).second >= maxFail) {
		CandidateIterator old = cit;
		cit++;
		candidates.erase(old);
	  } else 
		cit++;
	}
	
	it++;
	sc++;
	//	cout << sc << endl;
  };
  
  currSolution = candidates.begin();
}

DiscreteLabel *SimpleGroupLabelFinder::getNext() {
  if (currSolution != candidates.end()) {
	DiscreteLabel *cp = (*currSolution).first;
	DiscreteLabel *result = new DiscreteLabel(*cp);
	currSolution++;
	return result;
  } else 
	return NULL;
}
	
void SimpleGroupLabelFinder::clearCandidateSet() {
  for (CandidateIterator it = candidates.begin();
	   it != candidates.end(); it++) 
	delete (*it).first;
  candidates.clear();
}
