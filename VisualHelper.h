#pragma once

#include "parameters.h"

class	LoopBuffer{
	// All variables set to public for performance
public:
	int		N_total_frames;
	int		N_draw_frames;
	int		N_valid_frames;
	int		frame_size;
	int		current_frame;		// inclusive
	int		draw_size;
	int		data_size;
	int		status;				// 0:playing or stopped, 1:paused, 2:unpaused but buffering
	FLOAT	time_factor;
	FLOAT	time_stamp;
	char	*data;				// main data buffer
	char	*last_frame_data;

	LoopBuffer(int _N_total_frames, int _N_draw_frames, int _frame_size, FLOAT _fps);
	~LoopBuffer();

	// Functions
	int	AddFrame(FLOAT timeStamp, FLOAT *currFrame);
	int AddFrame(FLOAT timeStamp, FLOAT val);
	void	ResizeBuffer(int _N_total_frames, int _N_draw_frames, int _frame_size, FLOAT _fps);
	void	Reset(FLOAT _time_stamp);
	FLOAT*	getCurrentPtrFront();
	FLOAT*	getCurrentPtrBack();
};

class	Camera{
public:
	GLwindow *GLmain;
	FLOAT	eyev[3], centerv[3], upv[3], rightv[3];
	FLOAT	viewAngle;
	FLOAT	viewMat[16], projMat[16];

	Camera(GLwindow *pWin);
	~Camera();

	void reset();
	void setCamera();
	void rotateHV(FLOAT h, FLOAT v);
	void updateCamera(FLOAT *eyev, FLOAT *objv, FLOAT *topv);
	void shiftUp(FLOAT dist);
};

inline FLOAT *VectorCopy(FLOAT *vDst, FLOAT *vSrc);
void	Transpose(char *pDst, char *pSrc, int rows, int cols, int eachSize);
void	expUpdate(FLOAT *data, FLOAT *newdata, int size, FLOAT prev_time, FLOAT curr_time, FLOAT half_life);
inline void	RandomBoundVector(FLOAT *vec);
inline void	RandomDirnVector(FLOAT *vec);
inline DWORD RandomColorNorm();
inline DWORD RandomColor(int intense = 0);
inline DWORD InterColor(DWORD c1, DWORD c2, float s_factor);
inline FLOAT PhaseDiff(FLOAT f1, FLOAT f2);
inline int PhaseDiff(int v1, int v2, int period);
inline FLOAT getMin(FLOAT *data, int size);
inline FLOAT getMax(FLOAT *data, int size);
inline FLOAT findMinNon0(FLOAT *data, int size);
inline FLOAT findPeakHeight(FLOAT *data, int size, int posi);
inline int findPeakPosi(FLOAT *data, int size, int posi);
inline FLOAT calcPeakHeight(FLOAT *data, int size, FLOAT posi);
inline int findMax(FLOAT *data, int size){
	int	posi = 0;
	register FLOAT	minV = -FLT_MAX;
	for (int x = 0; x<size; x++){
		if (data[x]>minV){
			minV = data[x];
			posi = x;
		}
	}
	return	posi;
}// Obtain maximum point
inline int checkTempoPeak(FLOAT *data, int size, int peak){
	FLOAT p1 = peak / 2, p2 = peak / 3;
	if (	abs(findPeakPosi(data, size, p1) - p1) / p1 >TempoPeakSharp
		&&	abs(findPeakPosi(data, size, p2) - p2) / p2 >TempoPeakSharp)
		return 1;
	return 2;
}
inline int findBestTempoPeak(FLOAT *data, int size){
	int	posi = 0;
	register FLOAT	minV = -FLT_MAX;
	for (int x = size/100, X = size*99/100; x<X; x++){
		if (data[x]){
			FLOAT data_val = checkTempoPeak(data, size, x)*data[x];
			if (data_val > minV){
				minV = data_val;
				posi = x;
			}
		}
	}
	return	posi;
}// Obtain maximum point from 1/10 to 9/10, due to correlation spectrum edge effect

