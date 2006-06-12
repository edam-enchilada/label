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

#ifndef DISCRETE_GROUP_LABEL_FINDER
#define DISCRETE_GROUP_LABEL_FINDER

#include <ManhattenDistanceLabelFinder.h>
#include <SignatureDatabase.h>
#include <DiscreteLabel.h>
#include <Spectrum.h>
#include <Tile.h>

#include <vector>

class DiscreteGroupLabelFinder {
 public :
  DiscreteGroupLabelFinder(const SignatureDatabase *sd,
						   const Tile *tile = NULL,
						   double err = 0,
						   double support = 1);
  virtual ~DiscreteGroupLabelFinder();
  virtual void init() {} ;
  virtual DiscreteLabel *getNext() { return NULL; };

  void addSpectrum(const Spectrum *spc);

  void setTile(const Tile *tile) { pTile = tile; }
  const Tile *getTile() { return pTile; }

  void setError(const double err) { error = err; }
  const double getError() { return error; }

  void setSupport(const double spt) { support = spt; }
  const double getSupport() { return support; }

  virtual int getSolveCount() { return 0;}
  virtual void clearSolveCount() {}

  virtual long getCompensateCount() { return -1; }
  virtual void setCompensateCount(long c) { }
  virtual void clearCompensateCount() {};

 protected :

  void clearSpectraSet();

  vector<Spectrum *> spectra;
  const Tile *pTile;
  const SignatureDatabase *pSigDB;
  
  // the error bound allowed
  double error;
 
  // the minimal support for a label or label pattern in the 
  // group of spectra
  double support;
};

#endif
