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

#ifndef DATASET_GENERATOR_H
#define DATASET_GENERATOR_H

#include <Signature.h>

class DatasetGenerator {
	public :
		DatasetGenerator() {};
		
		void DatasetGenerator::generateMoreSignatures(const char *sigFileName,
					      const char *unknownSigFileName,
					      const char *newSigFileName,
					      int newNumSigs);
					      
		void generateSignature(const char *elementSignatureFileName,  // filename of the element signature 
				       int numUnknownSignatures, // number of unknown Signatures in the signature database
				       int numGroups,
				       const char *outputSignatureFileName, // signature filename, the filename for weka format has extra .weka extension
				       const char *outputUnknownSignatureFileName // unknown signature filename
				       );
		void generateSpectra(const char *signatureFileName,
				     const char *unknownSignatureFileName,				 
				     int numSpectra, // number of spectra in the testing date set
				     int numSigPerSpectrum, // number of signatures per spectrum
				     int numUnknownSigPerSpectrum, // number of unknown signature in every spectrum
				     int numInfPerSpectrum,  // The degree of ambiguity
				     int numDimWithNoise,  // Number of dimensions which have noise
				     int useInterUnknownSig,	// Whether the unknown signature will interfere 
				     				// with other signatures.
				     int numGroups,
				     double intensityOfNoise,  // the average intensity of the noise;
				     double varianceOfNoise	, // the variance of the noise
				     const double *signatureWeight,  // The weight vector
				     const double *signatureWeightVariance, // the variance vector of the weight
				     int tileSize,
				     const double *tile,
				     const char *outputSpectrumFileName,  // SpectraSet file name, the filename for weka format has extra .weka extension
				     const char *outputResultFileName,
				     const char *outputLabelFileName); // The desirable result for each spectrum
	private :
		
		double normal(double m, double s);
		
		// File name of the element signatures
		const char *eleFn;
		
		// number of total signatures
		int nSig;
		
		int nuSig;
		
		// average number of signatures per spectrum
		int nSigS;
		
		// number of unknown signature per spectrum
		int nuSigS;
		
		// average number of Interference per spectrum
		int nInfS;
		
		// average number of dimensions which have noise
		int nDim;
		
		// the average intensity of the noise
		double iNoise;
		
		double vNoise;
		
		// Number of spectra in the dataset
		int nSpc;
		
		// weight of each signature in the spectrum
		const double *weight;
		
		const double *var;
		
		const char * oSigFn;
		
		const char * ouSigFn;
		
		const char * oSpcFn;
		
		const char * oResFn;
		
		int freeC;
		int unknownC;
		int nGroups;
		
		Signature ** sigGroup[100];
		
		Signature ** freeGroup;
		
		Signature ** unknownGroup;
		
};
	
#endif
