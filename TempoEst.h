#pragma once

#include "parameters.h"

extern const float delta_mul;

typedef struct{
	int		frameSize;       	/* speech frameSize */
	int		numChans;        	/* number of channels */
	long		sampPeriod;			/* sample period */
	int		fftN;            	/* fft size */
	int		klo, khi;         	/* lopass to hipass cut-off fft indices */
	bool		usePower;    		/* use power rather than magnitude */
	bool		takeLogs;    		/* log filterbank channels */
	float	fres;				/* scaled fft resolution */
	float* 	cf;          	 	/* array[1..pOrder+1] of centre freqs */
	short* 	loChan;				/* array[1..fftN/2] of loChan index */
	float* 	loWt;				/* array[1..fftN/2] of loChan weighting */
	float* 	x;					/* array[1..fftN] of fftchans */
}FBankInfo;

float	*CreateVector(int size);
short	*CreateShortVector(int size);

float Mel(int k, float fres);


void deltaSpec(float *spec, float *out, int size);

FBankInfo InitFBank(int frameSize, long sampPeriod, int numChans,
	float lopass, float hipass, bool usePower, bool takeLogs);

void Wave2FBank(float* s, float* fbank, float *te, FBankInfo& info);

FLOAT stddev(float *data, int size);

FLOAT mean(float *data, int size);

FLOAT computeSpecWeight(float *data, int size);

void compressTowardsMean(FLOAT *data, int size, FLOAT factor=0.5f);

float ComputeTempo(float *data, int size, int sr, float &pri_tempo);

