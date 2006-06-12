#include <iostream.h>
#include <sys/time.h>

#include <DatasetGenerator.h>

int main(int argc, char ** argv) {
	struct timeval tm;
  	struct timezone tz;
  	
	gettimeofday(&tm, &tz);
	  
	srand(tm.tv_usec);
	
	const double weight[10] = {0.225, 0.20, 0.2, 0.1, 0.1, 0.05, 0.05, 0.05, 0.01, 0.01};
	//const double var[10] = {0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02};
	const double var[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	const double tile[] = {0,0.08,0.18,1};
	double PI = 3.141592653598;
	double base_error = 0.01 / 255 * sqrt(PI / 2);
	int nGroups = 5;
	
	char knownFn[1000];
	char unknownFn[1000];
		
	sprintf(knownFn, "sigSize/sigKnown.txt");
	sprintf(unknownFn, "sigSize/sigUnknown.txt");
		
	DatasetGenerator dg;
	dg.generateSignature(
		"sig_elements.txt",
		30,
		nGroups,
		knownFn,
		unknownFn);

	for (int i=0; i < 10; i++) {
		char buff[1000];
		sprintf(buff, "sigSize/sigKnown_%d.txt", i); 
		char prevFn[1000];
		if (i==0)
			sprintf(prevFn, "sigSize/sigKnown.txt");
		else 
			sprintf(prevFn, "sigSize/sigKnown_%d.txt", i-1);
		
		DatasetGenerator dataGen;
		dataGen.generateMoreSignatures(
			prevFn,
			"sigSize/sigUnknown.txt",
			buff,
			150 + i * 20
		);
	}	
	
	cout << "generating spectra..." << endl;
	
	DatasetGenerator dataGen;
	dataGen.generateSpectra(
		"sigSize/sigKnown.txt",
		"sigSize/sigUnknown.txt",
		100, // number of spectra
		10,
		0,
		1,
		254,
		1,
		nGroups, // #groups = 10
		0,
		base_error,
		weight,
		var,
		4,
		tile,
		"sigSize/spectraSet",
		"sigSize/result",
		"sigSize/label"
	);
}