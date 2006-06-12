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

#include <BBGroupLabelFinder.h>
#include <BranchAndBoundLabelFinder.h>

#include <iostream>
using namespace std;

void BBGroupLabelFinder::init() {
  // Initialize the search stack	
  const int size = pSigDB->numNonEmptySignatures();
  searchStack = new SearchState[size];
  currStackPosition = 0;  
  
  // Initialize the search space and set the initial constratins
  // of LP solver   
  searchSpace.low = new int[size];
  searchSpace.high = new int[size];
  const double *tileData = pTile->getData();
  int tileSize = pTile->getSize();

  int spcNum = spectra.size();

  // The voteMark array keep the information about whether
  // the spectrum has voted no previously in the current 
  // search path. If the spectrum has already vote no previously
  // Then it does not need to take part in the later vote
  if (voteMark != NULL)
	delete [] voteMark;
  voteMark = new int[spcNum];
  for (int i=0; i < spcNum; i++) 
	voteMark[i] = -1;

  // The minor array serves as a prefilter. if in the spectrum 
  // the ith m/z has a very small value but for signature k. the 
  // ith m/z has large value, that implies that the quantity of 
  // signature j has an upper bound. If the upper bound is
  // less than tileData[1], then the discretized quantity for signature
  // j will always be 0.
  if (minor != NULL)
	delete [] minor;
  minor = new int[spcNum * size];
  memset(minor, 0, sizeof(int) * spcNum * size);

  for (int i=0; i < size; i++) {
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

	  searchSpace.low[i] = 0;
	  searchSpace.high[i] = tileSize-1;
	  qLabeler->setRange(i, tileData[0], tileData[tileSize-1]);
	}
  }

//   for (int i = 0; i < spcNum; i++) {
// 	int vz =0;
// 	for (int j=0; j < size; j++) 
// 	  if (minor[j * spcNum + i] == 1)
// 		//		cout << j << " ";
// 		vz ++;
// 	cout << vz << endl;
//   }
}

BBGroupLabelFinder::~BBGroupLabelFinder() {
  delete [] searchStack;
  delete [] searchSpace.low;
  delete [] searchSpace.high;
  delete [] voteMark;
  delete [] minor;
  delete qLabeler;
}

int BBGroupLabelFinder::findCutting() {
  int size = pSigDB->numNonEmptySignatures();
  for (int i=0; i < size; i++) 
	if (searchSpace.low[i] + 1 != searchSpace.high[i]) 
	  return i;
  return -1;
}

DiscreteLabel *BBGroupLabelFinder::getNext() {
  DiscreteLabel *result = NULL;
  
  enum BranchAndBoundSearchState stage;

  if (currStackPosition == 0) 
	// This is the first time getNext() is called
	stage = STATE_TEST;
  else
	stage = STATE_ITERATE;

  double currSupport;
	
  do {
	switch (stage) {

	case STATE_TEST : {
	  int spcNum = spectra.size();
	  int count = 0;
	  QuantityLabel *label = NULL;

	  int sc = 0;
	  SearchState *cs = NULL;
	  if (currStackPosition != 0)
		cs = &searchStack[currStackPosition-1];

	  for (vector<Spectrum *>::iterator it = spectra.begin();
		   it != spectra.end(); it++, sc++) {
		if (voteMark[sc] == -1) {
		  if ((cs != NULL) && (minor[cs->cut * spcNum + sc] == 1)) {
			// We suppose to do a LP here. But it is by passed because
			// of the optimization. So increase the compensateCounter
			compensateCounter++;
			if (cs->curr == 0)
			  count++;
			else
			  voteMark[sc] = currStackPosition;
		  } else {
			label = qLabeler->solve(*it);
			if ((label != NULL) && (label->getError() < error))
			  count++;
			else 
			  voteMark[sc] = currStackPosition;
			delete label;
		  } 		
		}
	  }
	 
	  if (count >= spcNum * support) {
		// current search space contains feasible label
		// we need to dig into it
		currSupport = (double)count / spcNum;
		stage = STATE_CUT;
	  } else 
		if (currStackPosition == 0) 
		  stage = STATE_DONE;
		else 
		  stage = STATE_ITERATE;
	  break;
	}

	case STATE_CUT : {

	  // In this stage, the current searchspace need to be
	  // split

	  int cut = findCutting();
	  if (cut == -1) {
		// there is no possible cutting, the current search space
		// reach the finest granurality, return the result
		// result = pLabler->solve();
 
		//		cout << "ppp" << pSigDB->numNonEmptySignatures() << endl;

		result = new DiscreteLabel(pSigDB->numNonEmptySignatures(), 
								   searchSpace.low, NULL, currSupport);

		stage = STATE_DONE;
	  } else {
		searchStack[currStackPosition].cut = cut;
		int low = searchSpace.low[cut];
		int high = searchSpace.high[cut];
		searchStack[currStackPosition].low = low;
		searchStack[currStackPosition].high = high;
		searchStack[currStackPosition].curr = low-1;

		currStackPosition++;
		stage = STATE_ITERATE;
	  }
	}
	  break;

	case STATE_ITERATE : {
	  SearchState *cs = &searchStack[currStackPosition-1];
	  cs->curr++;

	  if (cs->curr == cs->high) 
		// We have try all the cuts for this dimension
		// It is time to go back
		stage = STATE_TRACEBACK;
	  else {
		searchSpace.low[cs->cut] = cs->curr;
		searchSpace.high[cs->cut] = cs->curr+1;
		const double *tileData = pTile->getData();
		qLabeler->setRange(cs->cut, tileData[cs->curr], tileData[cs->curr+1]);

		// clear the vote mark of those spectra which voted no
		// in this search lavel
		int spcNum = spectra.size();
		for (int i = 0; i < spcNum; i++)
		  if (voteMark[i] == currStackPosition)
			voteMark[i] = -1;

		//		if (cs->curr != 0) {
		  // use the precomputed filter to cut some spectra
		  //for (int sc = 0; sc < spcNum; sc++) 
		//	if ((minor[cs->cut * spcNum + sc] == 1) && (voteMark[sc] == -1)) 
		//	  voteMark[sc] = currStackPosition;
		//	}

		stage = STATE_TEST;
	  }
	}		
	  break;

	case STATE_TRACEBACK : {
	  SearchState *ss = &searchStack[currStackPosition-1];

	  // Restore the search space
	  searchSpace.low[ss->cut] = ss->low;
	  searchSpace.high[ss->cut] = ss->high;
	  const double *tileData = pTile->getData();
	  qLabeler->setRange(ss->cut, tileData[ss->low], tileData[ss->high]);

	  // Restore the voteMark
	  int spcNum = spectra.size();
	  for (int i=0; i < spcNum; i++) 
		if (voteMark[i] == currStackPosition)
		  voteMark[i] = -1;

	  currStackPosition--;

	  // The search stack is empty, return NULL;
	  if (currStackPosition == 0) {
		result = NULL;
		stage = STATE_DONE;
	  }
	  else 
		stage = STATE_ITERATE;
	}
	  break;

	default :
	  break;
	}
  } while (stage != STATE_DONE);

  return result;
}
