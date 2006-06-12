#include <iostream.h>
#include <stdio.h>
#include <sys/time.h>
#include <DatasetGenerator.h>

void mergeFile(char *dfn, char *sfn) {
  FILE *file = fopen(dfn, "a");
  FILE *nf = fopen(sfn, "r");
  char buff[10000];
  char *res = fgets(buff, 10000, nf);
  while (res != NULL) {
	// cout << res << endl;
	if ((buff[0] != 0) && 
		(buff[0] != '%') &&
		(buff[0] != '@') &&
		(buff[0] != '\n')) {
	  fprintf(file, "%s", buff);
	}
	res = fgets(buff, 10000, nf);
  }
 
  fclose(file);
  fclose(nf);
}


int main(int argc, char ** argv) {
	struct timeval tm;
  	struct timezone tz;
  	
	gettimeofday(&tm, &tz);
	  
	srand(tm.tv_usec);

	const double weight[10] = {0.225, 0.20, 0.2, 0.1, 0.1, 0.06, 0.06, 0.03, 0.01, 0.01};
	const double var[10] = {0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02};
	const double tile[] = {0,0.08,0.18,1};
	
	int nGroups = 5;
	int nSpcs = 100;
	double PI = 3.141592653598;
	
	DatasetGenerator dataGen;
	dataGen.generateSignature
		("sig_elements.txt", // file name of element signature file
		 10, // number of unknown signatures
		 nGroups, 
		 "sigKnown.txt",
		 "sigUnknown.txt");
		 
	/*	for (int i=0; i < 5; i++) {
		char knownFn[1000];
		char unknownFn[1000];
		
		sprintf(knownFn, "sigKnown_%d.txt", i);
		sprintf(unknownFn, "sigUnknown_%d.txt", i);
		
		DatasetGenerator dg;
		dg.generateSignature(
			"sig_elements.txt",
			10 * i,
			5,
			knownFn,
			unknownFn);
			}*/
		
	// Change noise; 
	double base_error = 0.01 / 255 * sqrt(PI / 2) * 0.4;
	cout << base_error << endl;

	for (int i=0; i < 10; i++) {
		cout << "Generating spectra with noise (" << i << ")." << endl;
		char sfn[1000];
		char rfn[1000];
		char lfn[1000];
		sprintf(sfn, "noise/spectraSet%d", i);
		sprintf(rfn, "noise/result%d", i);
		sprintf(lfn, "noise/label%d", i);
		dataGen.generateSpectra(
			"sigKnown.txt",
		 	"sigUnknown.txt",
		 	nSpcs, // number of spectra
		 	10, // signatures per spectrum
		 	0, // number of unknown signatures per spectrum
		 	0, // level of ambiguity
		 	254, // number of dimensions with noise
		 	1,
		 	nGroups, // number of groups
		 	0, // intense of the noise
		 	base_error * i, // standard deviation of the noise
		 	weight,
		 	var,
		 	4,
		 	tile, 		
		 	sfn,
		 	rfn,
		 	lfn);
	}
	
	for (int i=0; i < 5; i++) {
	  //double w[10] = {0.225, 0.20, 0.2, 0.1, 0.1, 0.06, 0.06, 0.03, 0.01, 0.01};
	  //double v[10] = {0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02};

		cout << "Generate spectra with ambuigity (" << i << ")" << endl;
		char sfn[1000];
		char rfn[1000];
		char lfn[1000];
		sprintf(sfn, "ambiguity/spectraSet%d", i);
		sprintf(rfn, "ambiguity/result%d", i);
		sprintf(lfn, "ambiguity/label%d", i);
		dataGen.generateSpectra(
			"sigKnown.txt",
		 	"sigUnknown.txt",
		 	nSpcs, // number of spectra
		 	10, // signatures per spectrum
		 	0, // number of unknown signatures per spectrum
		 	(i + 1) * (i + 2) / 2 - 1, // level of ambiguity
		 	0, // number of dimensions with noise
		 	1,
		 	nGroups, // number of groups
		 	0, // intense of the noise
		 	0, // variance of the noise
		 	weight,
		 	var,
		 	4,
		 	tile, 		
		 	sfn,
		 	rfn,
		 	lfn);
	}
	
	for (int i=0; i < 10; i++) {
		double v[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		double w[10];
		for (int cl=0; cl < 10; cl++)
			w[cl] = weight[cl];
			
		w[9] = 0.002 * i;
	
		for (int j=0; j < 9; j++) 
			w[j] = w[j] * (1 - w[9]) / 0.99;
			
		cout << "Generating spectra with unknown signature..." << endl;
		char sfn[1000];
		char rfn[1000];
		char lfn[1000];
		sprintf(sfn, "unknown_no_inter/spectraSet%d", i);
		sprintf(rfn, "unknown_no_inter/result%d", i);
		sprintf(lfn, "unknown_no_inter/label%d", i);
		dataGen.generateSpectra(
			"sigKnown.txt",
		 	"sigUnknown.txt",
		 	nSpcs, // number of spectra
		 	10, // signatures per spectrum
		 	1, // number of unknown signatures per spectrum
		 	0, // level of ambiguity
		 	0, // number of dimensions with noise
		 	0, // whether the unknown signature interfere with known signatures
		 	nGroups, // number of groups
		 	0, // intense of the noise
		 	0, // variance of the noise
		 	w,
		 	v,
		 	4,
		 	tile, 		
		 	sfn,
		 	rfn,
		 	lfn);
		
		sprintf(sfn, "unknown_inter/spectraSet%d", i);
		sprintf(rfn, "unknown_inter/result%d", i);
		sprintf(lfn, "unknown_inter/label%d", i);
		dataGen.generateSpectra(
			"sigKnown.txt",
		 	"sigUnknown.txt",
		 	nSpcs, // number of spectra
		 	10, // signatures per spectrum
		 	1, // number of unknown signatures per spectrum
		 	0, // level of ambiguity
		 	0, // number of dimensions with noise
		 	1, // whether the unknown signature interfere with known signatures
		 	nGroups, // number of groups
		 	0, // intense of the noise
		 	0.0005, // variance of the noise
		 	w,
		 	v,
		 	4,
		 	tile, 		
		 	sfn,
		 	rfn,
		 	lfn);
	}
	
	for (int i=0; i < 5; i++) {
		for (int j=0; j < 5; j++) {
			cout << "Generate spectra with hybrid (" << i << "," << j << ")" << endl;
			char sfn[1000];
			char rfn[1000];
			char lfn[1000];
			sprintf(sfn, "hybrid/spectraSet%d_%d", i, j);
			sprintf(rfn, "hybrid/result%d_%d", i, j);
			sprintf(lfn, "hybrid/label%d_%d", i, j);
			dataGen.generateSpectra(
				"sigKnown.txt",
		 		"sigUnknown.txt",
		 		nSpcs, // number of spectra
		 		10, // signatures per spectrum
		 		0, // number of unknown signatures per spectrum
		 		(i + 1) * (i + 2) / 2 - 1, // level of ambiguity
		 		254, // number of dimensions with noise
		 		1,
		 		nGroups, // number of groups
		 		0, // intense of the noise
		 		base_error * j * 2 , // variance of the noise
		 		weight,
		 		var,
		 		4,
		 		tile, 		
		 		sfn,
		 		rfn,
		 		lfn);
		}
	}

	/*
	// generate training data for machine learning algorithms
	dataGen.generateSpectra("sigKnown.txt", "sigUnknown.txt", 10, 10, 0, 0, 0, 1, nGroups, 0, 0,
							weight, 4, tile, "TrainSpectra.txt", "TrainResult.txt", "TrainLabel.txt");
							
	for (int i=0; i < 10; i++) {
	  char sfn[1000];
	  char rfn[1000];
	  char lfn[1000];
	  sprintf(sfn, "spectraTemp");
	  sprintf(rfn, "resultTemp");
	  sprintf(lfn, "labelTemp");
	  dataGen.generateSpectra(
							  "sigKnown.txt",
							  "sigUnknown.txt",
							  200, // number of spectra
							  10, // signatures per spectrum
							  0, // number of unknown signatures per spectrum
							  0, // level of ambiguity
							  254, // number of dimensions with noise
							  1,
							  nGroups, // number of groups
							  0, // intense of the noise
							  base_error * i, // standard deviation of the noise
							  weight,
							  4,
							  tile, 		
							  sfn,
							  rfn,
							  lfn);
	  mergeFile("TrainSpectra.txt.weka", "spectraTemp.weka");
	  mergeFile("TrainResult.txt.weka", "resultTemp.weka");
	  mergeFile("TrainLabel.txt.weka", "labelTemp.weka");
	}

	for (int i=0; i < 5; i++) {
	  // cout << "Generate spectra with ambuigity (" << i << ")" << endl;
	  char sfn[1000];
	  char rfn[1000];
	  char lfn[1000];
	  sprintf(sfn, "spectraTemp");
	  sprintf(rfn, "resultTemp");
	  sprintf(lfn, "labelTemp");
	  dataGen.generateSpectra(
			"sigKnown.txt",
		 	"sigUnknown.txt",
		 	200, // number of spectra
		 	10, // signatures per spectrum
		 	0, // number of unknown signatures per spectrum
		 	(i + 1) * (i + 2) / 2 - 1, // level of ambiguity
		 	0, // number of dimensions with noise
		 	1,
		 	nGroups, // number of groups
		 	0, // intense of the noise
		 	0, // variance of the noise
		 	weight,
		 	4,
		 	tile, 		
		 	sfn,
		 	rfn,
		 	lfn);

	  mergeFile("TrainSpectra.txt.weka", "spectraTemp.weka");
	  mergeFile("TrainResult.txt.weka", "resultTemp.weka");
	  mergeFile("TrainLabel.txt.weka", "labelTemp.weka");
	}	
	*/
}
