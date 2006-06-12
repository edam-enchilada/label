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

#include <SignatureDatabaseReader.h>
#include <string.h>
#include <iostream>
#include <assert.h>

using namespace std;

SignatureDatabaseReader::SignatureDatabaseReader(const char *filename, SigFileType filemode) {

  file = NULL;
  fn = NULL;
  
  fn = (char *)malloc(strlen(filename) + 1);
  strcpy(fn, filename);
  
  mode = filemode;
  
  switch (mode) {
  case SIG_SIMPLE_TEXT:
	file = fopen(filename, "r");
	break;
	
  default:
	cerr << "Unknown file mode" << endl;
  }
}

SignatureDatabaseReader::~SignatureDatabaseReader() {
  if (file != NULL) 
	fclose(file);

  if (fn != NULL)
	free(fn);
}
  
SignatureDatabase *SignatureDatabaseReader::getSignatureDatabase() {
  switch (mode) {
  case SIG_SIMPLE_TEXT :
	return readFromSimpleTextFile();
	break;
  default:
	break;
  }

  return NULL;
} 

SignatureDatabase *SignatureDatabaseReader::readFromSimpleTextFile() {

  SignatureDatabase *sd = new SignatureDatabase(1000);

  int indices[5000];
  double elements[5000];

  int indx;
  int flag;
  double val;
  char buffer[2000];
	
  flag = fscanf(file, "%s ", buffer);

  while (flag > 0) {
	printf("%s %d\n", buffer, flag);
	
	int vecsize = 0;

	// read a vector from one line
	fscanf(file, "%d", &indx);
	while (indx != -1) {
	  fscanf(file, "%lf", &val);
	  indices[vecsize] = indx;
	  elements[vecsize] = val;
	  vecsize++;
	  fscanf(file, "%d", &indx);
	}


	if (vecsize != 0) {
	  // add the new signature into the database
	  try {
		Signature sig(vecsize, indices, elements);
		sig.setName(buffer);
		sd->appendSignature(sig);
	  } catch (CoinError &error) {
		cerr << error.message() << endl;
		return NULL;
	  }
	}
	

	flag = fscanf(file, "%s ", buffer);
  }

  //  cout << "cols after read (signatureDBReader) : " << sd->getNumCols() << endl;

  /*  // Set the dimension of the signature database
	  try {
	  sd->setDimensions(sizeSigs, numSigs);
	  } catch (CoinError &error) {
	  cerr << error.message() << endl;
	  return NULL;
	  }*/

  return sd;
}

int SignatureDatabaseReader::close() {
  if (file == NULL) 
	return -1;
  else {
	fclose(file);
	file = NULL;
	return 0;
  }
}
	
