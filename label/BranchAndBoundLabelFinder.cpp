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

#include <iostream>
using namespace std;

#include <BranchAndBoundLabelFinder.h>

void BranchAndBoundLabelFinder::init() {
  // Initialize the search stack	
  const int size = pSigDB->numNonEmptySignatures();
  if (searchStack != NULL)
	delete [] searchStack;
  searchStack = new SearchState[size];
  currStackPosition = 0;
  firstTime = true;

  //  int vz = 0;
  
  // Initialize the search space and set the initial constratins
  // of LP solver 
  if (searchSpace.low != NULL)
	delete [] searchSpace.low;
  searchSpace.low = new int[size];
  
  if (searchSpace.high != NULL)
	delete [] searchSpace.high;
  searchSpace.high = new int[size];

  const double *tileData = pTile->getData();
  int tileSize = pTile->getSize();

  double *data  = pSpc->getUnpackedNormalized();
  int spcSize = pSpc->getMaxiumIndex() + 1;

  for (int i=0; i < size; i++) {
	const Signature *pSig = pSigDB->getSignatureByIndex(i);
	const CoinPackedVector *v = pSig->getNormalized();
	const int * indices = v->getIndices();
	const double * elements = v->getElements();
	int nElem = v->getNumElements();

	double cError = 0;

	for (int j=0; j < nElem; j++) {
	  double pv = elements[j] * tileData[1];

	  if (indices[j] >= spcSize)
		cError += pv;
	  else
		if (pv > data[indices[j]])
		  cError += pv - data[indices[j]];
	  
	  if (cError > error) 
	    break;
	}

  	searchSpace.low[i] = 0;
	if (cError > error) {
	  searchSpace.high[i] = 1;
	  pLabeler->setRange(i, tileData[0], tileData[1]);
	} else {
	  searchSpace.high[i] = tileSize-1;
	  pLabeler->setRange(i, tileData[0], tileData[tileSize-1]);
	}
  }

  delete [] data;
}

BranchAndBoundLabelFinder::~BranchAndBoundLabelFinder() {
  delete [] searchStack;
  delete [] searchSpace.low;
  delete [] searchSpace.high;
}

int BranchAndBoundLabelFinder::findCutting(int currentLevel) {
  int size = pSigDB->numNonEmptySignatures();
  for (int i=currentLevel; i < size; i++) 
	if (searchSpace.low[i] + 1 != searchSpace.high[i]) 
	  return i;
    else
	  // The quantity of this signature has been fixed to 0
	  // we need to increase the compensateCounter here
	  // If there is no optimization, we need to do d extra LP
	  // here, where d is the fanout of the search tree.
	  compensateCounter += pTile->getSize() - 1;
  return -1;
}
	  
DiscreteLabel *BranchAndBoundLabelFinder::getNext() {
  DiscreteLabel *result = NULL;
  
  //  enum BranchAndBoundSearchState stage;

  if (firstTime) {
	// This is the first time getNext() is called
	stage = STATE_TEST;
	firstTime = false;
  } else {
	if (currStackPosition != 0) 
	  stage = STATE_ITERATE;
	else
	  // This situation is possible because of the existance
	  // of empty spectrum
	  return NULL;
  }

  QuantityLabel *label = NULL;
	
  do {
	switch (stage) {

	case STATE_TEST : {
	  label = pLabeler->solve();

	  if ((label != NULL) && (label->getError() < error)) {
		// current search space contains feasible label
		// we need to dig into it
		stage = STATE_CUT;
	  } else {
		delete label;

		if (currStackPosition == 0) {
		  result = NULL;
		  stage = STATE_DONE;
		} else 
		  stage = STATE_ITERATE;
	  }
	}
	  break;

	case STATE_CUT : {

	  // cout << "cut" << endl;
	  // In this stage, the current searchspace need to be
	  // split
	  int cut = findCutting(currStackPosition);
	  if (cut == -1) {
		// there is no possible cutting, the current search space
		// reach the finest granurality, return the result
		// result = pLabler->solve();
 
		result = new DiscreteLabel(pSigDB->numNonEmptySignatures(), searchSpace.low, label, label->getError());

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

	  delete label;
	}
	  break;

    case STATE_ITERATE : {
	  //	  cout << "iterate" << endl;

	  SearchState *cs = &searchStack[currStackPosition-1];
	  cs->curr++;
	  //	  cout << cs->cut << "," << cs->curr << endl << ".";

	  if (cs->curr == cs->high) 
		// We have try all the cuts for this dimension
		// It is time to go back
		stage = STATE_TRACEBACK;
	  else {
		searchSpace.low[cs->cut] = cs->curr;
		searchSpace.high[cs->cut] = cs->curr+1;
		const double *tileData = pTile->getData();
		//		cout << tileData[cs->curr] << "< x" << cs->cut << " < " << tileData[cs->curr+1] << endl;
		pLabeler->setRange(cs->cut, tileData[cs->curr], tileData[cs->curr+1]);

		stage = STATE_TEST;
	  }
	}		
	  break;

	case STATE_TRACEBACK : {
	  SearchState *ss = &searchStack[currStackPosition-1];

	  //	  cout << "traceback" << endl;

	  // Restore the search space
	  searchSpace.low[ss->cut] = ss->low;
	  searchSpace.high[ss->cut] = ss->high;
	  const double *tileData = pTile->getData();
	  pLabeler->setRange(ss->cut, tileData[ss->low], tileData[ss->high]);

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
