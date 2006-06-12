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

#include <SignatureFileGenerator.h>
#include <SignatureGenerator.h>
#include <SignatureDatabaseReader.h>
#include <SignatureDatabase.h>
#include <Signature.h>
#include <SymbolReader.h>
#include <Symbol.h>

#include <CoinPackedVector.hpp>

SignatureFileGenerator::SignatureFileGenerator
(const char *elementSigFileName,
 const char *symbolFileName,
 const char *outputFileName) {
 
  // read the signature database
  SignatureDatabaseReader sdr(elementSigFileName, SIG_SIMPLE_TEXT);
  SignatureDatabase *pSigDB = sdr.getSignatureDatabase();
  sdr.close();
 
  printf("SignatureFileGenerator: read signatures\n");
  
  SignatureGenerator sigGen(pSigDB);
 
  vector<Signature *> outputSig;
  outputSig.clear();
 
  printf("SignatureFileGenerator: read symbols\n");
  
  SymbolReader sr(symbolFileName);
 
  Symbol symbol;
  Symbol *result;
  result = sr.getNextSymbol(&symbol);
 
  int sigCount = 0;
  int sigSize = 0;
  while (result != NULL) {
	//	printf("One ion \n");
	symbol.dump();

	printf("before\n");
	
 	Signature *pSig = sigGen.generateSignatureFromSymbol(symbol);

	printf("after\n");
 	
 	outputSig.push_back(pSig);
 	sigCount++;
 	
 	int currSize = pSig->getMaxiumIndex();
 	if (currSize + 1 > sigSize)
	  sigSize = currSize + 1;
 		
	result = sr.getNextSymbol(&symbol);
  }
 
  //printf("close ions list reader...\n");
  
  sr.close();
 
  writeSignatures(outputFileName, outputSig, sigCount, sigSize);

  char unpackedFileName[1000];
  strcpy(unpackedFileName, outputFileName);
  strcat(unpackedFileName, ".unpacked");
  writeUnpackedSignatures(unpackedFileName, outputSig, sigCount, sigSize);
   
  // free the allocated memory
  for (vector<Signature *>::iterator it = outputSig.begin();
	   it != outputSig.end(); it++)
 	delete (*it);
  outputSig.clear();
  delete pSigDB;
}

SignatureFileGenerator::~SignatureFileGenerator() {
}

void SignatureFileGenerator::writeSignatures(const char *outputFileName,
											 const vector<Signature *> &sigSet,
											 int sigCount, int sigSize) {
  FILE *outf = fopen(outputFileName, "w");

  //  fprintf(outf, "%d %d\n", sigCount, sigSize);

  for (vector<Signature *>::const_iterator it = sigSet.begin();
	   it != sigSet.end(); it++) {

	Signature *pSig = (*it);
	
	fprintf(outf, "%s ", pSig->getName().c_str());

	const CoinPackedVector *pV = pSig->getNormalized();
	int size = pV->getNumElements();
	const int * ind = pV->getIndices();
	const double *ele = pV->getElements();

	for (int i=0; i < size; i++)
	  fprintf(outf, "%d %lf ", ind[i], ele[i]);

	fprintf(outf, "-1\n");
  }

  fclose(outf);
}

void SignatureFileGenerator::writeUnpackedSignatures(const char *outputFileName,
													 const vector<Signature *> &sigSet,
													 int sigCount, int sigSize) {
  FILE *outf = fopen(outputFileName, "w");

  double buffer[5000];
  
  // fprintf(outf, "%d %d\n", sigCount, sigSize);

  for (vector<Signature *>::const_iterator it = sigSet.begin();
	   it != sigSet.end(); it++) {

	Signature *pSig = *it;

	const CoinPackedVector *pV = pSig->getNormalized();
	int size = pV->getNumElements();
	const int * ind = pV->getIndices();
	const double *ele = pV->getElements();	

	memset(buffer, 0, sizeof(double) * sigSize);
	for (int i=0; i < size; i++) 
	  buffer[ind[i]] = ele[i];
	
	for (int i=0;  i < sigSize; i++) 
	  fprintf(outf, "%lf ", buffer[i]);

	fprintf(outf, "\n");	
  }

  fclose(outf);
}
