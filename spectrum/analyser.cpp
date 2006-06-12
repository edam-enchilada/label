#include <stdio.h>
#include <DiscreteLabelReader.h>

#include <iostream>
using namespace std;

#define TRUE 1
#define FALSE 0

int sigSize = 83;
int nSpc = 100;

int labelCompare(DiscreteLabel *l1, DiscreteLabel *l2) {
  int sz = l1->getSize();
  int *data1 = l1->getData();
  int *data2 = l2->getData();

  for (int i=0; i < sz; i++) 
	if (data1[i] != data2[i])
	  return FALSE;
  
  return TRUE;
}

DiscreteLabel *aggLabel(DiscreteLabel **lset, int lcount) {
  if (lcount == 0) 
	return NULL;

  int ocount[sigSize][3];

  for (int i=0; i < sigSize; i++) 
	for (int j=0; j< 3; j++) 
	  ocount[i][j] = 0;

  for (int i=0; i < lcount; i++) {
	int *data = lset[i]->getData();
	for (int j=0;j < sigSize; j++)
	  ocount[j][data[j]]++;
  }

  int ele[sigSize];
  for (int i=0; i <sigSize; i++) {
	if (ocount[i][2] + ocount[i][1] == lcount)
	  ele[i] = 2;
	else 
	  if (ocount[i][0] == lcount) {
		ele[i] = 0;
	  } else
		ele[i] = 1;  
  }

  DiscreteLabel *l = new DiscreteLabel(sigSize, ele);

  return l;
}

int calcHit(DiscreteLabel *l1, DiscreteLabel *l2, int *res) {
  int sum = 0;
  int sz = l1->getSize();
  int *data1 = l1->getData();
  int *data2 = l2->getData();
  for (int i=0; i < 10; i++) {
	if (res[i] >= 0) {
	  if (data1[res[i]] == data2[res[i]]) {
		sum++;
	  }
	}
  }
  return sum;
}
 
void test(char *rfn, char *ofn, char *wfn, char *resfn,
		  double *hit, double *miss, double *phit) {

  //    cout << rfn << endl;
  FILE *rf = fopen(resfn, "r");
  char buf[10000];
  fgets(buf, 10000, rf);
  while (strstr(buf, "@data") != buf) {
	  fgets(buf, 10000, rf);
  }
		 
  DiscreteLabelWriter lw2(wfn, sigSize, LBL_WEKA_TEXT);
  lw2.writeMeta();

  DiscreteLabelReader lr1(rfn, sigSize, LBL_WEKA_TEXT);
  DiscreteLabelReader lr2(ofn, sigSize, LBL_WEKA_TEXT);
  
  DiscreteLabel *lset1[4000];
  DiscreteLabel *lset2[4000];
  
  // sid1 and sid2 are the ids of the spectrum for current
  // label
  int sid1, sid2;
  
  // number of labels in the current label set
  int c1, c2, cc1, cc2;
  
  //	cout << "read first label..." << endl;
  DiscreteLabel *l1 = lr1.readLabel(&sid1);
  DiscreteLabel *l2 = lr2.readLabel(&sid2);
    
  //cout << "Start processing..." << endl;
  // Initialize the hit and miss count
  *hit = *miss = *phit = 0;
  
  cc1 = 0;
  cc2 = 0;

  int missCount = 0;
  int totalCount = 0;

  for (int sp=1; sp <= nSpc; sp++) {
	int res[10];
	fscanf(rf, "%d", &(res[0]));
	for (int i=1; i<10; i++) 
	  fscanf(rf, ",%d", &(res[i]));

	/*	for (int i=0; i < 10; i++)
	  cout << res[i] << " ";
	  cout << endl;*/

	//	  cout << sp << endl;
	// read the labels into array
	c1 = 0;
	while ((l1 != NULL) && (sid1 == sp)) {
	  //l1->dump();
	  lset1[c1++] = l1;
	  l1 = lr1.readLabel(&sid1);
	}
	if (c1 != 0) cc1++;
	DiscreteLabel *ln1 = aggLabel(lset1, c1);	  	
	
	c2 = 0;
	while ((l2 != NULL) && (sid2 == sp)) {
	  //l2->dump();
	  lset2[c2++] = l2;
	  l2 = lr2.readLabel(&sid2);
	}

	if (c2 != 0) cc2++;

	DiscreteLabel *ln2 = aggLabel(lset2, c2);
	if (ln2 != NULL) {
	  lw2.writeLabel(-1, ln2);

	  if (ln1 != NULL) {
		(*phit) += calcHit(ln1, ln2, res);
	  }
	} else {
	  cout << "ERROR..." << endl;
	}

	delete ln1;
	delete ln2;

	//	  cout << "Start comparingin..." << endl;
	//	  cout << c1 << " " << c2 << endl;

	int hc = 0;
	for (int i=0; i < c1; i++) {
	  int *resData = lset1[i]->getData();		
	  for (int j=0; j < c2; j++) {
		//		  cout << "y" << endl;
		int *oriData = lset2[j]->getData();
		//		  cout << "x" << endl;
		int same = TRUE;
		for (int k=0; k < sigSize; k++) {
		  if (resData[k] != oriData[k]) {
			same = FALSE;
			break;
		  }
		}

		if (same == TRUE) {
		  hc++;
		  break;
		}
	  }
	}
	 

	// update the hit and miss ratio
	if (c2 != 0) {
	  *hit += (double)hc / (double)c2;
	}
	  
	if (c1 != 0) {
	  //	  cout << "(" << c1 << " " << hc << " " << (double)(c1 - hc) / (double)c1 << " " << *miss << ") ";

	  // *miss += (double)(c1 - hc) / (double)c1;
	  missCount  += c1 - hc;
	  totalCount += c1;
	}
	
	  
	// free those used labels
	for (int i=0; i < c1; i++)
	  delete lset1[i];
	for (int i=0; i < c2; i++)
	  delete lset2[i];
  }

  // cout << endl;

  *hit /= cc2;
  //  cout << *miss;
  *miss /= cc1;
  if (totalCount != 0)
	*miss = (double)missCount / (double)totalCount;
  *phit /= nSpc * 10;

  fclose(rf);
}


