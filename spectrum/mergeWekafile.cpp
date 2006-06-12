#include <stdio.h>
#include <iostream>

using namespace std;


int main(int argc, char* argv[]) {
  cout << argv[0] << " " << argc << endl;
  FILE *file = fopen(argv[1], "a");

  for (int i=2; i < argc; i++) {
	FILE *nf = fopen(argv[i], "r");
	char buff[10000];
	char *res = fgets(buff, 10000, nf);
	while (res != NULL) {
	  cout << res << endl;
	  if ((buff[0] != 0) && 
          (buff[0] != '%') &&
	      (buff[0] != '@') &&
		  (buff[0] != '\n')) {
		fprintf(file, "%s", buff);
	  }
	  res = fgets(buff, 10000, nf);
	}
	fclose(nf);
  }

  fclose(file);
}