FLOAT* resizeData(int srcWidth, int srcHeight, FLOAT *srcData, int dstWidth, int dstHeight, FLOAT *dstData);
int	getInnerMeter(FLOAT pri_tempo, FLOAT *spec, int size, int *bAmbiguous = NULL);
int	getOuterMeter(FLOAT pri_tempo, FLOAT *spec, int size, int *bAmbiguous = NULL);
bool testLower(FLOAT pri_tempo, FLOAT lo_tempo, FLOAT *TempoSpec, int size);
bool testUpper(FLOAT pri_tempo, FLOAT hi_tempo, FLOAT *TempoSpec, int size);
void	DrawSpikeArray(FLOAT*vals, int size, float Vstart, float Vend, DWORD color = 0xffffffff, bool zeroMin = false);
void	DrawPointArray(FLOAT*vals, int size, float Vstart, float Vend, DWORD color = 0xffffffff, bool zeroMin = false);
void FFT(float* s, int invert);
void Realft(float* s);
void HV_rotation(FLOAT h, FLOAT v);
void HV_shift(FLOAT h, FLOAT v);

using namespace std;
inline void	RandomBoundVector(FLOAT *vec){
	do{
		vec[0] = (FLOAT)rand() / RAND_MAX - 0.5f;
		vec[1] = (FLOAT)rand() / RAND_MAX - 0.5f;
		vec[2] = (FLOAT)rand() / RAND_MAX - 0.5f;
		VectorMul(vec, 2.0f, vec);
	} while (VectorLength(vec) > 1.0f);
}// random a unit cube bounded vector {x:[-1,1];y:[-1,1];z:[-1,1]}

inline void	RandomDirnVector(FLOAT *vec){
	do{
		vec[0] = (FLOAT)rand() / RAND_MAX - 0.5f;
		vec[1] = (FLOAT)rand() / RAND_MAX - 0.5f;
		vec[2] = (FLOAT)rand() / RAND_MAX - 0.5f;
	} while (VectorLength(vec) < 1e-4f);
	VectorNorm(vec);
}// random a unit length direction vector

inline DWORD RandomColorNorm(){
	FLOAT	colors[3] = { (FLOAT)rand() / RAND_MAX, (FLOAT)rand() / RAND_MAX, (FLOAT)rand() / RAND_MAX };
	VectorNorm(colors);
	return	(((DWORD)(colors[0] * 255 + 0.5f)) << 16) | (((DWORD)(colors[1] * 255 + 0.5f)) << 8) | ((DWORD)(colors[2] * 255 + 0.5f));
}// generate a random color with unit intensity

inline DWORD RandomColor(int intense){
	FLOAT	mul = 255.0f / RAND_MAX;
	DWORD	color;
re:
	color = (((DWORD)(rand()*mul + 0.5f)) << 16) | (((DWORD)(rand()*mul + 0.5f)) << 8) | ((DWORD)(rand()*mul + 0.5f));
	if (intense){
		if (intense == 1) return	RandomColorNorm();
		BYTE r = (BYTE)(color & 0xff), g = (BYTE)((color >> 8) & 0xff), b = (BYTE)((color >> 16) & 0xff);
		BYTE m = max(r, g);
		m = max(m, b);
		if (!m) goto re;
		mul = 255.0f / m;
		color = (((DWORD)(b*mul + 0.5f)) << 16) | (((DWORD)(g*mul + 0.5f)) << 8) | ((DWORD)(r*mul + 0.5f));
	}// stretch intensity to max
	return	color;
}// generate a random color*/

inline DWORD InterColor(DWORD c1, DWORD c2, float s_factor){
	register BYTE *colors = (BYTE*)&c1;
	FLOAT	cs1[4] = { colors[0], colors[1], colors[2], colors[3] };
	colors = (BYTE*)&c2;
	FLOAT	cs2[4] = { colors[0], colors[1], colors[2], colors[3] };
	Vector4Inter(cs1, cs2, s_factor, cs1);
	Vector4Add(cs1, 0.5f, cs1);
	return	(((DWORD)cs1[3]) << 24) | (((DWORD)cs1[2]) << 16) | (((DWORD)cs1[1]) << 8) | ((DWORD)cs1[0]);
}// Interpolate color, including alpha

