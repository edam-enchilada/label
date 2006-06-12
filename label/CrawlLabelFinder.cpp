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

#include <CrawlLabelFinder.h>
#include <iostream>
using namespace std;

CrawlLabelFinder::~CrawlLabelFinder() {
  clearQueue();
  delete [] possible;
}

// Initialize the crawl labeling algorithm

void CrawlLabelFinder::preprocess() {
  int size = pSigDB->numNonEmptySignatures();
  const double *tileData = pTile->getData(); 

  delete [] possible;
  possible = new bool[size];
  memset(possible, 0, sizeof(bool) * size);

  double *data  = pSpc->getUnpackedNormalized();
  int spcSize = pSpc->getMaxiumIndex() + 1;
  
  for (int i=0; i < size; i++) {
	const Signature *pSig = pSigDB->getSignatureByIndex(i);
	const CoinPackedVector *v = pSig->getNormalized();
	const int * indices = v->getIndices();
	const double * elements = v->getElements();
	int nElem = v->getNumElements();
	
    	/*double minQ = 1;
	for (int j=0; j < nElem; j++) {
	  double pv;
	  if (indices[j] >= spcSize)
		pv = this->error / elements[j];
	  else
	    pv = (data[indices[j]] + this->error) / elements[j];
	  if (pv < minQ) {
		minQ = pv;
		if (minQ < tileData[1])
		  break;
	  }
	}

	if (minQ >= tileData[1]) 
	  possible[i] = true;*/
	double sumDiff = 0;
	for (int j=0; j < nElem; j++) {
		double pv = tileData[1] * elements[j];
		if (indices[j] >= spcSize)
			sumDiff += pv;
		else
			if (pv > data[indices[j]])
				sumDiff += pv - data[indices[j]];
	}
	if (sumDiff > this->error)
		possible[i] = false;
	else 	
		possible[i] = true;
  }

	int count = 0;
	for (int i=0; i < size; i++) 
		if (possible[i]) 
			count++;
	cout << "total possible = " << count << endl;
	
  delete [] data;
}

void CrawlLabelFinder::init() {
  int size = pSigDB->numNonEmptySignatures();

  // Set the initial value of the labeler
  int tileSize = pTile->getSize();
  const double *tileData = pTile->getData();
  for (int i=0; i < size; i++) 
	pLabeler->setRange(i, tileData[0], tileData[tileSize-1]);

  // In preprocess method, those signature which are impossible
  // to appear in the spectrum is filtered out
  preprocess();

  // Clearing the queue and the set for solutions
  clearQueue();
  delete ss;
  ss = new SubSpaceSet(tileSize, size);

  QuantityLabel *label = pLabeler->solve();

  if (label != NULL) {
	//	label->dump();
	const double *sol = label->getData();
	double err = label->getError();
	
	if (err < this->error) {
	  DiscreteLabel *pl = new DiscreteLabel(size);
	  int *pData = pl->getData();
	  for (int i=0; i < size; i++) 
		for (int j=0; j < tileSize-1; j++) 
		  if (tileData[j+1] >= sol[i]) {
			pData[i] = j;
			break;
		  }
	  pl->setError(err);
	  pl->setQuantityLabel(*label);
	  queue.push_back(pl);
	  ss->set(pData);
	}

	delete label;
  }
}

DiscreteLabel *CrawlLabelFinder::getNext() {
  if (queue.empty()) {
	return NULL;
  } else {
	list<DiscreteLabel *>::iterator it = queue.begin();
	class DiscreteLabel *result = *it;
	queue.erase(it);
	
	int size = pSigDB->numNonEmptySignatures();
	int tileSize = pTile->getSize();
	const double *tileData = pTile->getData();
	int * resData = result->getData();

	for (int i=0; i < size; i++) 
	  pLabeler->setRange(i, tileData[resData[i]], tileData[resData[i]+1]);	  

	for (int i=0; i < size; i++) 
	  if (possible[i]) {
	  // loopStart and loopEnd determines the neighbors we try to check
	  int loopStart, loopEnd;
	  if (resData[i] == 0) 
		loopStart = 1;
	  else
		loopStart = -1;

	  if (resData[i] == tileSize - 2)
		loopEnd = -1;
	  else
		loopEnd = 1;

	  for (int j=loopStart; j <= loopEnd; j+=2) {
		resData[i]+=j;

		// If the new subspace has not been explored before,
		// it is added in the the queue
		if (!ss->get(resData)) {
		  pLabeler->setRange(i, tileData[resData[i]], tileData[resData[i]+1]);	  
		  QuantityLabel *label = pLabeler->solve();
		  
		  if ((label != NULL) && (label->getError() < this->error)) {
			DiscreteLabel *newl = new DiscreteLabel(size, resData, label, label->getError());
			queue.push_back(newl);
		  }
		  
		  // No matter the new cell is feasible or not
		  // we always need to mark that cell to avoid future unnecessary 
		  // LP checking
		  ss->set(resData);
		  delete label;
		}
		resData[i]-=j;
	  }
		
	  pLabeler->setRange(i, tileData[resData[i]], tileData[resData[i]+1]);
	} 
	
	return result;
  }
}

// free all the discrete label in the queue
void CrawlLabelFinder::clearQueue() {
  for (list<DiscreteLabel *>::iterator it = queue.begin();
	   it != queue.end();
	   ++it) {
	DiscreteLabel *l = *it;
	delete l;
  }

  queue.clear();
}