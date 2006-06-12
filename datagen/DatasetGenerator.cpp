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

#include <DatasetGenerator.h>
#include <SignatureDatabaseReader.h>
#include <SignatureDatabaseWriter.h>
#include <SignatureDatabase.h>
#include <SpectrumReader.h>
#include <SpectrumWriter.h>
#include <Signature.h>
#include <Spectrum.h>
#include <DiscreteLabel.h>
#include <DiscreteLabelWriter.h>
#include <QuantityLabelWriter.h>

#include <stdio.h>
#include <iostream>

// normal distribution with mean m and standard deviation s 
double DatasetGenerator::normal(double m, double s) {
  double x1, x2, w;
  do {
    x1 = 2.0 * (double(rand() / double(RAND_MAX))) - 1.0;
    x2 = 2.0 * (double(rand() / double(RAND_MAX))) - 1.0;
    w = x1*x1 + x2*x2;
  } while (w >= 1. || w < 1E-30);
  w = sqrt((-2.0 * log(w)) / w);
  x1 *= w;
  return x1 * s + m;
} 

void DatasetGenerator::generateMoreSignatures(const char *sigFileName,
					      const char *unknownSigFileName,
					      const char *newSigFileName,
					      int newNumSigs) {
	SignatureDatabaseReader *sdr = new SignatureDatabaseReader(sigFileName, SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sdr->getSignatureDatabase();
	int numSigs = sd->numSignatures();
	
	SignatureDatabase osd(1000);
	for (int i=0; i < numSigs; i++) {
		Signature *p = new Signature(*(sd->getSignatureByIndex(i)));
		osd.appendSignature(*p);
		delete p;
	}
		
	SignatureDatabaseReader *usdr = new SignatureDatabaseReader(unknownSigFileName, SIG_SIMPLE_TEXT);
	SignatureDatabase *usd = usdr->getSignatureDatabase();
	nuSig = usd->numSignatures();
	
	int ind1[] = {251};
	double ele1[] = {1};
	Signature *p = new Signature(1, ind1, ele1);
	
	int ind2[] = {252};
	double ele2[] = {1};
	Signature *q = new Signature(1, ind2, ele2);
	
	for (int i=0; i < newNumSigs - numSigs; i++) {
		
		const Signature *pp; 
		const Signature *qq; 
		
		if (rand() % 8 > 1) {
			pp = p;
			qq = q;
		} else {
			pp = usd->getSignatureByIndex(rand() % nuSig);
			qq = usd->getSignatureByIndex(rand() % nuSig);
		}
		
		const CoinPackedVector *cp = pp->getNormalized();
		const CoinPackedVector *cq = qq->getNormalized();
		
		double ratio = (double)rand() / (double)RAND_MAX;
		CoinPackedVector temp = ratio * (*cp)  + (1-ratio) * (*cq);
		
		Signature *sig = new Signature(temp.getNumElements(), temp.getIndices(), temp.getElements());
		
		//sig->dump();
		
		char buf[1000];
		sprintf(buf, "filler_signature_%d", i);
		sig->setName(buf);
		osd.appendSignature(*sig);
		
		delete sig;
	//	delete p;
	//	delete q;
	}
	
	delete p;
	delete q;
	
	cout << "write signature database..." << endl;
	
	// Write the signature database in simple format
	SignatureDatabaseWriter sdw0(newSigFileName, SIG_SIMPLE_TEXT);
	sdw0.writeSignatureDatabase(&osd);
	
	// Write the known signatures in the WEKA format
	char buf[1000];
	sprintf(buf, "%s.weka", newSigFileName);
	SignatureDatabaseWriter sdw0w(buf, SIG_WEKA_TEXT);
	sdw0w.writeSignatureDatabase(&osd);
	
	//sdr->close();
	//delete sd;
	//delete usd;
}
	
void DatasetGenerator::generateSignature(const char *elementSignatureFileName,  // filename of the element signature 
				         int numUnknownSignatures, // number of unknown Signatures in the signature database
				         int numGroups, // number of groups, within each group there are interference
				         const char *outputSignatureFileName, // signature filename, the filename for weka format has extra .weka extension
				         const char *outputUnknownSignatureFileName // unknown signature filename
				       ) {
	eleFn = elementSignatureFileName;
	nuSig = numUnknownSignatures;
	oSigFn = outputSignatureFileName;
	ouSigFn = outputUnknownSignatureFileName;
	nGroups = numGroups;
	
	cout << "Reading signature database" << endl;
	SignatureDatabaseReader sdr(eleFn, SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sdr.getSignatureDatabase();
	
	cout << "Finish reading Signature database" << endl;
	SignatureDatabase osd(1000);
	SignatureDatabase usd(1000);
	
	// Generate the signature group
	for (int i=0; i < nGroups; i++) {
		cout << "generate group " << i << " " << endl;
		
		sigGroup[i] = new Signature *[i + 2];
		//if (sd->getSignatureByIndex(i * 2 + 1) == 0) 
		//	cout << "Error" << endl;
			
		sigGroup[i][0] = new Signature(*(sd->getSignatureByIndex(i * 2)));
		sigGroup[i][1] = new Signature(*(sd->getSignatureByIndex(i * 2 + 1)));
		
		cout << "signatures retrieved." << endl;
		
		const CoinPackedVector *p1 = sigGroup[i][0]->getNormalized();
		const CoinPackedVector *p2 = sigGroup[i][1]->getNormalized();
		
		for (int j=1; j < i+2; j++) {
			double p = (double)rand() / (double)RAND_MAX;
			CoinPackedVector temp = (*p1) * p + (*p2) * (1-p);
			
			if (j== 1) {
				// Generate a signature which will interference with other
				// signatures in that group
				Signature *sig = new Signature(temp.getNumElements(),
							       temp.getIndices(), temp.getElements());
				char buf[1000];
				sprintf(buf, "unknown_sig_in_group_%d", i);
				sig->setName(buf);
				usd.appendSignature(*sig);
				delete sig;
			} else {
				sigGroup[i][j] = new Signature(temp.getNumElements(),
						       temp.getIndices(),
						       temp.getElements());
				char buf[1000];
				sprintf(buf, "group_%d_%s_%s_%d", i, 
					sigGroup[i][0]->getName().c_str(),
					sigGroup[i][1]->getName().c_str(), j);
				string name(buf);
				sigGroup[i][j]->setName(name);
			}
		}
		
		//add the signature into the signature database
		for (int j=0; j < i + 2; j++) {
			osd.appendSignature(*(sigGroup[i][j]));
		}
	}
	
	cout << "Groups generated." << endl;
	
	// total is the number of free singatures and unknown signatures
	int total = sd->numSignatures() - nGroups * 2;
	cout << "Total signatures = " << total << endl;

	if (total > nuSig) {
		// Select those unknown signatures
		int sel[10000];
		for (int i=0; i < total; i++)
			sel[i] = 1;
		for (int i=0; i < nuSig; i++) {
			int p;
			do {
				p = rand() % total;
			} while (sel[p] == 0);
			sel[p] = 0;
		}
		
		//freeGroup = new (Signature*)[total - nuSig];
		//unknownGroup = new (Signature *)[nuSig];
		//freeC = 0;
		//unknownC = 0;
		cout << "Unknown signatures selected" << endl;
		
		for (int i=0; i < total; i++) {
			if (sel[i] == 1) {
				Signature *newSig = new Signature(*(sd->getSignatureByIndex(i + nGroups * 2)));
				osd.appendSignature(*newSig);
				delete newSig;
				// cout << i << "xxx" << endl;
				//freeGroup[freeC++] = newSig;
			} else {
				usd.appendSignature(*(sd->getSignatureByIndex(i + nGroups * 2)));
				//unknownGroup[unknownC++] = new Signature(*(sd->getSignatureByIndex(i+40)));
			}
		}
		
		cout << "Independant signatures generated." << endl;
	}
	
	// Write the signature database in simple format
	SignatureDatabaseWriter sdw0(oSigFn, SIG_SIMPLE_TEXT);
	sdw0.writeSignatureDatabase(&osd);
	
	// Write the known signatures in the WEKA format
	char buf[1000];
	sprintf(buf, "%s.weka", oSigFn);
	SignatureDatabaseWriter sdw0w(buf, SIG_WEKA_TEXT);
	sdw0w.writeSignatureDatabase(&osd);
	
	// Write the unknown signatures in the simple format
	SignatureDatabaseWriter sdw1(ouSigFn, SIG_SIMPLE_TEXT);
	sdw1.writeSignatureDatabase(&usd);
	
	// Write the unknown signatures in the WEKA format
	sprintf(buf, "%s.weka", ouSigFn);
	SignatureDatabaseWriter sdw1w(buf, SIG_WEKA_TEXT);
	sprintf(buf, "%s  The first %d signatures are unknown signatures which\n", "%", nGroups);
	sdw1w.writeString("buf");
	sdw1w.writeString("% interference with other signatures in each group.\n");
	sdw1w.writeString("% The others are independent unknown signatures which\n");
	sdw1w.writeString("% do not interference with any other signatures in the\n");
	sdw1w.writeString("% signature database.\n");
	sdw1w.writeSignatureDatabase(&usd);
	// delete sd;
	
	for (int i=0; i < nGroups; i++) {
		for (int j = 0; j < i + 2; j++)  {
			delete sigGroup[i][j];
			sigGroup[i][j] = NULL;
		}
		delete sigGroup[i];
	}
	cout << "signature file has been written." << endl;
}

void DatasetGenerator::generateSpectra(
				 const char *signatureFileName,
				 const char *unknownSignatureFileName,
				 int numSpectra,
				 int numSigPerSpectrum,
				 int numUnknownSigPerSpectrum,
				 int numInfPerSpectrum,
				 int numDimWithNoise,
				 int useInterUnknownSig,
				 int numGroups,
				 double intensityOfNoise,
				 double varianceOfNoise,
				 const double *signatureWeight,
				 const double *signatureWeightVariance,
				 int tileSize,
				 const double *tile,
				 const char *outputSpectrumFileName,
				 const char *outputResultFileName,
				 const char *outputLabelFileName) {
	
	nSpc = numSpectra;
	nSigS = numSigPerSpectrum;
	nuSigS = numUnknownSigPerSpectrum;
	nInfS = numInfPerSpectrum;
	nDim = numDimWithNoise;
	iNoise = intensityOfNoise;
	vNoise = varianceOfNoise;
	weight = signatureWeight;
	var = signatureWeightVariance;
	nGroups = numGroups;
	
	oResFn = outputResultFileName;
	oSpcFn = outputSpectrumFileName;
	
	
	int numSig;
	
	// read signatures
	cout << "reading normal signatures..." << endl;
	SignatureDatabaseReader *sdr = new SignatureDatabaseReader(signatureFileName, SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sdr->getSignatureDatabase();
	numSig = sd->numSignatures();
	int readCount = 0;
	for (int i=0; i < nGroups; i++) {
		sigGroup[i] = new Signature *[i + 2];
		for (int j=0; j < i + 2; j++) {
			//cout << i << "-" << j << endl;
			sigGroup[i][j] = new Signature(*(sd->getSignatureByIndex(readCount)));
			readCount++;
		}
	}
	
	cout << "read free signatures..." << endl;
	freeC = sd->numSignatures() - readCount;
	freeGroup = new Signature *[freeC];
	for (int i=0; i < freeC; i++) 
		freeGroup[i] = new Signature(*(sd->getSignatureByIndex(readCount + i)));
	delete sd;
	delete sdr;
	
	cout << "reading unknown signatures..." << endl;
	sdr = new SignatureDatabaseReader(unknownSignatureFileName, SIG_SIMPLE_TEXT);
	sd = sdr->getSignatureDatabase();
	nuSig = sd->numSignatures();
	unknownGroup = new Signature *[nuSig];
	for (int i=0; i < nuSig; i++) 
		unknownGroup[i] = new Signature(*(sd->getSignatureByIndex(i)));
	delete sd;
	delete sdr;
	
	Signature * interSigS[2];
	Signature * unknownSigS[10000];
	Signature * fillSigS[10000];
	Signature * allSigS[10000];
	int * sigGroupID[20];
	int sigFillID[10000];
	int sigIDCount = 0;
	int sigId[10000];
	//int uSigID[nuSigS];
	
	int gp = 0;	
	int diff = nInfS;
	for (int i=1;  i < nGroups; i++) {
		int cdiff = abs(nInfS - (i + 2) * (i + 1) / 2 + 1);
		if (cdiff < diff) {
			diff = cdiff;
			gp = i;
		}
	}
	
	// Assign the id to each signatures
	for (int i=0; i < nGroups; i++) {
		sigGroupID[i] = new int[i + 2]; 
		for (int j=0; j < i + 2; j++) 
			sigGroupID[i][j] = sigIDCount++;
	}
	for (int i=0; i < freeC; i++) {
		sigFillID[i] = sigIDCount++;
		//cout << sigFillID[i] << " ";
	}
	//cout << "((((" << endl;
		
	SpectrumWriter spw(oSpcFn, SPC_SIMPLE_TEXT);
	char buf[1000];
	sprintf(buf, "%s.weka", oSpcFn);
	SpectrumWriter spw0(buf, SPC_WEKA_TEXT);
	
	// Print the meta data of the result file
	FILE *file = fopen(oResFn, "w");
	fprintf(file, "%s This file contains the id of signatures for the given spectrum\n", "%");
	fprintf(file, "%s The positive number is the id of known signature, starting from 0\n", "%");
	fprintf(file, "%s The negative number is the id of unknown signature, starting from -1\n", "%");
	fprintf(file, "%s #sigs per spectrum = %d\n", "%", nSigS);
	fprintf(file, "%s #unknown signatures per spectrum = %d\n", "%", nuSigS);
	fprintf(file, "%s degree of interference = %d\n", "%", nInfS);
	fprintf(file, "%s #dimensions with noise = %d\n", "%", nDim);
	fprintf(file, "%s average intensity of the noise = %.3lf\n", "%", iNoise);
	fprintf(file, "%s variance of intensity of the noise = %.5lf\n", "%", vNoise);
	fprintf(file, "% weight vector = \n");
	fprintf(file, "%s", "%");
	for (int i=0; i < nSigS; i++) {
		fprintf(file, " %.3lf", weight[i]);
	}
	fprintf(file, "\n");
	
	
	fprintf(file, "% weight variance vector = \n");
	fprintf(file, "%s", "%");
	for (int i=0; i < nSigS; i++) {
		fprintf(file, " %.3lf", var[i]);
	}
	fprintf(file, "\n");
	
	fprintf(file, "%s the number of the discretization = %d\n", "%", tileSize);
	fprintf(file, "%s", "% ");
	for (int i=0; i < tileSize; i++)  {
		fprintf(file, "%.3lf ", tile[i]);
	}
	fprintf(file, "\n\n");

	fprintf(file, "@relation result\n");
	for (int i=0; i < nSigS; i++) 
		fprintf(file, "@attribute sig%d real\n", i);
	//	fprintf(file, "\n\n");
	fprintf(file, "%s\n", "@data");
	
	QuantityLabelWriter lw(outputLabelFileName, numSig, LBL_WEKA_TEXT);
	sprintf(buf, "%s #sigs per spectrum = %d\n", "%", nSigS);
	lw.writeString(buf);
	sprintf(buf, "%s #unknown signatures per spectrum = %d\n", "%", nuSigS);
	lw.writeString(buf);
	sprintf(buf, "%s degree of interference = %d\n", "%", nInfS);
	lw.writeString(buf);
	sprintf(buf, "%s #dimensions with noise = %d\n", "%", nDim);
	lw.writeString(buf);
	sprintf(buf, "%s average intensity of the noise = %.3lf\n", "%", iNoise);
	lw.writeString(buf);
	sprintf(buf, "%s variance of intensity of the noise = %.5lf\n", "%", vNoise);
	lw.writeString(buf);
	lw.writeString("% weight vector = \n");
	lw.writeString("% ");
	for (int i=0; i < nSigS; i++) {
		sprintf(buf, " %.3lf", weight[i]);
		lw.writeString(buf);
	}
	lw.writeString("\n");
	
	lw.writeString("% weight variance vector = \n");
	lw.writeString("% ");
	for (int i=0; i < nSigS; i++) {
		sprintf(buf, " %.3lf", var[i]);
		lw.writeString(buf);
	}
	lw.writeString("\n");
	
	sprintf(buf, "%s the number of the discretization = %d\n", "%", tileSize);
	lw.writeString(buf);
	lw.writeString("% ");
	for (int i=0; i < tileSize; i++)  {
		sprintf(buf, "%.3lf ", tile[i]);
		lw.writeString(buf);
	}
	lw.writeString("\n");
	lw.writeMeta();
	
	for (int ts = 0; ts < nSpc; ts++) {
		int p = abs(int(rand()) % (gp + 2));
		int q;
		do {
			q = abs(int(rand()) % (gp + 2));
		} while (p == q);
	
		// pick the interference signatures
		interSigS[0] = sigGroup[gp][p];
		sigId[0] = sigGroupID[gp][p];
		interSigS[1] = sigGroup[gp][q];
		sigId[1] = sigGroupID[gp][q];
			
		// pick the free signatures
		int sef[10000];
		for (int i=0; i < freeC; i++) sef[i] = 0;
		for (int i=0; i < nSigS - 2 - nuSigS; i++) {
			int t;
			do {
				t = rand() % freeC;
			} while (sef[t] == 1);
			sef[t] = 1;
			fillSigS[i] = freeGroup[t];
			sigId[i + 2] = sigFillID[t];
		}
		
		// pick the unknown signatures
		int sel[10000];
		for (int i=0; i < nuSig; i++) sel[i] = 0;
		if ((nuSigS == 1) && (useInterUnknownSig == 1)) {
			sel[gp] = 1;
			unknownSigS[0] = unknownGroup[gp];
			sigId[nSigS - 1] = - gp - 1;
		} else {
			for (int i=0; i < nuSigS; i++) {
				int t;
				do {
					t = rand() % (nuSig - nGroups) + nGroups;
				} while (sel[t] == 1);
				cout << t << "---" << endl;
				sel[t] = 1;
				unknownSigS[i] = unknownGroup[t];
				sigId[nSigS - nuSigS + i] = - t - 1;
			}
		}
		
		allSigS[0] = interSigS[0];
		allSigS[1] = interSigS[1];
		for (int i=0; i < nSigS - 2 - nuSigS; i++) 
			allSigS[i + 2] = fillSigS[i];
		for (int i=0; i < nuSigS; i++) 
			allSigS[nSigS - nuSigS + i] = unknownSigS[i];
		
		//int lbl[numSig];
		//for (int i=0; i < numSig; i++) lbl[i] = 0;
		double ql[10000];
		for (int i=0; i < numSig; i++) ql[i] = 0;
		
		// generate the weight vector for the new spectrum
		double w[10000];
		double ss = 0;
	  	for (int j=0; j < nSigS; j++) {
	  		if (j < nSigS - nuSigS) 
				w[j] = normal(weight[j], var[j]);
			else
				w[j] = weight[j];
			if (w[j] < 0) 
				w[j] = 0;
			//cout << w[j] << " ";
			ss += w[j];
	 	 }
	  	for (int j=0; j < nSigS; j++) {
			w[j] /= ss;
			
	  	}
	  	//cout << endl;
	  	
		for (int i=0; i < nSigS - nuSigS; i++) {
			ql[sigId[i]] = w[i];
			/*int v = 0;
			while (weight[i] > tile[v + 1])
				v++;
			lbl[sigId[i]] = v;*/
		}
		
		//DiscreteLabel label(numSig, lbl, NULL, 0);
		lw.writeLabel(ts, ql);
		
		// randomize the order of the signature
		//for (int i=0; i < nSigS * 2; i++) {
		//	int p = rand() % (nSigS - 2) + 2;
		//	int q = rand() % (nSigS - 2) + 2;
		//	if (p != q) {
		//		//cout << p << " --- " << q << endl;
		//		Signature * temp = allSigS[p];
		//		allSigS[p] = allSigS[q];
		//		allSigS[q] = temp;
		//		int sigTemp = sigId[p];
		//		sigId[p] = sigId[q];
		//		sigId[q] = sigTemp;
		//	}
		//}
		//for (int i=0; i < nSigS; i++ ) 
		//	cout << sigId[i] << " ";
		//cout << endl;		
	
	
		fprintf(file, "%d", sigId[0]);
		for (int i=1; i < nSigS; i++) 
			fprintf(file, ",%d", sigId[i]);
		fprintf(file, "\n");
		
		/*for (int i=0; i < nSigS; i++) 
			cout << weight[i] << " ";
		cout << endl;*/
		
		// Calculate the weighted sum
		
	  	
		CoinPackedVector sum = (*allSigS[0]->getNormalized()) * w[0];
		for (int i=1; i < nSigS; i++) { 
			CoinPackedVector temp = sum + (*allSigS[i]->getNormalized()) * w[i];
			sum = temp;
		}
	
		// Add noise to the final result	
		// cout << "adding noise..." << endl;
		if (nDim != 0) {
			int index[10000];
			double value[10000];
			for (int i=0; i < nDim; i++) {
				int x, ok;
				do {
					x = rand() % 254 + 1;
					ok = 1;
					for (int j = 0; j < i; j++) {
						if (x == index[j]) {
							ok = 0;
							break;
						}
					}
				} while (ok == 0);
				index[i] = x;
				value[i] = normal(iNoise, vNoise);
			}
		
			CoinPackedVector noiseVector(nDim, index, value);
		
			sum = sum + noiseVector;
		}
		
		char buf[1000];
		sprintf(buf, "spectrum%d", ts);
		double *data = sum.getElements();
		for (int i=0; i < sum.getNumElements(); i++) 
			if (data[i] < 0)
				data[i] = 0;

		Spectrum *resultSpectrum = new Spectrum(buf, sum.getNumElements(),
							   sum.getIndices(),
							   sum.getElements());
		//resultSpectrum->dump();
		spw.writeSpectrum(resultSpectrum);		
		spw0.writeSpectrum(resultSpectrum);
		delete resultSpectrum;
	}
	
	lw.close();
	fclose(file);
	spw.close();
	
	for (int i=0; i < nGroups; i++) {
		for (int j=0; j < i + 2; j++)
			delete sigGroup[i][j];
		delete sigGroup[i];
	}
	for (int i = 0; i < freeC; i++)
		delete freeGroup[i];
	for (int i = 0; i < nuSig; i++) 
		delete unknownGroup[i];
}