inline FLOAT PhaseDiff(FLOAT f1, FLOAT f2){
	register FLOAT df = abs(f1 - f2);
	return	min(df, (FLOAT)M_2PI - df);
}// absolute phase difference

inline int PhaseDiff(int v1, int v2, int period){
	register int phaseDiff = v2 - v1;
	if (phaseDiff > 0) return phaseDiff > (period >> 1) ? period - phaseDiff : phaseDiff;
	else return (-phaseDiff) > (period >> 1) ? period + phaseDiff : phaseDiff;
}// return discrete phase difference from 0 to period

inline FLOAT getMin(FLOAT *data, int size){
	register FLOAT minV = *data;
	register FLOAT *pData = data, *pEnd = &data[size];
	for (; pData < pEnd; pData++){
		if (*pData < minV) minV = *pData;
	}
	return	minV;
}// Obtain minimum point

inline FLOAT getMax(FLOAT *data, int size){
	register FLOAT maxV = *data;
	register FLOAT *pData = data, *pEnd = &data[size];
	for (; pData < pEnd; pData++){
		if (*pData > maxV) maxV = *pData;
	}
	return	maxV;
}// Obtain maximum value

inline FLOAT findMinNon0(FLOAT *data, int size){
	register FLOAT	minV = FLT_MAX;
	for (int x = 0; x < size; x++){
		register FLOAT fval = data[x];
		if (fval) if (fval < minV) minV = fval;
	}
	return	(minV == FLT_MAX) ? 0 : minV;
}// Obtain minimum non-zero point

inline FLOAT findPeakHeight(FLOAT *data, int size, int posi){
	register FLOAT	hLeft, hRight;
	register int	x;
	for (x = posi; x >= 0 && data[x - 1] < data[x]; x--);
	if ((hLeft = data[posi] - data[x]) <= FLT_MIN) return 0;
	for (x = posi; x < size && data[x + 1] < data[x]; x++);
	if ((hRight = data[posi] - data[x]) <= FLT_MIN)return 0;
	return	1 / (1 / hLeft + 1 / hRight);
}// Obtain the height and width of a peak

/*
inline int findPeakPosi(FLOAT *data, int size, int posi){
	int iLeft, iRight;
	register int x;
	assert(posi>=0&&posi < size);
	for (x = posi; x >= 0 && data[x - 1] > data[x]; x--);
	iLeft = (x == posi ? 0 : x);
	for (x = posi; x<size && data[x + 1]>data[x]; x++);
	iRight = (x == posi ? size : x);
	if (!iLeft && iRight == size) return	posi;
	int ret = ((posi - iLeft) < (iRight - posi)) ? iLeft : iRight;
	assert(ret>=0&&ret < size);
	return ret;
}// Obtain the position of closest peak
*/

inline int findPeakPosi(FLOAT *data, int size, int posi){
	int iLeft, iRight;
	if (posi < 0) posi = 0;
	if (posi >= size) posi = size - 1;
	for (int x = 0, X = max(posi, size - posi); x < X; ++x){
		int p = posi + x;
		if (p > 0 && p<size-1 && data[p]>data[p - 1] && data[p] > data[p + 1])
			return p;
		p = posi - x;
		if (p > 0 && p<size-1 && data[p]>data[p - 1] && data[p] > data[p + 1])
			return p;
	}
	return size/2;
}// Obtain the position of closest peak

inline int findPeakPosiWrap(FLOAT *data, int size, int posi){
	int iLeft, iRight;
	if (posi < 0) posi = 0;
	if (posi >= size) posi = size - 1;
	for (int x = 0, X = size>>1; x < X; ++x){
		int p = (posi + x) % size;
		if (p > 0 && p<size-1 && data[p]>data[p - 1] && data[p] > data[p + 1])
			return p;
		p = (posi - x + size) % size;
		if (p > 0 && p<size-1 && data[p]>data[p - 1] && data[p] > data[p + 1])
			return p;
	}
	return size/2;
}// Obtain the position of closest peak

