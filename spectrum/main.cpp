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

#include <stdio.h>
#include <IOTester.h>
#include <ManLabelTester.h>
#include <BBTester.h>
#include <DiscreteLabelTester.h>
#include <TopKTester.h>
#include <Spectrum.h>
#include <Tile.h>
#include <GeneralTester.h>
#include <CrawlTester.h>
#include <BreadthTester.h>
#include <SimpleGroupTester.h>

#include <DiscreteLabelFinder.h>
#include <CrawlLabelFinder.h>
#include <TopKLabelFinder.h>
#include <BreadthLabelFinder.h>
#include <BranchAndBoundLabelFinder.h>

#include <DiscreteGroupLabelFinder.h>
#include <SimpleGroupLabelFinder.h>
#include <StupidGroupLabelFinder.h>
#include <BFGroupLabelFinder.h>
#include <BBGroupLabelFinder.h>

#include <SpectrumReader.h>
#include <SpectrumWriter.h>
#include <SignatureDatabaseReader.h>
#include <SignatureDatabaseWriter.h>
#include <DiscreteLabelWriter.h>
#include <QuantityLabelWriter.h>
#include <SignatureFileGenerator.h>

#include <CoinPackedVector.hpp>

#include <time.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

struct Config {
  int task;
  int quantityLabelAlgor;
  int discreteLabelAlgor;
  int groupLabelAlgor;
  Tile *pTile;
  double error;
  int topK;
  string signatureFileName;
  string spectrumFileName;
  string compoundFileName;
  string outputSignatureFileName;
  string outputLabelFileName;
  string outputQuantityLabelFileName;
  int batchSize;
  double support;

  string originalSpectraFileName;
  string outputSpectraFileName;
  double noise_variance;
  int numberSpectra;
  int numberSimilarSpectra;

  int outputQuantityLabel; 
} config;

void printUsage() {
  cout << "Usage " << endl;
}

void printConfig() {
  cout << "discrete label algorithm : " << config.discreteLabelAlgor << endl;
  cout << " tile size : " << config.pTile->getSize() << endl;
  const double *data = config.pTile->getData();
  for (int i=0; i < config.pTile->getSize(); i++) 
	cout << " " << data[i];
  cout << endl;
  cout << " error bound : " << config.error << endl;
}

