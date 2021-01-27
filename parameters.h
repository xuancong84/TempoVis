#pragma once

#define		_USE_MATH_DEFINES
#define		__DRAWFAST 1
#include	<math.h>
#include	<assert.h>
#include	<mmsystem.h>
#include	<mmreg.h>
#include	<msacm.h>
#include	<math.h>
#include	<iostream>
#include	<string>
#include	<fstream>
#include	<float.h>
#include	<queue>
#include	<map>
#include	<cmath>
#include	"opengl.h"
#include	"glext.h"
#include	"gl\glu.h"
#include	"effects.h"

// Extra math constants
#define	M_2PI				6.283185307179586476925286766559
#define	M_LOG_2				0.69314718055994530941723212145818

// System Parameters
#define	FFTSIZE			1024
#define	FFTSIZE_2		512
#define	FFTSIZE_4		256
#define	FFTMAX			255
#define	WAVMAX			127
#define	WAVSIZE			1024
#define	WAVSIZE2		2048
#define	WAVSIZE4		4096
#define	WAVSIZE_2		512
#define	FFTHIST			1024
#define	FLOAT			float
#define	RecBufSize		441000				// recording buffer size in BYTES
#define	MINWAVLENGTH	16					// min duration of wave in second for tempo estimation
#define	MAXWAVLENGTH	1024				// max duration of wave in second for tempo estimation
#define	RECORDERSR		44100

// Dynamic tempo-est parameters
#define	TempoPrecision			120																	// ticks per second
#define	TempoBufferLength		8																	// seconds
#define	TempoMinBufferSize		2																	// seconds
#define	TempoStableTime			4.0f																// in seconds
#define	TempoMaxPeriod			8.0f																// in seconds, 10 bpm (exclusive)
#define	TempoMinPeriod			0.2f																// in seconds, 300 bpm(inclusive)
#define	TempoHalfLife			16																	// seconds (tempo pattern memory)
#define	PhaseHalfLife			32																	// seconds (phase pattern memory)
#define	TempoMaxShift			(int)(TempoPrecision*TempoMaxPeriod+0.5)
#define	CTempoPeriod			1.5f																// critical tempo period in second
#define	CTempoEnhFactor			M_E																	// critical tempo enhancement factor
#define	PhaseCombFiltSharp		0.05f																// Pulse width w.r.t 1 period
#define	TempoPeakSharp			0.05f																// Pulse width w.r.t corr spec
#define	TempoEnhPhaseMax		1.4142f																// max tempo strength enhancement factor
#define	PresetTempoEnhPhaseMax	4.0f																// max tempo strength enhancement factor
#define	PhaseEnhTempoMax		1.4142f																// max tempo strength enhancement factor
#define	TempoEnhPhaseMaxTime	6																	// time for tempo strength enhancement to reach max
#define	PhaseEnhTempoMaxTime	6																	// time for tempo strength enhancement to reach max
#define	TempoEnhPhaseRatio		pow(TempoEnhPhaseMax,1.0f/(TempoEnhPhaseMaxTime*TempoPrecision))	// tempo strength enhancement per phase lock
#define	PhaseEnhTempoRatio		pow(PhaseEnhTempoMax,1.0f/(PhaseEnhTempoMaxTime*TempoPrecision))	// tempo strength enhancement per phase lock
#define	MinTempoPeriodStep		0.015625
#define	MeterAmbiThreshold		0.03125f
#define ResetDelayThreshold		3.0f																// minimum time difference to trigger a reset buffer

// Graphic parameter
#define	DELTA				1.0e-8f
#define	RMSMIN				16
#define	SPACEPOINTS			65536								// number of stars in the space
#define	SPACERADIUS			64									// radius of the space globe
#define	VIEWDISTANCE		(SPACERADIUS*0.8f)					// radius of the space globe
#define	BELTCIRCUMSIZE		12									// no. of line segments on circumference
#define	BELTDEPTHSIZE		64									// no. of layers to store
#define	BELTRADIUS			0.05f								// max radius
#define	BELTPERTURB			BELTRADIUS							// max perturbation
#define	BELTLENGTH			0.7071f								// belt length
#define	BELTHEAD			BELTLENGTH*0.382f					// belt head posi
#define	BELTTAIL			(-BELTLENGTH*0.618f)				// belt tail posi
#define	BELT_FPS			16									// for each belt frame
#define	BELT_RETENTION		0.5f
#define	BELT_COLOR_SRC		0.618f
#define	BELTALPHAMAX		128.0f
#define	MAXSPRITES			32									// max no. of spinning sprites
#define	SPRITESIZE			32.0f								// max size of spinning sprites
#define	SPINRADIUS			(BELTRADIUS*2.1f)					// added radius of spin stars
#define	SPINSTARSCALE		0.0025f								// scale of spin stars
#define	WAVERINGWIDTH		(SPINRADIUS*0.2f)					// width of time waveform ring
#define	STARRADIUS			8.0f								// length of star tips w.r.t cube radius (1.0f)
#define	DefaultCameraSpeed	0.03125f							// w.r.t unit sphere per second
#define	DefaultSceneRotSpeed	1/60.0f							// in seconds times average FPS
#define	StarScaleHalfLife	1									// in seconds
//#define	SceneRotPeriod		M_PI*BELTDEPTHSIZE*SPACERADIUS/(BELT_FPS*BELTLENGTH)

#define	FFTDURATION			4									// in seconds
#define	FFTSIDELENGTH		0.3
#define	FFTHEIGHT			0.08
#define	FFTALPHAMAX			192
#define	ELEVATIONANGLE		15.0								// in degrees
#define	ELEVATIONDIST		0.15								// vertical elevation between FFT and Belt
#define	POWERHALFLIFE		4									// in seconds
#define	FFTEXPFACTOR		0.03125f

extern	const int	BELT_INC;;
extern	const FLOAT BELT_OFF;

extern	int total_added;

extern	GLfloat	space_attn[3];
extern	float	zero_vector[4];
extern	float	ones_vector[4];
extern	float	zero_4vector[4];

// Lighting and material parameters
#define	attn_const_0		1.0f								// attenuation, constant term
#define	attn_const_1		1.5f								// attenuation, linear term
#define	attn_const_2		6.0f								// attenuation, quadratic term
#define	shiny_const			20.0f								// attenuation, quadratic term
extern	FLOAT	l_ambient[4];
extern	FLOAT	l_diffuse[4];
extern	FLOAT	l_specular[4];
extern	FLOAT	m_ambient[4];
extern	FLOAT	m_diffuse[4];
extern	FLOAT	m_specular[4];
extern	FLOAT	m_emissive[4];

// For offline estimation
#define	window_size		(4.0f/TempoPrecision)	// in seconds
#define	hop_size		(1.0f/TempoPrecision)	// in seconds
//#define	window_size		0.04	// in seconds
//#define	hop_size		0.01	// in seconds
#define	delta_width		6
#define	nFilterBanks	8

#define nTotalParams (nFilterBanks + 2)
#define	nTotalBufs (6 + nFilterBanks * 3)

#define SafeRelease(p) {if(p){p->Release();p=NULL;}}