inline FLOAT calcPeakHeight(FLOAT *data, int size, FLOAT posi){
	if (posi >= size || posi<=0)
		return 0;
	int x = findPeakPosi(data, size, (int)(posi + 0.5f));
	register FLOAT y = (max(abs(posi-x)-1, 0) / (posi*TempoPeakSharp));
	return	findPeakHeight(data, size, x)*exp(-y*y);
}// Obtain normalized peak height value

inline void normCorr(FLOAT *data, int size, FLOAT value=1.0f){
	FLOAT	maxV = getMax(data, size);

	if (maxV == 0) return;
	FLOAT	scale = value / maxV;
	for (int x = 0; x<size; x++){
		data[x] = max(0, data[x] * scale);
	}
	//CheckFloat(data,size);
}// Normalize autocorrelation

inline void normCorrNon0(FLOAT *data, int size, FLOAT value=1.0f){
	register FLOAT	maxV = getMax(data, size);
	register FLOAT	minV = findMinNon0(data, size);

	if (maxV == minV) return;
	register FLOAT	scale = (FLOAT)value / (maxV - minV);
	for (register int x = 0; x<size; x++){
		register FLOAT fval = (data[x] - minV)*scale;
		data[x] = fval<0 ? 0 : fval;
	}
	//CheckFloat(data,size);
}// Normalize autocorrelation

inline FLOAT* autoCorr(float *data_out, float *data_in, int max_shift, int data_size){
	if (max_shift>data_size){
		memset(data_out, 0, sizeof(float)*max_shift);
		max_shift = data_size;
	}
	for (int x = 0; x<max_shift; x++){
		register float	*data_in2 = &data_in[x];
		register float	sum = 0;
		// Do not use the entire residue buffer of size (data_size - x) to avoid aliasing
		//register int	I = (x == 0 ? data_size : (data_size - x) / x * x);
		register int	I = data_size - x;
		register int	i = 0;
/*
		{// this block of AVX optimization code can be commented out if CPU/compiler does not support
			__m256 ymm0 = _mm256_setzero_ps();
			for (; i + 7 < I; i += 8){
				__m256 ymm1 = _mm256_loadu_ps(&data_in[i]);
				__m256 ymm2 = _mm256_loadu_ps(&data_in2[i]);
				__m256 ymm3 = _mm256_mul_ps(ymm1, ymm2);
				ymm0 = _mm256_add_ps(ymm0, ymm3);
			}
			float float8[8];
			__m256 ymm1 = _mm256_setzero_ps();
			ymm0 = _mm256_hadd_ps(ymm0, ymm1);
			ymm0 = _mm256_hadd_ps(ymm0, ymm1);
			_mm256_storeu_ps(float8, ymm0);
			sum = float8[0] + float8[4];
		}
*/
		for (; i<I; ++i)
			sum += data_in[i]*data_in2[i];

		data_out[x] = sum / I;
	}

	// normalize all values into 0 and 1 range
	normCorr(data_out, max_shift);

	return	data_out;
	//CheckFloat( data_out, max_shift );
}// Compute autocorrelation

inline void ExpUpperBound(float *data, int size){
	normCorr(data, size);

	// find k so that exp(-kx) upper bounds the auto-correlation spectrum
	float k = 1.0f;
	for (int x = 1; x < size; ++x){
		if (exp(-k*x) < data[x])
			k = -log(data[x]) / x;
	}

	// negate the negative exponential decay
	k /= exp(1);
	for (int x = 0; x < size; ++x)
		data[x] /= exp(-k*x);
}

inline void addCorr(FLOAT *dst, FLOAT *src, int size, FLOAT factor){
	//if( !CheckFloat( &factor ) ) return;
	if (factor == 1){
		for (register int x = 0; x < size; x++)
			dst[x] += src[x] * src[x];
	}else{
		for (register int x = 0; x<size; x++)
			dst[x] += src[x] * src[x] * factor;
	}
}// Add correlation spectrum

