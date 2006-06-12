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

#include <Spectrum.h>
#include <ManhattenDistanceLabelFinder.h>

#define MAX_VALUE 1e20

/***************************************************************************/
/* Finding quantity label with minimal error using Manhatten distance      */
/* The optimization taks can be described as following                     */
/* min e*s                                                                 */
/*   s.t. -s < Ax - b < s                                                  */
/*        xi >= 0                                                          */
/*   e = (1, 1, 1, ... )                                                   */

ManhattenDistanceLabelFinder::ManhattenDistanceLabelFinder(const SignatureDatabase &sd) : QuantityLabelFinder(sd) {
  sizeSig = sd.sizeOfSignature();
  numSigs = sd.numNonEmptySignatures();
  
  // cout << "numSigs :" << numSigs << endl;
  // cout << "matrix : " << sd.getNumCols() << endl;

  int colnum = sizeSig + numSigs;
  int rownum = sizeSig * 2;
  
  double rowlow[5000];
  double rowhigh[5000];
  double collow[5000];
  double colhigh[5000];

  //  cout << "initialize matrix." << endl;

  CoinPackedMatrix solveMatrix(sd);
  solveMatrix.bottomAppendPackedMatrix(sd);

  // cout << "col :" << solveMatrix.getNumCols() << endl;

  //  cout << "merge two original matrices." << endl;
  
  int indices[2];
  double elements[2];

  for (int i=0; i < sizeSig; i++) {
	indices[0] = i;
	indices[1] = i + sizeSig;
	elements[0] = 1.0;
	elements[1] = -1.0;
	solveMatrix.appendCol(2, indices, elements);
  }

  //  cout << solveMatrix.getNumCols() << endl;

  /*** Add contraints over the columns ***/
  // 0 <= xi <= 1, for all i=1,2,...,m
  // 0 <= sj <= 1, for all j=1,2,...,n  
  for (int i=0; i < colnum; i++) {
	collow[i] = 0;
	colhigh[i] = 1;
  }
  
  double *obj = new double[colnum];
  /*** Create the objective for optimization */
  // The optimization goal is e*s 
  for (int i=0; i < numSigs; i++)
    obj[i] = 0;
  for (int i=numSigs; i < colnum; i++)
	obj[i] = 1.0;


  model.loadProblem(solveMatrix,
					collow, colhigh,
					obj,
					rowlow, rowhigh);

  model.passInMessageHandler(&msgHandler);

  solveCount = 0;
}

ManhattenDistanceLabelFinder::~ManhattenDistanceLabelFinder() {
}

void ManhattenDistanceLabelFinder::setSpectrum(const Spectrum *spc) {
  /* Initialize the value of row bound */
  CoinPackedMatrix *matrix = model.matrix();
  const CoinPackedVector *spv = spc->getNormalized();
  int numRow = matrix->getNumRows();
  //int sizeSigs = numRow / 2;

  double *rowlow = model.rowLower();
  double *rowhigh = model.rowUpper();

  for (int i=0; i < sizeSig; i++) {
	rowlow[i]  = 0;
	rowhigh[i] = 2;

	rowlow[i + sizeSig] = -2;
	rowhigh[i + sizeSig] = 0;
  }

  const int *indices = spv->getIndices();
  const double *elem = spv->getElements();
  int vecsize = spv->getNumElements();
  
  for (int i=0; i < vecsize; i++) 
	if (indices[i] <= sizeSig) {
	  rowlow[indices[i]] = elem[i];
	  rowhigh[indices[i] + sizeSig] = elem[i];
	} else {
	  // cerr << "ERROR : Index " << indices[i] << " exceeds the maxium signature size " << sizeSig << endl;
	  // exit(0);
	}
}


QuantityLabel *ManhattenDistanceLabelFinder::solve() {

  solveCount++;

  //  dump();

  model.dual();

  const double * solution = model.getColSolution();

  CoinPackedMatrix *matrix = model.matrix();
  int numRows = matrix->getNumRows();
  int numSigs = matrix->getNumCols() - numRows / 2;

  /*  const double * rowSol = model.getRowActivity();
  for (int i=0; i < matrix->getNumRows() / 2; i++) 
	if (rowSol[i] != 0) 
	  cout << "(" << i << "," << rowSol[i] << ")";
  cout << endl;
 
  for (int i=0; i < matrix->getNumRows() / 2; i++) 
	if (rowSol[i + numRows / 2] != 0) 
	  cout << "(" << i << "," << rowSol[i + numRows / 2] << ")";
	  cout << endl;*/

  // The first sizeSig element in the solution column 
  // correpsonds to the quantity of signatures.
  QuantityLabel *pLabel =  
	new QuantityLabel(model.objectiveValue(),
					  numSigs, solution);

  /*  cout << model.objectiveValue() << endl;
  for (int i=0; i < matrix->getNumCols(); i++) 
	cout << solution[i] << ",";
	cout << endl;*/

  return pLabel;
}

QuantityLabel *ManhattenDistanceLabelFinder::solve(const Spectrum *spc) {
  setSpectrum(spc);
  return solve();
}

/****************************************************************/
/* Apply contraints over the quantity of a specific m/z value.  */
/* This method is used during the search process                */
/* Parameter :                                                  */
/*     indx - the index of the m/z value                        */
/*     low  - new lower bound                                   */
/*     high - new upper bound                                   */
/****************************************************************/

void ManhattenDistanceLabelFinder::setRange(int indx, double low, double high) {
  double *collow = model.columnLower();
  double *colhigh = model.columnUpper();
  
  collow[indx] = low;
  colhigh[indx] = high;
}

void ManhattenDistanceLabelFinder::dump() {
  CoinPackedMatrix *pMatrix = model.matrix();
  int numCols = pMatrix->getNumCols();
  int numRows = pMatrix->getNumRows();
  
  double *collow = model.columnLower();
  double *colhigh = model.columnUpper();

  double *rowlow = model.rowLower();
  double *rowhigh = model.rowUpper();

  double *obj = model.objective();

  pMatrix->dumpMatrix();

  cout << "Column constraints" << endl;

  cout << "(" << collow[0];
  for (int i=1; i < numCols; i++) 
	cout << "," << collow[i];
  cout << ")" << endl;

  cout << "(" << colhigh[0];
  for (int i=1; i < numCols; i++) 
	cout << "," << colhigh[i];
  cout << ")" << endl;

  cout << "Row constraints" << endl;

  cout << "(" << rowlow[0];
  for (int i=1; i < numRows; i++) 
	cout << "," << rowlow[i];
  cout << ")" << endl;

  cout << "(" << rowhigh[0];
  for (int i=1; i < numRows; i++) 
	cout << "," << rowhigh[i];
  cout << ")" << endl;

  
  cout << "Objective " << endl;
   
  cout << "(" << obj[0];
  for (int i=1; i < numCols; i++) 
	cout << "," << obj[i];
  cout << ")" << endl;
}
