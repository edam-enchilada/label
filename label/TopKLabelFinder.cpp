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
 
 #include <common.h>
#include <TopKLabelFinder.h>

TopKLabelFinder::~TopKLabelFinder() {
  // free all the Search Space in the candidate queue,
  // this step is necessary since the user might stop asking 
  // before K results is returned.
  clearQueue();
}

void TopKLabelFinder::clearQueue() {
  list<SearchSpace *>::iterator it;
  for (it = candidates.begin(); it != candidates.end(); ++it) {
	SearchSpace *sp = *it;
	delete [] sp->low;
	delete [] sp->high;
	delete sp;
  }
  candidates.clear();
}

void TopKLabelFinder::init() {
  int size = pSigDB->numNonEmptySignatures();

  // Create the first search space which is the whole space
  SearchSpace *ps = new SearchSpace;
  ps->low = new int[size];
  ps->high = new int[size];	

  int tileSize = pTile->getSize();
  const double *tileData = pTile->getData();

  double *data  = pSpc->getUnpackedNormalized();
  int spcSize = pSpc->getMaxiumIndex() + 1;



  // The basic filter is applied here
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

  	ps->low[i] = 0;
	if (cError > error) {
	  ps->high[i] = 1;
	  pLabeler->setRange(i, tileData[0], tileData[1]);
	} else {
	  ps->high[i] = tileSize-1;
	  pLabeler->setRange(i, tileData[0], tileData[tileSize-1]);
	}
  }
  delete [] data;  

  
  clearQueue();
  // Caculate the error for the first search space
  QuantityLabel *label = pLabeler->solve();

  if ((label != NULL) && (label->getError() <= this->error)) {
	ps->error = label->getError();  
	ps->qLabel = label;
	candidates.push_back(ps);
  } else {
	delete [] ps->low;
	delete [] ps->high;
	delete ps;
  }

  remain = topK;
}

DiscreteLabel *TopKLabelFinder::getNext() {  
  // If K results have already returned, return NULL;
  if (remain == 0) 
	return NULL;

  int size = pSigDB->numNonEmptySignatures();

  while (!candidates.empty()) {
	//	cout << "queue size : " << candidates.size() << endl;
	//	cout << "reamin : " << remain << endl;

	// Select the current best search space in the queue
	list<SearchSpace *>::iterator iterator = candidates.begin();
	SearchSpace *cp = *iterator;
	candidates.erase(iterator);

	int cut = findCutting(cp);
	
	if (cut == -1) {
	  DiscreteLabel *result = 
		new DiscreteLabel(size, cp->low, cp->qLabel, cp->error);
	  delete [] cp->low;
	  delete [] cp->high;
	  delete cp->qLabel;
	  delete cp;
	  remain--;
	  return result;
	} else {
	  //	  cout << "cut : " << cut << endl;

	  const double *tileData = pTile->getData();
	  for (int i=0; i < size; i++) 
		pLabeler->setRange(i, tileData[cp->low[i]], tileData[cp->high[i]]);

	  for (int k=cp->low[cut]; k < cp->high[cut]; k++) {
	  //	cout << "(" << tileData[k] << "," << tileData[k+1] << ")" << endl;

		// create a new search space which is the subspace of the
		// selected one (cp)
		SearchSpace *newp = new SearchSpace;
		int size = pSigDB->numNonEmptySignatures();
		newp->low = new int[size];
		memmove(newp->low, cp->low, sizeof(int) * size);
		newp->high = new int[size];
		memmove(newp->high, cp->high, sizeof(int) * size);

		newp->low[cut] = k;
		newp->high[cut] = k+1;

		// Find the error of the new sub search space
		pLabeler->setRange(cut, tileData[k], tileData[k+1]);
		QuantityLabel *label = pLabeler->solve();
		if ((label != NULL) && (label->getError() <= this->error)) {
		  //		  cout << "new label" << endl;

		  newp->error = label->getError();
		  newp->qLabel = new QuantityLabel(*label);
		  
		  // Insert the new search space into the queue
		  bool inserted = FALSE;
		  list<SearchSpace *>::iterator it;
		  for (it = candidates.begin(); it != candidates.end(); it++) {
			SearchSpace *op = *it;
			if (op->error > newp->error) {
			  candidates.insert(it, newp);
			  inserted = TRUE;
			  break;
			}
		  }
		  if ((!inserted) && (candidates.size() < remain)) 
			candidates.push_back(newp);

		  //		  cout << "queue size " << candidates.size(); 
		  
		  // If the queue is too long, delete the worst one
		  // in the queue
		  if (candidates.size() > remain) {
			list<SearchSpace *>::iterator rit = candidates.end();
			rit--;
			SearchSpace *dp = *rit;
			candidates.erase(rit);
			
			delete [] dp->low;
			delete [] dp->high;
			delete dp;
		  }
		}

		delete label;

	  }
	}
  }
  
  // The candidate queue is empty, not more result
  return NULL;
}

// Find the best cut of a certain search space
// TODO :
//        The search space should be packaed as a class
int TopKLabelFinder::findCutting(SearchSpace *sp) {
  int size = pSigDB->numNonEmptySignatures();

  for (int i=0; i < size; i++) 
	if (sp->low[i] + 1 != sp->high[i]) 
	  return i;
  
  return -1;
}