inline FLOAT calcMaxPeakHeight(FLOAT *data, int size){
	vector <FLOAT> left_min(size), right_min(size);	// min value to the left/right of current position
	FLOAT min_val = data[1];
	for (int x = 0; x < size; ++x){
		left_min[x] = min_val;
		min_val = min(min_val, data[x]);
	}
	min_val = data[size - 1];
	for (int x = size - 1; x >= 0; --x){
		right_min[x] = min_val;
		min_val = min(min_val, data[x]);
	}
	FLOAT	maxPeakHeight = 0;
	for (int x = 2; x<size-1; ++x){
		if (data[x]>data[x - 1] && data[x] > data[x + 1]){
			FLOAT left_height = data[x] - left_min[x];
			FLOAT right_height = data[x] - right_min[x];
			FLOAT peak_height = (left_height == 0 || right_height == 0) ?
				0 : 2.0 / (1.0/left_height+1.0/right_height);
			if (peak_height > maxPeakHeight)
				maxPeakHeight = peak_height;
		}
	}
	return	maxPeakHeight;
}

inline FLOAT calcPeakHeight(FLOAT *data, int size, int peak){
	peak = findPeakPosi(data, size, peak);
	FLOAT left_min=data[peak], right_min=data[peak];
	for (int x = peak; x < size; ++x){
		right_min = min(right_min, data[x]);
	}
	for (int x = peak; x >= 0; --x){
		left_min = min(left_min, data[x]);
	}
	FLOAT left_height = data[peak] - left_min;
	FLOAT right_height = data[peak] - right_min;
	FLOAT peak_height = (left_height == 0 || right_height == 0) ?
		0 : 2.0 / (1.0 / left_height + 1.0 / right_height);
	return	peak_height;
}

inline void getPeakSpectrum(FLOAT *dst, FLOAT *src, int size){
	FLOAT th = calcMaxPeakHeight(src, size)*0.25;
	memset(dst, 0, sizeof(FLOAT)*size);
	for (register int x = 1, y = size - 1; x<y; x++)
		if (src[x]>src[x - 1] && src[x]>src[x + 1])
			if (calcPeakHeight(src,size,x) > th)
				dst[x] = src[x];
}// Get peak spectrum, non-peak position = 0

inline void getPeakSpectrum(FLOAT *dst, FLOAT *src_peak, FLOAT *src_value, int size){
	FLOAT th = calcMaxPeakHeight(src_peak, size)*0.25;
	memset(dst, 0, sizeof(FLOAT)*size);
	for (register int x = 1, y = size - 1; x<y; x++)
		if (src_peak[x]>src_peak[x - 1] && src_peak[x] > src_peak[x + 1])
			if (calcPeakHeight(src_peak,size,x) > th)
				dst[x] = src_value[x];
}// Get peak spectrum, non-peak position = 0

inline FLOAT interPeakPosi(FLOAT *middle){
	FLOAT	left = *middle - *(middle - 1);
	FLOAT	right = *middle - *(middle + 1);
	FLOAT	sum = left + right;
	if (abs(sum) <= FLT_MIN) return 0;
	if (left<0) return	-0.5f;
	if (right<0) return 0.5f;
	return	(FLOAT)(left / sum - 0.5);
}// interpolate peak position

inline bool	isIntegerMultiple(int i1, int i2){
	if (!i1 || !i2) return	false;
	if (i2 > i1) swap(i1, i2);
	int		quot = (int)((float)i1 / i2 + 0.5f);
	float	f2 = (float)i1 / quot;
	return	abs(f2 - i2) / (f2 + i2) < 0.0625f / sqrt((FLOAT)quot);
}// Whether i1(i2) is an integer multiple of i2(i1)

