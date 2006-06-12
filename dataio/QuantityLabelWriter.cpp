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

#include <CoinError.hpp>

#include <Signature.h>
#include <QuantityLabelWriter.h>

#include <string.h>
#include <iostream>
using namespace std;

void QuantityLabelWriter::writeString(char *buf) {
	fprintf(file, "%s", buf);
}

QuantityLabelWriter::QuantityLabelWriter(const char *filename, 
					 int nSigs,
					 LabelFileType mode) {
  file = NULL;
  fn = NULL;
  numSigs = nSigs;

  fn = (char *)malloc(strlen(filename) + 1);
  strcpy(fn, filename);
  this->mode = mode;

  switch (mode) {
  case LBL_SIMPLE_TEXT :
  case LBL_CLEAR_TEXT :
	file = fopen(filename, "w");
	break;
	
  case LBL_WEKA_TEXT :
  	file = fopen(filename, "w");
	fprintf(file, "%s\n", "%%%%%%%%%%%%%%%%%%%%%%%%%%");
	fprintf(file, "%s\n", "% Format for label file");
	fprintf(file, "%s\n", "% In comment lines, specify");
	fprintf(file, "%s\n", "% (1) discretization vector and  (2) # of signatures");
	fprintf(file, "%s\n\n", "%%%%%%%%%%%%%%%%%%%%%%%%%%");

	fprintf(file,  "%s\n", "%note that instead of using nominal represetation for the label");
	fprintf(file,  "%s\n", "%I choose to preserve the original real value which is the quantity");
	fprintf(file, "%s\n", "%label in a particular solution cell");

	break;
	
  default:
	cerr << "Unkown file mode" << endl;
  }
}

void QuantityLabelWriter::writeMeta() {
	fprintf(file, "@relation label\n\n");
	
	fprintf(file, "@attribute specID real\n");
	for (int i=1; i <= numSigs; i++) 
		fprintf(file, "@attribute sigValue%d real\n", i);	
	
	fprintf(file, "\n\n@data\n");
}

QuantityLabelWriter::~QuantityLabelWriter() {
  if (file != NULL)
	fclose(file);

  if (fn != NULL)
	free(fn);
}

int QuantityLabelWriter::close() {
  if (file == NULL) 
	return -1;
  else {
	fclose(file);
	file = NULL;
	return 0;
  }
}

void QuantityLabelWriter::writeLabel(int id, double *lbl) {
  	// fprintf(file, "%d", id);
  	
  	fprintf(file, "%.6lf", lbl[0]);
  	for (int i=1; i < numSigs; i++) 
  		fprintf(file, ",%.6lf",lbl[i]);
	fprintf(file, "\n");
}

void QuantityLabelWriter::writeLabel(int id, double *lbl, SignatureDatabase *psd) {
  
  switch (mode) {
  case LBL_CLEAR_TEXT:
	fprintf(file, "%d\n", id);
	
	int order[10000];
	for (int i=0; i< numSigs; i++) order[i] = i;
	
	for (int i=0; i < numSigs; i++) {
	  for (int j=i+1; j < numSigs; j++) {
		if (lbl[order[i]] < lbl[order[j]]) {
		  int temp = order[i];
		  order[i] = order[j];
		  order[j] = temp;
		}
	  }
	}
	
	for (int i=0; i < numSigs; i++) {
	  int sg = order[i];
	  const Signature *ps = psd->getSignatureByIndex(sg);
	  
	  if (lbl[sg] > 1e-10) 
		fprintf(file, "%s %.6lf ", ps->getName().c_str(), lbl[sg]);
	}
	
	fprintf(file, "\n");
	break;

  case LBL_WEKA_TEXT :
	fprintf(file, "%d", id);
	for (int i=0; i < numSigs; i++) 
	  fprintf(file, ",%.6lf", lbl[i]);
	fprintf(file, "\n");
	break;

  default :
	break;
  }
}
