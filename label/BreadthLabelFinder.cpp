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

#include <BreadthLabelFinder.h>

void BreadthLabelFinder::init() {
  if (genSet != NULL) 
	clearSet(genSet);
  else
	genSet = new CompactSearchSpaceSet();

  if (resSet != NULL) 
	clearSet(resSet);
  else
	resSet = new CompactSearchSpaceSet();

  if (solutionSet != NULL) 
	clearSet(solutionSet);
  else 
	solutionSet = new CompactSearchSpaceSet();

  const int size = pSigDB->numNonEmptySignatures();
  const double *tileData = pTile->getData();
  int tileSize = pTile->getSize();

  int indx[1];
  int rang[1];

  // Try the (0,0,...,0) label, if it is feasible, put it in the solution
  // set
  for (int i=0; i < size; i++) 
	pLabeler->setRange(i, tileData[0], tileData[1]);
  QuantityLabel *label = pLabeler->solve();
  if ((label != NULL) && (label->getError() < error)) {
	CompactSearchSpace *sp = 
	  new CompactSearchSpace(0, indx, rang, label->getError());
	solutionSet->push_back(sp);
  }

  // Setup the initial constraints over signatures
  for (int i=0; i < size; i++) 
	pLabeler->setRange(i, tileData[0], tileData[tileSize-1]);
  
  for (int i=0; i < size; i++) {
	// Here j starts from 1 because we do not care those signature
	// whose quanity is less than tileData[1]. We simply treat those
	// signatures as non-exist
 	for (int j=1; j < tileSize - 1; j++) {
	  pLabeler->setRange(i, tileData[j], tileData[j+1]);
	  QuantityLabel *label = pLabeler->solve();

	  // if there is a feasible solution in the answer
	  // then insert the labeling template into candidate set
	  if ((label != NULL) && (label->getError() < error)) {
		indx[0] = i;
		rang[0] = j;
		CompactSearchSpace *sp = 
		  new CompactSearchSpace(1, indx, rang, label->getError());
		resSet->push_back(sp);
	  }

	  delete label;
	}

	//retore the constrant over ith signatures
	pLabeler->setRange(i, tileData[0], tileData[tileSize-1]);
  }

  dumpSet(resSet);
}

void BreadthLabelFinder::execute() {  
  int currLevel = 1;

  while (currLevel < requiredLevel) {
	clearSet(genSet);
	CompactSearchSpaceSet *temp = genSet;
	genSet = resSet;
	resSet = temp;

	CompactSearchSpaceSetIterator start;
	CompactSearchSpaceSetIterator end;

	const CompactSearchSpace *bs;
	const CompactSearchSpace *es;

	start = genSet->begin();

	while (start != genSet->end()) {
	  bs = *start;
	  end = start;
	  
	  do {
		end++;
		if (end == genSet->end())
		  break;
		es = *end;
	  } while (canMerge(es, bs));

	  for (CompactSearchSpaceSetIterator it0 = start; it0 != end; it0++) {
		CompactSearchSpace *s0 = (*it0);

		for (CompactSearchSpaceSetIterator it1 = it0+1; it1 != end; it1++) {
		  CompactSearchSpace *s1 = (*it1);
		  
		  if (s0->index[s0->size-1] != s1->index[s1->size-1]) {
			CompactSearchSpace *news = new CompactSearchSpace(currLevel + 1);
		  
			memmove(news->index, s0->index, sizeof(int) * (currLevel));
			memmove(news->range, s0->range, sizeof(int) * (currLevel));
			news->index[currLevel] = s1->index[currLevel-1];
			news->range[currLevel] = s1->range[currLevel-1];

			const double *tileData = pTile->getData();
			int tileSize = pTile->getSize();
			for (int i=0; i <= currLevel; i++) 
			  pLabeler->setRange(news->index[i], tileData[news->range[i]], tileData[news->range[i]+1]);

			QuantityLabel *label = pLabeler->solve();
			
			if ((label != NULL) && (label->getError() < error)) {
			  s0->expand = true;
			  s1->expand = true;
			  resSet->push_back(news);
			}

			delete label;
			
			for (int i=0; i <= currLevel; i++) 
			  pLabeler->setRange(news->index[i], tileData[0], tileData[tileSize]);
		  }
		}

		if (!s0->expand) {
		  // s0 can not be expand so it should be output as a feasible label
		  // it0 is set to NULL so that we will not clear s0 in the clearSet() call
		  // This is simply like remove the it0 from the set, However, since the 
		  // CompactSpaceSet is implemented as a vector, we probably would not remove
		  // it0 from genSet because that will simply to too costly
		  *it0 = NULL;
		  solutionSet->push_back(s0);
		}
	  }

	  start = end;
	}

	if (resSet->empty()) 
	  break;

	currLevel++;
  }

  clearSet(genSet);
  clearSet(resSet);
  currSolution = solutionSet->begin();
}		 		 

bool BreadthLabelFinder::canMerge(const CompactSearchSpace *s1, const CompactSearchSpace *s2) {
  if (s1->size != s2->size)
	return false;

  for (int i=0; i < s1->size - 1; i++) 
	if ((s1->index[i] != s2->index[i]) || (s1->range[i] != s2->range[i])) 
	  return false;

  return true;
}

BreadthLabelFinder::~BreadthLabelFinder() {
  clearSet(genSet);
  clearSet(resSet);
  clearSet(solutionSet);
  delete genSet;
  delete resSet;
  delete solutionSet;
}

// remove all the records in the set
void BreadthLabelFinder::clearSet(CompactSearchSpaceSet *set) {
  for (CompactSearchSpaceSetIterator it = set->begin(); it != set->end(); it++) {
	const CompactSearchSpace *sp = (const CompactSearchSpace *)(*it);
	delete sp;
  }
  set->clear();
}

DiscreteLabel *BreadthLabelFinder::getNext() {
  if (currSolution == solutionSet->end())
	return NULL;

  const CompactSearchSpace *sp = (*currSolution);
  DiscreteLabel *label = sp->convertIntoDiscreteLabel(pSigDB->numNonEmptySignatures());
  currSolution++;

  return label;

}

void BreadthLabelFinder::dumpSet(CompactSearchSpaceSet *set) {
  for (CompactSearchSpaceSetIterator it = set->begin(); it != set->end(); it++) 
	if (*it != NULL) 
	  (*it)->dump();
}
