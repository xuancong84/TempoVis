#include "StdAfx.h"

#include <vector>
#include "parameters.h"
#include "TempoEst.h"
#include "VisualHelper.h"
#include "TempoVis1.h"
#include "opengl.h"

using namespace std;

const float delta_mul = 3.0f/(delta_width*(delta_width+1)*(2*delta_width+1));

float	*CreateVector( int size ){
	float	*v = new float [(size+1)*sizeof(float)];
	*(int*)v = size;
	return	v;
}
short	*CreateShortVector( int size ){
	short	*v = new short [(size+1)*sizeof(short)];
	*(short*)v = (short)size;
	return	v;
}

float Mel(int k,float fres){
	return 1127 * log(1 + (k-1)*fres);
}


void deltaSpec( float *spec, float *out, int size ){
	for( int x=0; x<size; x++ ){
		if( x+delta_width>=size || x-delta_width<0 ){
			out[x] = 0;
			continue;
		}
		float num = 0;
		for( int y=1; y<=delta_width; y++ ){
			num += (spec[x+y]-spec[x-y])*y;
		}
		out[x] = num*delta_mul;
		if(out[x]<0) out[x] = 0;
	}
}

FBankInfo InitFBank(int frameSize, long sampPeriod, int numChans,
					float lopass, float hipass, bool usePower, bool takeLogs )
{
	FBankInfo fb;
	float mlo,mhi,ms,melk;
	int k,chan,maxChan,Nby2;

	/* Save sizes to cross-check subsequent usage */
	fb.frameSize = frameSize; fb.numChans = numChans;
	fb.sampPeriod = sampPeriod;
	fb.usePower = usePower; fb.takeLogs = takeLogs;
	/* Calculate required FFT size */
	fb.fftN = 2;
	while (frameSize>fb.fftN) fb.fftN *= 2;
	Nby2 = fb.fftN / 2;
	fb.fres = 1.0e7f/(sampPeriod * fb.fftN * 700.0f);
	maxChan = numChans+1;
	/* set lo and hi pass cut offs if any */
	fb.klo = 2; fb.khi = Nby2;       /* apply lo/hi pass filtering */
	mlo = 0; mhi = Mel(Nby2+1,fb.fres);
	if (lopass>=0.0) {
		mlo = 1127.0f*log(1+lopass/700.0f);
		fb.klo = (int) ((lopass * sampPeriod * 1.0e-7 * fb.fftN) + 2.5);
		if (fb.klo<2) fb.klo = 2;
	}
	if (hipass>=0.0) {
		mhi = 1127.0f*log(1+hipass/700.0f);
		fb.khi = (int) ((hipass * sampPeriod * 1.0e-7 * fb.fftN) + 0.5);
		if (fb.khi>Nby2) fb.khi = Nby2;
	}

	/* Create vector of fbank centre frequencies */
	fb.cf = CreateVector(maxChan);
	ms = mhi - mlo;
	for (chan=1; chan <= maxChan; chan++) {
		fb.cf[chan] = ((float)chan/(float)maxChan)*ms + mlo;
	}

	/* Create loChan map, loChan[fftindex] -> lower channel index */
	fb.loChan = CreateShortVector(Nby2);
	for (k=1,chan=1; k<=Nby2; k++){
		melk = Mel(k,fb.fres);
		if (k<fb.klo || k>fb.khi) fb.loChan[k]=-1;
		else {
			while (fb.cf[chan] < melk  && chan<=maxChan) ++chan;
			fb.loChan[k] = chan-1;
		}
	}

	/* Create vector of lower channel weights */
	fb.loWt = CreateVector(Nby2);
	for (k=1; k<=Nby2; k++) {
		chan = fb.loChan[k];
		if (k<fb.klo || k>fb.khi) fb.loWt[k]=0.0;
		else {
			if (chan>0)
				fb.loWt[k] = ((fb.cf[chan+1] - Mel(k,fb.fres)) /
				(fb.cf[chan+1] - fb.cf[chan]));
			else
				fb.loWt[k] = (fb.cf[1]-Mel(k,fb.fres))/(fb.cf[1] - mlo);
		}
	}
	/* Create workspace for fft */
	fb.x = CreateVector(fb.fftN);
	return fb;
}

void Wave2FBank( float* s, float* fbank, float *te, FBankInfo& info )
{
   const float melfloor = 1.0;
   int k, bin;
   float t1,t2;   /* real and imag parts */
   float ek;      /* energy of k'th fft channel */
   float mul=(float)M_PI/(info.frameSize-1);

   /* Compute frame energy if needed */
   if (te != NULL){
      *te = 0.0;
      for (k=0; k<info.frameSize; k++)
         *te += (s[k]*s[k]);
   }
   /* Apply FFT */
   for (k=0; k<info.frameSize; k++)
      info.x[k+1] = s[k]*(sin(mul*k)+1);    /* copy to workspace */
   for (k=info.frameSize+1; k<=info.fftN; k++)
      info.x[k] = 0.0;   /* pad with zeroes */
   Realft(info.x);                            /* take fft */

   /* Fill filterbank channels */
   memset( &fbank[1], 0, *(int*)fbank*sizeof(float) );
   for (k = info.klo; k <= info.khi; k++) {             /* fill bins */
      t1 = info.x[2*k-1]; t2 = info.x[2*k];
      if (info.usePower)
         ek = t1*t1 + t2*t2;
      else
         ek = sqrt(t1*t1 + t2*t2);
      bin = info.loChan[k];
      t1 = info.loWt[k]*ek;
      if (bin>0) fbank[bin] += t1;
      if (bin<info.numChans) fbank[bin+1] += ek - t1;
   }

   /* Take logs */
   if (info.takeLogs)
      for (bin=1; bin<=info.numChans; bin++) {
         t1 = fbank[bin];
         if (t1<melfloor) t1 = melfloor;
         fbank[bin] = log(t1);
      }
}

