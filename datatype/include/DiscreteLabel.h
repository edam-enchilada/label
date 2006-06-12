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

#ifndef DISCRETE_LABEL_H
#define DISCRETE_LABEL_H

#include <stdlib.h>
#include <QuantityLabel.h>

class DiscreteLabel {
  public :
	DiscreteLabel(int sz, const int *data = NULL, 
				  const QuantityLabel *label = NULL,
				  double error=-1);
  DiscreteLabel(const DiscreteLabel &l);

  ~DiscreteLabel();
  int *getData() { return data; }
  int getSize() { return size; }
  void setError(double error) { this->error = error; }
  double getError() { return this->error; }

  void setQuantityLabel(const QuantityLabel &label) {
	if (qLabel != NULL)
	  delete qLabel;
	qLabel = new QuantityLabel(label);
  }
  const QuantityLabel *getQuantityLabel() { return qLabel; }


  struct lt {
	bool operator()(const DiscreteLabel *l1, const DiscreteLabel *l2) const {
	  for (int i=0; i < l1->size; i++) 
		if (l1->data[i] < l2->data[i])
		  return true;
		else
		  if (l1->data[i] > l2->data[i])
			return false;
	  return false;
	}
  };

  void dump() const;

  private :
	int *data;
  int size;
  double error;
  QuantityLabel *qLabel;
};

#endif