void initializeConfigFromFile(char * fileName) {
  FILE *file = fopen(fileName, "r");
  char buffer[10000];

  while (fgets(buffer, 10000, file) != NULL) {

	// Trim the space in front of and at the end of the string
    char *p = buffer;
	while ((*p <= ' ') && (*p != 0)) p++;

	if (strlen(p) != 0) {
	  char *q = &p[strlen(p) - 1];
	  while (*q <= ' ') {
		*q = 0;
		if (q == p)
		  break;
		else
		  q--;
	  }
	}

	if ((p[0] != '#') && (p[0] != 0)) {
	  char *pe = strstr(p, "=");
	  *pe = 0;
	  // v is the starting point of the value string
	  char *v = pe + 1;

	  if (strcmp(p, "TASK") == 0) {

		if (strcmp(v, "signature_generation") == 0)
		  config.task = 0;
		if (strcmp(v, "single_discrete_label") == 0) 
		  config.task = 1;
		if (strcmp(v, "group_discrete_label") == 0)
		  config.task = 2;
		if (strcmp(v, "support_solution_histogram") == 0)
		  config.task = 3;
		if (strcmp(v, "generate_spectra_dataset") == 0)
		  config.task = 4;
		if (strcmp(v, "test_sensitivity") == 0)
		  config.task = 5;
		if (strcmp(v, "genereate_spectra_for_decision_table") == 0)
		  config.task = 6;
		if (strcmp(v, "generate_for_WEKA") == 0)
		  config.task = 7;
		if (strcmp(v, "generate_training_set") == 0)
		  config.task = 8;
		if (strcmp(v, "generate_quantity_label") == 0)
		  config.task = 9;

	  } else if (strcmp(p, "DISCRETE_LABEL_ALGORITHM") == 0) {

		if (strcmp(v, "branch_and_bound") == 0) 
		  config.discreteLabelAlgor = 0;
 		if (strcmp(v, "crawling") == 0) 
		  config.discreteLabelAlgor = 1;
 		if (strcmp(v, "topK") == 0) 
		  config.discreteLabelAlgor = 2;
		if (strcmp(v, "breadth") == 0) 
		  config.discreteLabelAlgor = 3;

	  } else if (strcmp(p, "GROUP_LABEL_ALGORITHM") == 0) {

		if (strcmp(v, "simple") == 0) 
		  config.groupLabelAlgor = 0;
		if (strcmp(v, "branch_and_bound") == 0)
		  config.groupLabelAlgor = 1;
		if (strcmp(v, "naive") == 0) 
		  config.groupLabelAlgor = 2;

	  } else if (strcmp(p, "QUANTITY_LABEL_ALGORITHM") == 0) {
		config.quantityLabelAlgor = 0;
	  } else if (strcmp(p, "TILE") == 0) {
		int ts;
		istringstream instream;
		instream.str(v);
		instream >> ts;
		double arr[1000];

		for (int i=0; i < ts; i++)
		  instream >> arr[i];

		config.pTile = new Tile(ts, arr);
	  } else if (strcmp(p, "ERROR_BOUND") == 0) {
		sscanf(v, "%lf", &(config.error));
	  } else if (strcmp(p, "TOP_K") == 0) {
		sscanf(v, "%d", &(config.topK));
	  } else if (strcmp(p, "SUPPORT") == 0) {
		sscanf(v, "%lf", &(config.support));
	  } else if (strcmp(p, "SIGNATURE_FILENAME") == 0) {
		config.signatureFileName = v;
	  } else if (strcmp(p, "SPECTRUM_FILENAME") == 0) {
		config.spectrumFileName = v;
	  } else if (strcmp(p, "COMPOUND_FILENAME") == 0) {
		config.compoundFileName = v;
	  } else if (strcmp(p, "OUTPUT_SIGNATURE_FILENAME") == 0) {
		config.outputSignatureFileName = v;
	  } else if (strcmp(p, "OUTPUT_LABEL_FILENAME") == 0) {
		config.outputLabelFileName = v;
	  } else if (strcmp(p, "OUTPUT_QUANTITY_LABEL_FILENAME") == 0) {
		config.outputQuantityLabelFileName = v;
	  } else if (strcmp(p, "BATCH_SIZE") == 0) {
		sscanf(v, "%d", &(config.batchSize));
	  } else if (strcmp(p, "ORIGINAL_SPECTRA_FILENAME") == 0) {
		config.originalSpectraFileName = v;
	  } else if (strcmp(p, "OUTPUT_SPECTRA_FILENAME") == 0) {
		config.outputSpectraFileName = v;
	  } else if (strcmp(p, "NOISE_VARIANCE") == 0) {
		sscanf(v, "%lf", &config.noise_variance);
	  } else if (strcmp(p, "NUMBER_SIMILAR_SPECTRA") == 0) {
		sscanf(v, "%d", &config.numberSimilarSpectra);
	  } else if (strcmp(p, "NUMBER_SPECTRA") == 0) {
		sscanf(v, "%d", &config.numberSpectra);
	  } else if (strcmp(p, "OUTPUT_QUANTITY_LABEL") == 0) {
		sscanf(v, "%d", &config.outputQuantityLabel);
	  }
	}
  }
  
  fclose(file);
}

void initializeConfig(int argc, char **argv) {
  config.pTile = NULL;
  config.outputLabelFileName = "";

  if (argc == 1) {
	printUsage();
	exit(1);
  }

  for (int i=1; i < argc; i++) {
	char *flag = argv[i];
	if (flag[0] != '-') {
	  cerr << "Bad command line argument " << flag <<  endl;
	  printUsage();
	  exit(0);
	}

	switch (flag[1]) {

	case 'c' :
	  if (++i < argc)
		initializeConfigFromFile(argv[i]);
	  break;

	case 't' :
	  if (++i < argc) {
		sscanf(argv[i], "%d", &(config.task));
	  }
	  break;
	
	case 'd' :
	  if (++i < argc) 
		sscanf(argv[i], "%d", &(config.discreteLabelAlgor));
	  break;

	case 'q' :
	  if (++i < argc) 
		sscanf(argv[i], "%d", &(config.quantityLabelAlgor));
	  break;

	case 'g' :
	  if (++i < argc) 
		sscanf(argv[i], "%d", &(config.groupLabelAlgor));
	  break;

	case 'l' :
	  if (++i < argc) {
		int ts;
		sscanf(argv[i], "%d", &ts);

		double arr[1000];

		for (int j=0; j < ts; j++) 
		  if (++i < argc)
			sscanf(argv[i], "%lf", &arr[j]);

		config.pTile = new Tile(ts, arr);
	  }
	  break;

	case 'e' :
	  if (++i < argc) 
		sscanf(argv[i], "%lf", &(config.error));
 	  break;

	case 'k' :
	  if (++i < argc)
		sscanf(argv[i], "%d", &(config.topK));
	  break;

	case 's' :
	  if (++i < argc)
		sscanf(argv[i], "%lf", &(config.support));
	  break;

	case 'b' :
	  if (++i < argc) 
		sscanf(argv[i], "%d", &(config.batchSize));
	  break;

	case 'f' :
	  if (strlen(flag) < 2) 
		cerr << "Bad command line parameter." << endl;

	  if (++i < argc) {
		char buffer[2000];
		sscanf(argv[i], "%s", buffer);

		switch (flag[2]) {
		case 's' :		  
		  config.signatureFileName = buffer;
		  break;
		case 'c' :
		  config.compoundFileName = buffer;
		  break;
		case 'o' :
		  config.outputSignatureFileName = buffer;
		  break;
		case 'l' :
		  config.outputLabelFileName = buffer;		
		  break;
		case 'p' :
		  config.spectrumFileName = buffer;
		case 'q':
		  config.outputQuantityLabelFileName = buffer;
		default :
		  break;
		}
	  }
	  break;

	default :
	  cerr << "Bad command line parameter : " << flag << endl;
	}
  }
}

