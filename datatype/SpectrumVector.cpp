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
#include <SpectrumVector.h>
using namespace std;

SpectrumVector::SpectrumVector(const SpectrumVector &spv) {
  size = spv.size;
  originalVector = spv.originalVector;
  normalizedVector = spv.normalizedVector;
}

SpectrumVector::SpectrumVector(int size, const int *inds, const double *elems) {

  this->size = size;
  originalVector.setVector(size, inds, elems);
  
  double sum = 0;
  for (int i=0; i < size; i++) 
	sum += elems[i];

  // Normalized the vector
  if (sum != 0) {
	double newElems[5000];
	for (int i=0; i < size; i++) 
	  newElems[i] = elems[i] / sum;
	normalizedVector.setVector(size, inds, newElems);	
  } else {
	normalizedVector.setVector(size, inds, elems);
  }
}

void SpectrumVector::dump() const {
  const int *indices = originalVector.getIndices();
  const double *elements = originalVector.getElements();
  int nElements = originalVector.getNumElements();

  cout << "Original : ";
  for (int i=0; i < nElements; i++) 
	cout << "(" << indices[i] << "," << elements[i] << ")";
  cout << endl;

  indices = normalizedVector.getIndices();
  elements = normalizedVector.getElements();

  cout << "Normalized : ";
  for (int i=0; i < nElements; i++) 
	cout << "(" << indices[i] << "," << elements[i] << ")";
  cout << endl;

}

int SpectrumVector::getMaxiumIndex() const {
	const int *indices = normalizedVector.getIndices();
	int nElements = originalVector.getNumElements();
	int maxIndex = 0;
	for (int i=0; i < nElements; i++) 
		if (indices[i] > maxIndex)
			maxIndex = indices[i];
	return maxIndex;
}

double * SpectrumVector::getUnpackedNormalized() const {
  int sz = getMaxiumIndex() + 1;

  double *data = new double[sz];
  memset(data, 0, sizeof(double) * sz);
  
  const int *indices = originalVector.getIndices();
  const double *elements = originalVector.getElements();
  int nElements = originalVector.getNumElements();

  for (int i=0; i < nElements; i++)
	data[indices[i]] = elements[i];
  
  return data;
}

double SpectrumVector::getDistance(const SpectrumVector *spc) const {

  CoinPackedVector diff = this->normalizedVector - spc->normalizedVector;

  double dis = 0;
  const double *elements = diff.getElements();
  const int *indices = diff.getIndices();
  int nElements = diff.getNumElements();

//    for (int i=0; i < nElements; i++) 
//  	cout << "(" << indices[i] << "," << elements[i] << ")";
//    cout << endl;

  for (int i=0; i < nElements; i++) 
	dis += abs(elements[i]);

  //    cout << dis << endl;

  

  return dis;
}
