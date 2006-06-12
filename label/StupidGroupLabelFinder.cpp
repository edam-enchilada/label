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

#include <StupidGroupLabelFinder.h>
#include <BranchAndBoundLabelFinder.h>

StupidGroupLabelFinder::~StupidGroupLabelFinder() {
  clearCandidateSet();  
  delete pLabeler;
  pLabeler = NULL;
  qLabeler= NULL;
}

// the init() method contains two steps
// In the first, step we find the feasible labels of the first
// m specta, where m=(1-support)*N. Every group label should be 
// one of them.
// In the second setp, we check the candidate label with the 
// remaining spectra.
void StupidGroupLabelFinder::init() {
  const double *tileData = pTile->getData();
  int setSize = spectra.size();

  pLabeler->setTile(pTile);
  pLabeler->setErrorBound(error);  

  int spCount = 0;
  for (vector<Spectrum *>::iterator it = spectra.begin(); 
	   it != spectra.end(); it++) {

	Spectrum *sp = *it;
	pLabeler->setSpectrum(sp);	
	pLabeler->init();

	int sCount = 0;
	DiscreteLabel *label = pLabeler->getNext();
	while (label != NULL) {
	  sCount++;
	  CandidateIterator cit = candidates.find(label);
	  if (cit == candidates.end()) 
		candidates[label] = 1;
	  else {
		((*cit).second)++;
		delete label;
	  }

	  label = pLabeler->getNext();
	}

//	cout << spCount << " " << sCount << endl;
	spCount++;
  }
  
  currSolution = candidates.begin();
}

DiscreteLabel *StupidGroupLabelFinder::getNext() {
  int setSize = spectra.size();

  if (currSolution == candidates.end()) 
	return NULL;

  do {
	if ((*currSolution).second >= support * setSize) {
	  DiscreteLabel *cp = (*currSolution).first;
	  DiscreteLabel *result = new DiscreteLabel(*cp);
	  currSolution++;
	  return result;
	} else 
	  currSolution++;
  } while (currSolution != candidates.end());

  return NULL;
}
	
void StupidGroupLabelFinder::clearCandidateSet() {
  for (CandidateIterator it = candidates.begin();
	   it != candidates.end(); it++) 
	delete (*it).first;
  candidates.clear();
}