Spectrum *reconstructSpectrum(const SignatureDatabase *sd, 
							  const QuantityLabel *label) {
  const int size = sd->sizeOfSignature();
  double quan[5000];

  sd->times(label->getData(), quan);

  int ind[5000];
  double ele[5000];
  int count = 0;
  for (int i=0; i < size; i++) 
	if (quan[i] != 0) {
	  ind[count] = i;
	  ele[count] = quan[i];
	  count++;
	}

  Spectrum *sp = new Spectrum("spectrum", count, ind, ele);

  return sp;
}

void doGroupLabel() {
  time_t tstart, tend;

  SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(), 
									SIG_SIMPLE_TEXT);
  SignatureDatabase *sd = sd_reader.getSignatureDatabase();

  DiscreteGroupLabelFinder *pLabeler = NULL;
  switch (config.groupLabelAlgor) {
  case 0 :
	//	cout << " Choose simple group label" << endl;
	pLabeler = new SimpleGroupLabelFinder(sd);
	break;
  case 1 : 
	//	cout << " Choose brand-and-bound group label" << endl;
	pLabeler = new BBGroupLabelFinder(sd);
	break;
  case 2:
	pLabeler = new StupidGroupLabelFinder(sd);
	break;

  default :
	cout << "Error : Unknown labeling algorithm " << endl;
	break;
  }

  pLabeler->setTile(config.pTile);
  pLabeler->setError(config.error);
  pLabeler->setSupport(config.support);

  SpectrumReader s_reader(config.spectrumFileName.c_str(), 
						  SPC_SIMPLE_TEXT,
						  sd->sizeOfSignature());
  Spectrum *ps = s_reader.nextSpectrum();
  while (ps != NULL) {
	pLabeler->addSpectrum(ps);
	delete ps;
	ps = s_reader.nextSpectrum();
  }

  time(&tstart);
  pLabeler->clearSolveCount();

  pLabeler->init();

  DiscreteLabel *label = pLabeler->getNext();	
  int solCount = 0;
  while (label != NULL) {
	solCount++;
	/*int arrSize = label->getSize();
	const int *data = label->getData();

	cout << " Error : " << label->getError() << endl;
	
	cout << " Discrete Label : ";
	for (int i=0; i < arrSize; i++)
	  cout << data[i] << " ";
	cout << endl;
	
	if (label->getQuantityLabel() != NULL) {
	  cout << " Quantity Label : ";
	  label->getQuantityLabel()->dump();
	 
	  cout << " Restore spectrum : ";
	  Spectrum *spc = reconstructSpectrum(sd, label->getQuantityLabel());
	  //	  spc->dump();
	  
	  delete spc;
	  }*/
	
	delete label;
	label = pLabeler->getNext();
  }

  time(&tend);
  cout << config.groupLabelAlgor << " "
	   << config.support << " " 
	   << pLabeler->getSolveCount() << " " 
	   << solCount << " " 
	   << tend - tstart << endl;

  delete pLabeler;
  delete sd;
}