FLOAT stddev(float *data, int size){
	double sum = 0, sum2 = 0;
	for (int x = 0; x < size; ++x){
		sum += data[x];
		sum2 += data[x] * data[x];
	}
	sum /= size, sum2 /= size;
	return sqrt(sum2 - sum*sum);
}

FLOAT mean(float *data, int size){
	double sum = 0;
	for (int x = 0; x < size; ++x)
		sum += data[x];
	return sum / size;
}

void zero_mean(float *data, int size){
	FLOAT m = mean(data, size);
	for (int x = 0; x < size; ++x)
		data[x] -= m;
}

FLOAT computeSpecWeight(float *data, int size){
	//return 1;
	//FLOAT a = stddev(data, size);
	FLOAT b = calcMaxPeakHeight(data, size);
	return b*b;
}

FLOAT computeSpecWeightForPeak(float *data, int size, int peak){
	//return 1;
	//FLOAT a = stddev(data, size);
	FLOAT b = calcPeakHeight(data, size, peak);
	return b*b;
}

void compressTowardsMean(FLOAT *data, int size, FLOAT factor){
	FLOAT m = mean(data, size);
	for (int x = 0; x < size; ++x)
		data[x] = m + (data[x] - m)*factor;
}

FLOAT cos_sim(vector <FLOAT> &v1, vector <FLOAT> &v2){
	FLOAT dot = 0, n1 = 0, n2 = 0;
	for (int x = 0; x < v1.size(); ++x){
		dot += v1[x] * v2[x];
		n1 += v1[x] * v1[x];
		n2 += v2[x] * v2[x];
	}
	return dot / sqrt(n1*n2);
}

vector <FLOAT> cosine_auto_corr(vector <vector <FLOAT>> &fbank, int max_shift){
	vector <FLOAT> autocorr(max_shift);
	for (int x = 0; x < max_shift; ++x){
		register double sum = 0;
		register int Y = fbank.size() - x;
		for (register int y=0; y < Y; ++y){
			sum += cos_sim(fbank[y], fbank[y + x]);
		}
		autocorr[x] = sum / Y;
	}
	return autocorr;
}