void oneBatch(char *batchName, 
			  int run, 
			  int size) {

  double hit[size], miss[size], agg[size];

  for (int i=0; i < size; i++) {
	char fn1[1000], fn2[1000];
	char wfn[1000], resfn[1000];	

	if (strcmp(batchName, "hybrid") == 0) {
	  sprintf(fn2, "%s/oriLbl_d_%d_%d", batchName, run, i);
	  sprintf(fn1, "%s/resLbl_d_%d_%d", batchName, run, i);
	  sprintf(wfn, "%s/aggLbl%d_%d", batchName, run, i);
	  sprintf(resfn, "%s/result%d_%d", batchName, run, i);

	} else {
	  sprintf(fn2, "%s/oriLbl_%d", batchName, i);
	  sprintf(fn1, "%s/resLbl_d_%d", batchName, i);
	  sprintf(wfn, "%s/aggLbl%d", batchName, i);
	  sprintf(resfn, "%s/result%d", batchName, i);
	}


	// cout << fn2 << endl;

	//	cout << "Test case " << i << endl;
	test(fn1, fn2, wfn, resfn, &(hit[i]), &(miss[i]), &(agg[i]));
	// cout << "Test end..." << endl;
  }
  
  cout << " Batch Name : " << batchName << run << endl;

  cout << " Hit : " << endl;
  for (int i=0; i < size; i++) printf("%8.4lf", hit[i]);
  printf("\n");

  cout << " Miss : " << endl;
  for (int i=0; i < size; i++) printf("%8.4lf", miss[i]);
  printf("\n");

  cout << " Aggregration accuracy : " << endl;
  for (int i=0; i < size; i++) printf("%8.4lf", agg[i]);
  printf("\n");

}

int main() {

  oneBatch("noise", 0, 10);

  oneBatch("ambiguity", 0, 5);

  oneBatch("unknown_inter", 0, 10);
  
  oneBatch("unknown_no_inter", 0, 10);

  for (int i=0; i<5; i++) 
	oneBatch("hybrid", i, 5);
}