void doSingleLabel() {
	//cout << "read signatures" << endl;
	
	SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(), SIG_SIMPLE_TEXT);
	
  	cout << "reading signature database..." << endl;
  	SignatureDatabase *sd = sd_reader.getSignatureDatabase();

	//	for (int i=0; i < sd->numSignatures(); i++) 
	//	const Signature * sig = sd->getSignatureByIndex(i);
	
  	DiscreteLabelFinder *pLabeler = NULL;

  	cout << "reading spectra dataset ..." << endl;
  	
  	SpectrumReader s_reader(config.spectrumFileName.c_str(), SPC_SIMPLE_TEXT, 255);

  	Spectrum *ps = s_reader.nextSpectrum();
	//  	const double *data = config.pTile->getData();

	switch (config.discreteLabelAlgor) {
	case 0 :
	  pLabeler = new BranchAndBoundLabelFinder(sd);
	  break;
	case 1 : 
	  pLabeler = new CrawlLabelFinder(sd);
	  break;
	case 2 :
	  pLabeler = new TopKLabelFinder(sd);
	  ((TopKLabelFinder *)pLabeler)->setK(config.topK);
	  break;
	case 3 :
	  pLabeler = new BreadthLabelFinder(sd);
	default :
	  break;
	}

	pLabeler->setTile(config.pTile);
	pLabeler->setErrorBound(config.error);

/*	struct timeval tbegin;
	struct timeval tend;
	struct timezone tz;
*/
	int pc = 0; 
  
	cout << "Label filename: " << config.outputLabelFileName << endl;
	DiscreteLabelWriter dlw(config.outputLabelFileName.c_str(), sd->numSignatures(), LBL_WEKA_TEXT);
	dlw.writeMeta();

	// Initialize the quantity label writer
	QuantityLabelWriter *qlw;
	if (config.outputQuantityLabel != 0) {
	  cout << "init quantity label writer..." << endl;
	  qlw = new QuantityLabelWriter(config.outputQuantityLabelFileName.c_str(), 
									sd->numSignatures(), LBL_WEKA_TEXT);
	  qlw->writeMeta();
	}

	//*************************************************************
	// The spw keeps spectrum with desirable labels
	//	SpectrumWriter spw("spc_deb.txt", SPC_SIMPLE_TEXT);
	//  QuantityLabelWriter qlw("lbl_deb.txt", sd->numNonEmptySignatures(), LBL_CLEAR_TEXT);
	//*************************************************************	
  
	double totalInterval = 0;
	long totalSolveCount = 0;
  
	pLabeler->setTile(config.pTile);
	pLabeler->setErrorBound(config.error);
  
	while (ps != NULL) {
	  if (ps->getSize() != 0) {

//		gettimeofday(&tbegin, &tz);

		pLabeler->setSpectrum(ps);

	  	 
		pLabeler->init();
		if (config.discreteLabelAlgor == 3)
		  ((BreadthLabelFinder *)pLabeler)->execute();
	
		DiscreteLabel *label = pLabeler->getNext();

		int solutionCount = 0;
	
		pc++;
		while (label != NULL) {
		  solutionCount++;
		
		  //cout << "new label" << endl;
		
		  /*if (solutionCount == 1) {
			pc++;
			}*/
		
		  dlw.writeLabel(pc, label);

		  //*****************************************************************
		  if (config.outputQuantityLabel != 0) {
			//	cout << "write quantity label..." << endl;
			qlw->writeLabel(pc, label->getQuantityLabel()->getData(), sd);
		  }
		  //*****************************************************************

		  /*int arrSize = label->getSize();
			const int *data = label->getData();

			cout << " Error : " << label->getError() << endl;
			cout << " Discrete Label : ";
			for (int i=0; i < arrSize; i++)
			cout << data[i] << " ";
			cout << endl;

			if (label->getQuantityLabel() != NULL) {
			cout << " Quantity Label : ";
			label->getQuantityLabel()->dump();
		
			cout << " Restore spectrum : ";
			Spectrum *spc = reconstructSpectrum(sd, label->getQuantityLabel());
			spc->dump();
		
			delete spc;
			}*/

		  delete label;
		  label = pLabeler->getNext();
		} 
		delete label;

//		gettimeofday(&tend, &tz);

		double interval = -1;
//		  (double)(tend.tv_sec - tbegin.tv_sec) * 1000000 + (double)tend.tv_usec - (double)tbegin.tv_usec;

		cout << "(" << pc << ") " << solutionCount << " " << pLabeler->getSolveCount() << " " 
			 << interval << " " << endl;

		//***********************************************************
		//if ((solutionCount > 2) && (solutionCount < 8))
		//  spw.writeSpectrum(ps);
		//***********************************************************

		totalInterval += interval;
		totalSolveCount += pLabeler->getSolveCount();
		pLabeler->clearSolveCount();
	  }

	  delete ps;
	  ps = s_reader.nextSpectrum();
	}
  
	cout << totalSolveCount << endl;

	cout << totalInterval << endl;
	
	//	spw.close();
	qlw->close();
	delete qlw;
	dlw.close();
	delete pLabeler;
	delete sd;
}

