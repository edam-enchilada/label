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
#include <SignatureDatabaseWriter.h>
#include <Spectrum.h>

#include <string.h>
#include <iostream>
using namespace std;

void SignatureDatabaseWriter::writeString(char *buf) {
	fprintf(file, "%s", buf);
}

SignatureDatabaseWriter::SignatureDatabaseWriter(const char *filename, SigFileType mode) {
  file = NULL;
  fn = NULL;
  sigCount = 0;

  fn = (char *)malloc(strlen(filename) + 1);
  strcpy(fn, filename);
  this->mode = mode;

  switch (mode) {
  case SIG_SIMPLE_TEXT :
	file = fopen(filename, "w");	
	break;
	
  case SIG_WEKA_TEXT :
  	file = fopen(filename, "w");
	fprintf(file, "%s\n", "%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	fprintf(file, "%s\n", "% Format for signature file\n");
	//fprintf(file, "%s\n", "In comment lines, specify\n");
	//fprintf(file, "%s\n", "% (1) dimension and  (2) discretization vector\n");
	fprintf(file, "%s\n\n", "%%%%%%%%%%%%%%%%%%%%%%%%%%");
	fprintf(file, "@relation signature\n");
	fprintf(file, "\n");
	//fprintf(file, "attribute sigID real\n");
	for (int i=1; i <= 255; i++) 
		fprintf(file, "@attribute mz%d real\n", i);	
	fprintf(file, "@attribute tag {Low, Mid, High}\n");
	//fprintf(file, "% This tag field is reserved for ML algorithms");
	fprintf(file, "\n@data\n");
	break;
	
  default:
	cerr << "Unkown file mode" << endl;
  }
}

SignatureDatabaseWriter::~SignatureDatabaseWriter() {
  if (file != NULL)
	fclose(file);

  if (fn != NULL)
	free(fn);
}

int SignatureDatabaseWriter::close() {
  if (file == NULL) 
	return -1;
  else {
	fclose(file);
	file = NULL;
	return 0;
  }
}

void SignatureDatabaseWriter::writeSignatureDatabase(SignatureDatabase *sd) {
  int ns = sd->numSignatures();
  
  /*  if (this->mode == SIG_SIMPLE_TEXT) 
  	fprintf(file, "%d %d\n", ns, 255);
  */
	
  
  for (int i=0; i < ns; i++) {
  	const Signature *sg = sd->getSignatureByIndex(i);
  	const CoinPackedVector *v = sg->getNormalized();
  	const int *ind = v->getIndices();
  	const double *elem = v->getElements();
  	int numElems = v->getNumElements();
  
  	switch (this->mode) {
  		case SIG_SIMPLE_TEXT :
 	  		fprintf(file, "%s\n", sg->getName().c_str());
  	  		for (int i=0; i < numElems; i++) 
	  		fprintf(file, "%d %lf ", ind[i], elem[i]);
          		fprintf(file, "-1\n\n");
          	break;
          
        	case SIG_WEKA_TEXT:
			//cout <<  << endl;
          		// print the id of the signature
          		//sigCount++;
          		//fprintf(file, "%d", i+1);
          		//cout << sigCount << endl;
          		
          		double values[256];
  	  		for (int i=0; i < 256; i++) values[i] = 0;
  
  	  		for (int i=0; i < numElems; i++) 
  	  			if ((ind[i] > 0) && (ind[i] < 256))
  					values[ind[i]] = elem[i]; 
	  		for (int i=1; i <= 255; i++) 
  				fprintf(file, "%lf,", values[i]);
  			fprintf(file, "High\n");
  	  		break;
  	}
  }
}
