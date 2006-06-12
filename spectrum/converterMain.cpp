#include <SpectrumReader.h>
#include <SpectrumWriter.h>
#include <SignatureDatabaseReader.h>
#include <SignatureDatabaseWriter.h>
#include <SignatureFileGenerator.h>
#include <QuantityLabelReader.h>

#include <CoinPackedVector.hpp>

#include <sys/time.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

char * inputFileName;
char * outputFileName;
int datatype; // 0 : spectrum file, 1 : signature file

void initializeConfig(int argc, char **argv) {
  for (int i=1; i < argc; i++) {
	char *flag = argv[i];
	if (flag[0] != '-') {
	  cerr << "Bad command line argument " << flag <<  endl;
	  exit(0);
	}

	switch (flag[1]) {
	
		case 'i' : // Input File Name
	  		if (++i < argc)
			inputFileName = strdup(argv[i]);
	  		break;

		case 'o' :
	  		if (++i < argc) {
				outputFileName = strdup(argv[i]);
	  		}
	  	break;
	
		case 't' :
			if (++i < argc) {
				sscanf(argv[i], "%d", &datatype);
			}
			break;

		default :
	  		cerr << "Bad command line parameter : " << flag << endl;
	}
  }
}

void convertSpectrumFile() {
	cout << "Converting spectra file..." << endl;
	
	SpectrumReader spr(inputFileName, SPC_SIMPLE_TEXT, 255);
	SpectrumWriter spw(outputFileName, SPC_WEKA_TEXT);

	Spectrum *sp = spr.nextSpectrum();
	while (sp != NULL) {
		spw.writeSpectrum(sp);
		delete sp;
		sp = spr.nextSpectrum();
	}

	spr.close();
	spw.close();
}

void reconstructSpectrum() {
  char * sigInputFileName = "sigKnown.txt";
  int nSigs = 83;

  cout << "Open label file..." << endl;
  QuantityLabelReader lr(inputFileName, nSigs, LBL_WEKA_TEXT);

  cout << "Open signature database..." << endl;
  SignatureDatabaseReader sdr(sigInputFileName, SIG_SIMPLE_TEXT);
  SignatureDatabase *sd = sdr.getSignatureDatabase();
  cout << "Open spectrum writer..." << endl;
  SpectrumWriter spw(outputFileName, SPC_SIMPLE_TEXT);

  cout << "begin reconstruction..." << endl;
  int cnt = 0;
  QuantityLabel *lbl = lr.readLabel();
  double *q;

  while (lbl != NULL) {
	lbl->dump();
	cnt++;
	q = lbl->getData();

	int indx[] = {1};
	double ele[] = {0};
	CoinPackedVector sum;
	sum.setVector(1, indx, ele);

	double ss  = 0;
	for (int i=0; i < nSigs; i++) 
	  if (q[i] != 0) {
		const Signature *ps = sd->getSignatureByIndex(i);
		ps->dump();
		const CoinPackedVector *v = ps->getNormalized();
		sum = sum + q[i] * (*v);
		//		q[i] += ss;
		ss+= q[i];
	  }
	cout << "ss=" << ss << endl;

	char buf[1000];
	sprintf(buf, "spectrum%d", cnt);
	Spectrum *spc = new Spectrum(buf, sum.getNumElements(),
								sum.getIndices(), sum.getElements());
	cnt++;

	spw.writeSpectrum(spc);
	lbl = lr.readLabel();
  }

  spw.close();
  delete sd;
  
}

void compareSpectraSet() {

  SpectrumReader spr0("noise/spectra_ori4.txt", SPC_SIMPLE_TEXT, 1000);
  SpectrumReader spr1("noise/spectraSet4", SPC_SIMPLE_TEXT, 1000);

  Spectrum *sp0 = spr0.nextSpectrum();
  Spectrum *sp1 = spr1.nextSpectrum();
  
  while (sp0 != NULL) {
	//	sp0->dump();
	//	sp1->dump();

	const CoinPackedVector *pv1 = sp0->getOriginal();
	const CoinPackedVector *pv2 = sp1->getOriginal();

	CoinPackedVector v = (*pv1) - (*pv2);

	int sz = v.getNumElements();
	int *indx = v.getIndices();
	double *data = v.getElements();

	double sum = 0;
	for (int i=0; i < sz; i++) 
	  sum += fabs(data[i]);
	//	cout << sum << endl; 	 
	printf("%lf\n", sum);

	//	delete sp0;
	//	delete sp1;

	sp0 = spr0.nextSpectrum();
	sp1 = spr1.nextSpectrum();
  }
}


void convertSignatureDatabase() {
	cout << "Converting signatures file..." << endl;
	SignatureDatabaseReader sdr(inputFileName, SIG_SIMPLE_TEXT);
	SignatureDatabaseWriter sdw(outputFileName, SIG_WEKA_TEXT);
	SignatureDatabase *sd = sdr.getSignatureDatabase();
	sdw.writeSignatureDatabase(sd);
	delete sd;
}

int main(int argc, char** argv) {
	initializeConfig(argc, argv);

	switch (datatype) {
	case 0 :
	  convertSpectrumFile();	
	  break;
	case 1 :
	  convertSignatureDatabase();
	  break;
	case 2 :
	  reconstructSpectrum();
	  break;
	case 4 :
	  compareSpectraSet();
	  break;
	}
}	
