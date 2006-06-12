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

#include <ManLabelTester.h>

#include <Signature.h>
#include <SignatureDatabase.h>
#include <SignatureDatabaseReader.h>

#include <Spectrum.h>
#include <SpectrumReader.h>

#include <ManhattenDistanceLabelFinder.h>

#include <iostream>
using namespace std;

void ManLabelTester::test1() {
  SignatureDatabaseReader sd_reader("sig.txt", SIG_SIMPLE_TEXT);
  SpectrumReader s_reader("spc.txt", SPC_SIMPLE_TEXT);

  cout << "Read signature database." << endl;
  
  SignatureDatabase *sd = 
	sd_reader.getSignatureDatabase();

  cout << "Finish reading signature database." << endl;

  ManhattenDistanceLabelFinder labeler(*sd);

  //  labeler.dump();

  Spectrum *ps = s_reader.nextSpectrum();

  while (ps != NULL) {
	QuantityLabel *label = labeler.solve(ps);
	
	cout << "Error : " << label->getError() << endl;
	
	double * data = label->getData();

	for (int i=0; i < label->getSize(); i++) 
	  cout << data[i] << " ";

	cout << endl;
	
	ps = s_reader.nextSpectrum();
  }
}
