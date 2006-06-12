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

#ifndef BRANCH_AND_BOUND_LABEL_FINDER_H
#define BRANCH_AND_BOUND_LABEL_FINDER_H

#include <DiscreteLabel.h>
#include <DiscreteLabelFinder.h>
#include <SearchSpace.h>


class BranchAndBoundLabelFinder : public DiscreteLabelFinder {
  public :
	BranchAndBoundLabelFinder(const SignatureDatabase *sd,
							  const Tile *tile = NULL,
							  const Spectrum *spc = NULL,
							  double err = 0) 
	: DiscreteLabelFinder(sd, tile, spc, err) {
	searchStack = NULL;
	searchSpace.low = NULL;
	searchSpace.high = NULL;
  };

	~BranchAndBoundLabelFinder();
	virtual void init();
    virtual DiscreteLabel *getNext();

	void clearCompensateCount() { compensateCounter = 0; }
	void setCompensateCount(int c) { compensateCounter = c; }
	long getCompensateCount() { return compensateCounter; }
	
   
  private :	
	// count those LP calls bypassed because of the optimization
	long compensateCounter;

	SearchState *searchStack;
    SearchSpace searchSpace;
	int currStackPosition;
	bool firstTime;

	enum BranchAndBoundSearchState {
	  STATE_TEST,
	  STATE_CUT,
	  STATE_ITERATE,
	  STATE_TRACEBACK,
	  STATE_DONE
	} stage;

	int findCutting(int currLevel);
	// Find the best way to split current search space
	
};

#endif