float ComputeTempo( float *data, int size, int sr, float &pri_tempo ){	//sr: sampling rate
	const int	winpts = (int)(window_size*sr+0.5);
	const int	hoppts = (int)(hop_size*sr+0.5);
	const int	fftpts = (int)pow(2.0,ceil(log((double)winpts)/log(2.0)));
	const int	nframe = (int)((double)(size-winpts)/hoppts+1.0);

	vector <vector <FLOAT>>	TempoCorr;
	vector <FLOAT>	TempoSpec(TempoMaxShift),
					TempoSpecP(TempoMaxShift),
					Tempo2WindowFunc(TempoMaxShift),
					Tempo3WindowFunc(TempoMaxShift);

	// Initialize filterbank
	FBankInfo fb_info = InitFBank( fftpts, (long)(1e7f/sr+0.5), nFilterBanks, 0.0f, 8000.0f, false, false );

	// Initialize window functions
	{
		FLOAT	f2 = 2*CTempoPeriod/hop_size;
		FLOAT	f3 = 3*CTempoPeriod/hop_size;
		for( int x=0; x<TempoMaxShift; x++ ){
			Tempo2WindowFunc[x]	= pow((FLOAT)M_E*x/f2,(FLOAT)f2/x)*x*(exp(-(FLOAT)M_E*x/f2) );
			Tempo3WindowFunc[x]	= pow((FLOAT)M_E*x/f2,(FLOAT)f3/x)*x*(exp(-(FLOAT)M_E*x/f3) );
			//ExpWindowFunc[x] = exp(-(FLOAT)x / (10*CTempoPeriod*TempoPrecision));
		}
		Tempo2WindowFunc[0] = Tempo3WindowFunc[0] = 0;
		normCorr(Tempo2WindowFunc.data(), TempoMaxShift);
		//compressTowardsMean(Tempo2WindowFunc, TempoMaxShift);
		normCorr(Tempo3WindowFunc.data(), TempoMaxShift);
		//compressTowardsMean(Tempo3WindowFunc, TempoMaxShift);
		//compressTowardsMean(ExpWindowFunc, TempoMaxShift);
	}

	// Allocate dynamic buffers
	float	fbank[nFilterBanks+1];
	vector <vector<FLOAT>> AllBuf(nFilterBanks+1, vector<FLOAT>(nframe));
	vector <FLOAT> DifBuf(nframe);
	vector <FLOAT> fdata(fftpts);

	// Extract all frames
	zero_mean(data, size);
	vector <vector <FLOAT>> fbs(nframe);
	*(int*)fbank = nFilterBanks;
	for( int x=0; x<nframe; x++ ){
		memcpy( fdata.data(), &data[x*hoppts], winpts*sizeof(float) );

		// get filterbank and energy
		Wave2FBank( fdata.data(), fbank, &AllBuf[0][x], fb_info );
		for( int y=1; y<=nFilterBanks; y++ )
			AllBuf[y][x] = fbank[y];
	}

	{// extract filter bank and normalize energy w.r.t. filter bank
		double sum_e = 0, sum_fb = 0;
		for (int x = 0; x < nframe; ++x){
			vector <FLOAT> fb(nFilterBanks + 1);
			for (int y = 0; y <= nFilterBanks; ++y)
				fb[y] = AllBuf[y][x];
			fbs[x] = fb;
			sum_e += fb[0] * fb[0];
			for (int y = 1; y < nFilterBanks; ++y)
				sum_fb += fb[y] * fb[y];
		}
		FLOAT mul = sqrt(sum_fb / sum_e);
		for (int x = 0; x < nframe; ++x)
			fbs[x][0] *= mul;
	}

	// Process features
	for (int x = 0; x<AllBuf.size(); x++){
		vector <FLOAT> buf(TempoMaxShift);
		FLOAT weight;

		autoCorr( buf.data(), AllBuf[x].data(), TempoMaxShift, nframe );
		weight = computeSpecWeight(buf.data(), TempoMaxShift);
		addCorr(TempoSpec.data(), buf.data(), TempoMaxShift, weight );
		TempoCorr.push_back(buf);
		est_fact.push_back(weight);

		// add delta spectrum
		deltaSpec( AllBuf[x].data(), DifBuf.data(), nframe );
		AllBuf[x] = DifBuf;
		autoCorr(buf.data(), AllBuf[x].data(), TempoMaxShift, nframe);
		weight = computeSpecWeight(buf.data(), TempoMaxShift);
		addCorr(TempoSpec.data(), buf.data(), TempoMaxShift, weight );
		TempoCorr.push_back(buf);
		est_fact.push_back(weight);

		// add delta-delta spectrum
		deltaSpec( AllBuf[x].data(), DifBuf.data(), nframe );
		AllBuf[x] = DifBuf;
		autoCorr( buf.data(), AllBuf[x].data(), TempoMaxShift, nframe );
		weight = computeSpecWeight(buf.data(), TempoMaxShift);
		addCorr(TempoSpec.data(), buf.data(), TempoMaxShift, weight );
		TempoCorr.push_back(buf);
		est_fact.push_back(weight);
	}
	est_spec = TempoCorr;
	est_spec.push_back(TempoSpec);

	vector <FLOAT> cos_ac = cosine_auto_corr(fbs, TempoMaxShift);
	est_spec.push_back(cos_ac);

	// find the positions of all peaks
	getPeakSpectrum(TempoSpecP.data(), TempoSpec.data(), TempoMaxShift);
	
	// Obtain primary tempo peak, the most important step
	int	itempo = findBestTempoPeak(TempoSpecP.data(), TempoMaxShift);
	pri_tempo = itempo + interPeakPosi(&TempoSpec[itempo]);

	// Select correlation spectrums that are more relevant to this peak and re-weight
	memset(TempoSpec.data(), 0, sizeof(FLOAT)*TempoMaxShift);
	for (int x = 0; x<TempoCorr.size(); x++){
		FLOAT weight = computeSpecWeightForPeak(TempoCorr[x].data(), TempoMaxShift, itempo);
		addCorr(TempoSpec.data(), TempoCorr[x].data(), TempoMaxShift, weight);
		est_fact2.push_back(weight);
	}

	//suppress the edge effect of correlation spectrum for tempo adjustment
	//ExpUpperBound(TempoSpec.data(), TempoMaxShift);

	getPeakSpectrum(TempoSpecP.data(), TempoSpec.data(), cos_ac.data(), TempoMaxShift);

	// new method: find all acceptable peaks, then apply the window function
	itempo	=	adjustTempo(pri_tempo, (int)(TempoMinPeriod/hop_size+0.5),
							TempoMaxShift, TempoSpecP.data(), TempoSpecP.data(),
							Tempo2WindowFunc.data(), Tempo3WindowFunc.data());

	// for display purposes
	est_spec.push_back(TempoSpecP);

	float	tempo = itempo+interPeakPosi(&TempoSpec[itempo]);
	bool	bAmbi = false;
	if (getInnerMeter(tempo, TempoSpec.data(), TempoMaxShift) == 3) if (!bAmbi) tempo = -tempo;
	return	(float)hop_size*tempo;
}