inline bool	isHarmonicMultiple(int i1, int i2){
	if (!i1 || !i2) return	false;
	if (i2 > i1) swap(i1, i2);
	int		quot = (int)((float)i1 / i2 + 0.5f);
	float	f2 = (float)i1 / quot;
	if (abs(f2 - i2) / (f2 + i2) > 0.0625f / sqrt((FLOAT)quot)) return	false;
	while (!(quot & 1)) quot >>= 1;
	while (!(quot % 3)) quot /= 3;
	return	quot == 1;
}// Whether i1(i2) is an harmonic integer (no factor other than 2&3) multiple of i2(i1)


inline int	adjustTempo1(float pri_tempo, int min_tempo, int max_tempo,
	FLOAT *TempoSpec, FLOAT *PeakSpec, FLOAT *Window2, FLOAT *Window3){
	assert(pri_tempo > 0 && pri_tempo < max_tempo);

	float	best_tempo = pri_tempo;
	float	new_val, cur_val = TempoSpec[(int)(pri_tempo + 0.5f)] *
		(getInnerMeter(pri_tempo, TempoSpec, max_tempo) == 2 ? Window2[(int)(pri_tempo + 0.5f)] : Window3[(int)(pri_tempo + 0.5f)]);

	//float	window = getMeter(pri_tempo,PeakSpec,max_tempo)==2?Window2[pri_tempo]:Window3[pri_tempo];
	for (int x = min_tempo, y = (int)(pri_tempo*0.5625f); x <= y; x++){
		if (!PeakSpec[x]) continue;
		if (!isHarmonicMultiple((int)(pri_tempo + 0.5f), x)) continue;
		new_val = (getInnerMeter((FLOAT)x, TempoSpec, max_tempo) == 2 ? Window2[x] : Window3[x])*TempoSpec[x];
		if (new_val > cur_val){
			if (!testLower((FLOAT)pri_tempo, (FLOAT)x, TempoSpec, max_tempo)) continue;
			cur_val = new_val;
			best_tempo = (FLOAT)x;
		}
	}
	for (int x = (int)(pri_tempo*1.875f); x < max_tempo; x++){
		if (!PeakSpec[x]) continue;
		if (!isHarmonicMultiple(x, (int)(pri_tempo + 0.5f))) continue;
		new_val = (getInnerMeter((FLOAT)x, TempoSpec, max_tempo) == 2 ? Window2[x] : Window3[x])*TempoSpec[x];
		if (new_val > cur_val){
			if (!testUpper((FLOAT)pri_tempo, (FLOAT)x, TempoSpec, max_tempo)) continue;
			cur_val = new_val;
			best_tempo = (FLOAT)x;
		}
	}

	return	(int)(best_tempo + 0.5f);
}// adjust tempo index according to perceptual window


inline int Round(double v){
	return (int)round(v);
}
inline int	adjustTempo(float pri_tempo, int min_tempo, int max_tempo,
	FLOAT	*TempoSpec, FLOAT *PeakSpec, FLOAT *Window2, FLOAT *Window3){
	if (pri_tempo < 0)
		return -1;

	vector <FLOAT> valid_tempos(1, Round(pri_tempo));

	// search inwards
	for (FLOAT cur_tempo = pri_tempo; (cur_tempo /= getInnerMeter(cur_tempo, TempoSpec, max_tempo)) >= min_tempo;
		valid_tempos.push_back(cur_tempo));

	// search outwards
	for (FLOAT cur_tempo = pri_tempo; (cur_tempo *= getOuterMeter(cur_tempo, TempoSpec, max_tempo)) < max_tempo;
		valid_tempos.push_back(cur_tempo));

	// determine the best position by applying the window function
	FLOAT	best_score = 0;
	int		best_tempo = 0;
	for (FLOAT cur_tempo : valid_tempos){
		int		ipeak = findPeakPosi(TempoSpec, max_tempo, Round(cur_tempo));
		FLOAT	score = calcPeakHeight(TempoSpec, max_tempo, cur_tempo)
			*(getInnerMeter(cur_tempo, TempoSpec, max_tempo) == 3 ? Window3[ipeak] : Window2[ipeak]);
		if (score > best_score){
			best_score = score;
			best_tempo = ipeak;
		}
	}

	return	best_tempo;
}// adjust tempo index according to perceptual window