void generateOneQuantityLabel()
{
  SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(), SIG_SIMPLE_TEXT);
  SignatureDatabase *sd = sd_reader.getSignatureDatabase();
  int ss = sd->numSignatures();
  
  ManhattenDistanceLabelFinder *finder = new ManhattenDistanceLabelFinder(*sd);
 
  FILE *file = fopen(config.outputQuantityLabelFileName.c_str(), "w");
  
  SpectrumReader s_reader(config.spectrumFileName.c_str(), SPC_SIMPLE_TEXT, 500);
  Spectrum *sp = s_reader.nextSpectrum();
  while (sp != NULL) {
	QuantityLabel *label = finder->solve(sp);
	fprintf(file, "%s %d\n", sp->getName().c_str(), 1);
	
	double * arr = label->getData();
	for (int i=0; i < ss; i++) {
	  if (arr[i] > 1e-10)
		fprintf(file, "%d %lf ", i, arr[i]);
	}
	fprintf(file, "\n");

	printf("%s\n", sp->getName().c_str());
	
	delete sp;
	sp = s_reader.nextSpectrum();
  }

  fclose(file);
}
  
  // Generate label files for the ML training 
  // All the spectrum should have at least one label
  void generateSingleLabel() {
	SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(), 
									  SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sd_reader.getSignatureDatabase();

	DiscreteLabelFinder *pLabeler = NULL;

	SpectrumReader s_reader(config.spectrumFileName.c_str(), 
							SPC_SIMPLE_TEXT, 255);

	Spectrum *ps = s_reader.nextSpectrum();
	//  const double *data = config.pTile->getData();
	//cout << data[1] << endl;

	switch (config.discreteLabelAlgor) {
	case 0 :
	  pLabeler = new BranchAndBoundLabelFinder(sd);
	  break;
	case 1 : 
	  pLabeler = new CrawlLabelFinder(sd);
	  break;
	case 2 :
	  pLabeler = new TopKLabelFinder(sd);
	  ((TopKLabelFinder *)pLabeler)->setK(config.topK);
	  break;
	case 3 :
	  pLabeler = new BreadthLabelFinder(sd);
	default :
	  break;
	}

	pLabeler->setTile(config.pTile);
	pLabeler->setErrorBound(config.error);

/*	struct timeval tbegin;
	struct timeval tend;
	struct timezone tz;
*/
	int pc = 0; 
  
	//cout << "init label writer" << endl;
	DiscreteLabelWriter dlw(config.outputLabelFileName.c_str(), sd->numSignatures(), LBL_WEKA_TEXT);
	dlw.writeMeta();
	SpectrumWriter spw(config.outputSpectraFileName.c_str(), SPC_WEKA_TEXT);
  
	while (ps != NULL) {
	  //if (pc == 296) break;
	  if (ps->getSize() != 0) {

//		gettimeofday(&tbegin, &tz);

		pLabeler->setSpectrum(ps);
	 
		pLabeler->init();
		if (config.discreteLabelAlgor == 3)
		  ((BreadthLabelFinder *)pLabeler)->execute();
	
		DiscreteLabel *label = pLabeler->getNext();

		int solutionCount = 0;

		while (label != NULL) {
		  solutionCount++;
		  //	if (solutionCount == 1) {
		  //		pc++;
		  //		spw.writeSpectrum(ps);
		  //	}
		
		  //cout << "new label" << endl;
		
		  pc++;
		  dlw.writeLabel(pc, label);

		  /*int arrSize = label->getSize();
			const int *data = label->getData();

			cout << " Error : " << label->getError() << endl;
			cout << " Discrete Label : ";
			for (int i=0; i < arrSize; i++)
			cout << data[i] << " ";
			cout << endl;

			if (label->getQuantityLabel() != NULL) {
			cout << " Quantity Label : ";
			label->getQuantityLabel()->dump();
		
			cout << " Restore spectrum : ";
			Spectrum *spc = reconstructSpectrum(sd, label->getQuantityLabel());
			spc->dump();
		
			delete spc;
			}*/

		  delete label;
		  label = pLabeler->getNext();
		} 
		delete label;

//		gettimeofday(&tend, &tz);

		long interval = -1;
//		  (tend.tv_sec - tbegin.tv_sec) * 1000000 + tend.tv_usec - tbegin.tv_usec;

		cout << solutionCount << " " 
			 << pLabeler->getSolveCount() << " " 
			 << interval << " " << pc << endl;
		pLabeler->clearSolveCount();
	  }

	  delete ps;
	  ps = s_reader.nextSpectrum();
	}

	spw.close();
	dlw.close();
	delete pLabeler;
	delete sd;
  }

  // In this method, we test the sensitivity of the threshold
  void testSensitivity() {
	int count[110];  

	SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(),
									  SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sd_reader.getSignatureDatabase();

	//   BranchAndBoundLabelFinder *pLabeler = new BranchAndBoundLabelFinder(sd);
	//   pLabeler->setTile(config.pTile);
	//   pLabeler->setError(config.error);
	ManhattenDistanceLabelFinder *qLabeler = new ManhattenDistanceLabelFinder(*sd);
  
	SpectrumReader s_reader(config.spectrumFileName.c_str(), SPC_SIMPLE_TEXT, 255);
  
	Spectrum *ps = s_reader.nextSpectrum();
	memset(count, 0, sizeof(int) * 110);
	while (ps != NULL) {
	  cout << "*" << endl;
	  QuantityLabel *ql = qLabeler->solve(ps);

	  if (ql != NULL) {
		double * data = ql->getData();
		int size = ql->getSize();
		for (int i=0; i < size; i++) 
		  if (data[i] > 0.005) 
			count[int(data[i] * 100)]++;
	  }

	  delete ql;
	
	  ps = s_reader.nextSpectrum();
	}

	FILE *file = fopen("quan_dist.txt", "w");
	for (int i=0; i < 101; i++) 
	  fprintf(file, "%d %d\n", i, count[i]);
	fclose(file);

	delete qLabeler;
	delete sd;
  }
  
  void doHistogram() {
	SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(), 
									  SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sd_reader.getSignatureDatabase();

	BFGroupLabelFinder *pLabeler = new BFGroupLabelFinder(sd);

	pLabeler->setTile(config.pTile);
	pLabeler->setError(config.error);
	pLabeler->setSupport(config.support);

	SpectrumReader s_reader(config.spectrumFileName.c_str(), 
							SPC_SIMPLE_TEXT, 255);

	int sc = 0;
	Spectrum *ps = s_reader.nextSpectrum();
	while (ps != NULL) {
	  sc++;
	  pLabeler->addSpectrum(ps);
	  delete ps;
	  ps = s_reader.nextSpectrum();
	}

	pLabeler->init();

	int *his = pLabeler->getHistogram();

	int sum = 0;
	for (int i=0; i <= sc; i++) {
	  cout << his[i] << endl;
	  sum+= his[i];
	}

	for (int i=1; i <= sc; i++) {
	  cout << i << " " << sum << endl;
	  sum -= his[i];
	}

	delete pLabeler;
	delete sd;

  }

  void generateSpectraAndSignatureForWEKA() {
	cout << "Converting spectra file..." << endl;
	// write the WEKA spectrum
	SpectrumReader spr(config.originalSpectraFileName.c_str(), SPC_SIMPLE_TEXT, 255);
	SpectrumWriter spw(config.outputSpectraFileName.c_str(), SPC_WEKA_TEXT);
	
	//cout << "OK" << endl;
	
	Spectrum *sp = spr.nextSpectrum();
	while (sp != NULL) {
	  //sp->dump();
	  spw.writeSpectrum(sp);
	  delete sp;
	  sp = spr.nextSpectrum();
	}
	
	spr.close();
	spw.close();
	
	cout << "Converting signatures file..." << endl;
	SignatureDatabaseReader sdr(config.signatureFileName.c_str(), SIG_SIMPLE_TEXT);
	SignatureDatabaseWriter sdw(config.outputSignatureFileName.c_str(), SIG_WEKA_TEXT);
	SignatureDatabase *sd = sdr.getSignatureDatabase();
	sdw.writeSignatureDatabase(sd);
	
	delete sd;
  }
  	
  // In this method, we generate spectra for the decision table
  // So the data similarity indicator X and the support should 
  // be some random value
  void generateSpectraForDecisionTable() {
	Spectrum *sp0 = NULL;
	SpectrumReader spr(config.originalSpectraFileName.c_str(), 
					   SPC_SIMPLE_TEXT, 255);

	// get the set of spectrum
	vector <Spectrum *> spcSet;
	Spectrum *sp = spr.nextSpectrum();
	while (sp != NULL) {
	  if ((sp->getMaxiumIndex() <= 255) && (sp->getSize() > 0))
		spcSet.push_back(sp);
	  sp = spr.nextSpectrum();
	}

	// Initialize the labeler
	SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(), SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sd_reader.getSignatureDatabase();
	BranchAndBoundLabelFinder *pLabeler = new BranchAndBoundLabelFinder(sd);
	pLabeler->setTile(config.pTile);
	pLabeler->setErrorBound(config.error);

	// Pick the sp0 which has suitable number of labels
	for (vector <Spectrum *>::iterator it = spcSet.begin();
		 it != spcSet.end(); it++) {
	  pLabeler->setSpectrum(*it);
	  pLabeler->init();
	  DiscreteLabel *pl = pLabeler->getNext();
	  int count = 0;
	  while (pl != NULL) {
		count++;
		delete pl;
		pl = pLabeler->getNext();
	  }

	  cout << count << endl;
	
	  if ((count < 20) && (count > 15)) {
		sp0 = *it;
		break;
	  }
	}

	FILE *file = fopen("metafile", "w");
	for (int i=0; i < 50; i++) {
	  double support = double(rand() % 100) / 100;
	  double similar = double(rand() % 100) / 100;
	  char fn[1000];
	  sprintf(fn, "sample%d.txt", i);
	  fprintf(file, "%lf %lf %s\n", support, similar, fn);

	  SpectrumWriter spw(fn, SPC_SIMPLE_TEXT);	
	  int dups = int((double)config.numberSpectra * similar);
	  cout << dups << endl;
	  int spcSize = config.numberSpectra - dups;
	  int cnt[10000];
	  memset(cnt, 0, sizeof(int) * spcSize);
	  for (int i=0; i < dups; i++) {
		//	  int idx = (int)(rand() * spcSize);
		cnt[abs(rand()) % spcSize]++;
	  }
	
	  // Create new spectra file
	  for (int i=0; i < spcSize; i++) {
		int indx = rand() % spcSet.size();	
		while (spcSet[indx]->getSize() == 0) {
		  indx = rand() % spcSet.size();
		}
		spw.writeSpectrum(spcSet[indx]);
	  
		for (int j =0; j < cnt[i]; j++)
		  spw.writeSpectrum(sp0);
	  }

	  spw.close();
	}

	delete pLabeler;
	delete sd;

	// Free the spectrum set
	for (vector<Spectrum *>::iterator it = spcSet.begin();
		 it != spcSet.end(); it++) {
	  delete *it;
	}
	spcSet.clear();
  }

  // This method genereates the test cases from existing spectra
  // database. The test cases have different potions of identical 
  // spectra
  void doSpectraGeneration() {
	Spectrum *sp0 = NULL;
	SpectrumReader spr(config.originalSpectraFileName.c_str(), 
					   SPC_SIMPLE_TEXT, 255);

	// get the set of spectrum
	vector <Spectrum *> spcSet;
	Spectrum *sp = spr.nextSpectrum();
	while (sp != NULL) {
	  if ((sp->getMaxiumIndex() <= 255) && (sp->getSize() > 0))
		spcSet.push_back(sp);
	  sp = spr.nextSpectrum();
	}

	// Initialize the labeler
	SignatureDatabaseReader sd_reader(config.signatureFileName.c_str(), 
									  SIG_SIMPLE_TEXT);
	SignatureDatabase *sd = sd_reader.getSignatureDatabase();
	BranchAndBoundLabelFinder *pLabeler = new BranchAndBoundLabelFinder(sd);
	pLabeler->setTile(config.pTile);
	pLabeler->setErrorBound(config.error);

	vector <DiscreteLabel *> labelSet;
 
	// Pick the sp0 which has suitable number of labels
	for (vector <Spectrum *>::iterator it = spcSet.begin();
		 it != spcSet.end(); it++) {
	  pLabeler->setSpectrum(*it);
	  pLabeler->init();
	  DiscreteLabel *pl = pLabeler->getNext();
	  int count = 0;
	  while (pl != NULL) {
		count++;
		labelSet.push_back(pl);
		pl = pLabeler->getNext();
	  }
	
	  if ((count < 20) && (count > 15)) {
		sp0 = *it;
		break;
	  } else {
		cout << count << endl;
		for (vector <DiscreteLabel *>::iterator dit = labelSet.begin();
			 dit != labelSet.end(); dit++) 
		  delete (*dit);
		labelSet.clear();
	  }
	}

	// Decide the position to put those identical spectra
	//  int dups = config.numberSimilarSpectra;

	for (int r=1; r < 10; r++) {
	  char ofn[1000];
	  sprintf(ofn, "%s%d.txt", config.outputSpectraFileName.c_str(), r);

	  SpectrumWriter spw(ofn, SPC_SIMPLE_TEXT);	
	  int dups = int((double)config.numberSpectra * r / 10.0);
	  cout << dups << endl;
	  int spcSize = config.numberSpectra - dups;
	  int cnt[10000];
	  memset(cnt, 0, sizeof(int) * spcSize);
	  for (int i=0; i < dups; i++) {
		//	  int idx = (int)(rand() * spcSize);
		cnt[abs(rand()) % spcSize]++;
	  }
	
	  // Create new spectra file
	
	  for (int i=0; i < spcSize; i++) {
		int indx = rand() % spcSet.size();	
		while (spcSet[indx]->getSize() == 0) {
		  indx = rand() % spcSet.size();
		}
		spw.writeSpectrum(spcSet[indx]);
	  
		for (int j =0; j < cnt[i]; j++)
		  spw.writeSpectrum(sp0);
	  }

	  spw.close();
	}

	// Create test case with similar spectra
	for (int r=1; r < 10; r++) {
	  char ofn[1000];
	  sprintf(ofn, "%s%ds.txt", config.outputSpectraFileName.c_str(), r);

	  SpectrumWriter spw(ofn, SPC_SIMPLE_TEXT);	
	  int dups = int((double)config.numberSpectra * r / 10.0);
	  cout << dups << endl;
	  int spcSize = config.numberSpectra - dups;
	  int cnt[10000];
	  memset(cnt, 0, sizeof(int) * spcSize);
	  for (int i=0; i < dups; i++) {
		//	  int idx = (int)(rand() * spcSize);
		cnt[abs(rand()) % spcSize]++;
	  }
	
	  // Create new spectra file	
	  for (int i=0; i < spcSize; i++) {
		int indx = rand() % spcSet.size();	
		while (spcSet[indx]->getSize() == 0) {
		  indx = rand() % spcSet.size();
		}
		spw.writeSpectrum(spcSet[indx]);
	  
		for (int j =0; j < cnt[i]; j++) {
		  // 		int k;
		  // 		Spectrum * spz;
		  // 		do {
		  // 		  k = rand() % spcSize;
		  // 		  spz = spcSet[k];
		  // 		} while (spz->getDistance(sp0) < 0.05);
		  // 		spw.writeSpectrum(spz);

		  // In this procedure, we need to randomly find a quantity label of sp0
		  // and change it a little bit, then put it back to the test case.
		
		  // First, we pick a discrete label
		  int pc = rand() % labelSet.size();
		  DiscreteLabel *pl = labelSet[pc];
		  const QuantityLabel *ql = pl->getQuantityLabel(); // get the quantity label
		  QuantityLabel *nq = new QuantityLabel(*ql);
		  int size = nq->getSize();
		  double *data = nq->getData();

		  // Change at most 2.5%
		  for (int i=0; i < size; i++) 
			data[i] = data[i] * (1 + (double)(rand() % 5 - 2) / 100);

		  Spectrum *nsp = reconstructSpectrum(sd, nq);
		  nsp->setName(sp0->getName());
		  spw.writeSpectrum(nsp);
		}
	  }

	  spw.close();
	}


	// Free the label set (for sp0)
	for (vector <DiscreteLabel *>::iterator dit = labelSet.begin();
		 dit != labelSet.end(); dit++) 
	  delete (*dit);
	labelSet.clear();

	// Free the spectrum set
	for (vector<Spectrum *>::iterator it = spcSet.begin();
		 it != spcSet.end(); it++) {
	  delete *it;
	}
	spcSet.clear();

	// Free the signature database
	delete sd;
  }

  int main(int argc, char **argv) {
	time_t tm;
	srand(time(&tm));
    
	initializeConfig(argc, argv);

	switch (config.task) {

	case 0 : {//generate signatures from compound list
	  printf("%s\n", config.signatureFileName.c_str());
	  printf("%s\n", config.compoundFileName.c_str());
	  printf("%s\n", config.outputSignatureFileName.c_str());
	  
	  SignatureFileGenerator sigGen(config.signatureFileName.c_str(),
									config.compoundFileName.c_str(),
									config.outputSignatureFileName.c_str());
	  break;
	}
  
	case 1 :
	  doSingleLabel();
	  break;

	case 2 :
	  for (double z = 0.1; z < 1.01; z+=0.1) {
		config.support = z;
		doGroupLabel();
	  }
	  break;

	case 3 :
	  doHistogram();
	  break;

	case 4 : 
	  doSpectraGeneration();
	  break;

	case 5 :
	  testSensitivity();
	  break;

	case 6 :
	  generateSpectraForDecisionTable();
	  break;
	
	case 7 :
	  generateSpectraAndSignatureForWEKA();
	  break;

	case 8 : 
	  generateSingleLabel();
	  break;

	case 9:
	  generateOneQuantityLabel();
	  break;

	default :
	  break;
	}

	if (config.pTile != NULL)
	  delete config.pTile;

	return 0;	
  }
