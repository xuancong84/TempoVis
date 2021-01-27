#pragma once

#include <map>
#include <queue>
#include "opengl.h"
#include "parameters.h"
#include "Visualization.h"

using namespace std;

class	TempoVis{
public:
	GLwindow		*GLmain;
	Visualization	*pVisual;
	BYTE			*bmpData;
	TimedLevel		frameData;
	FLOAT			counter, FPS;
	DWORD			ss_stamp;
	HWND			hWndParent;
	RECT			win_rect;
	bool			bShowFPS;
	int				last_state, record_state;
	int				last_posi;
	__int64			last_stamp;
	__int64			last_frame_stamp;
	map <DWORD,HANDLE>	thread_id2handles;

	// For recorder
	HWAVEIN			hWaveIn;
	WAVEHDR			wavhdrs[2], *last_hdr;
	char			*wavbufs[2];			// two alternating 1M waveform buffers

	string	curr_media_name;
	double	curr_media_duration;
	queue	<TimedLevel>	paused_frames;
	map		<string, float>	tempo_values;	// stores tempo period in seconds

	TempoVis(HWND hwnd);
	~TempoVis();

	void	LoadTempoData();
	void	StoreTempoData();
	float	GetPresetTempo(const char* bstrTitle);
	int		get_rec_posi();
	int		init_rec();
	int		start_rec();
	int		stop_rec();
	void	Record(int state, bool bClear);
	bool	DrawFrame(TimedLevel *pLevels);
	void	m_WaveCBFunc(WAVEHDR *pWavHdr);
};

extern	TempoVis*	g_tempoVis;
extern	LONG		g_mode;			// bit0: show debug info, bit1: record audio?
extern	string		database_filename;
extern	string		waveform_filename;
extern	const		WAVEFORMATEX g_wavfmt;
extern	char*		all_status[];
extern	char*		g_status;
extern	FLOAT		CameraSpeed, SceneRotSpeed;
extern	vector <FLOAT>	est_fact, est_fact2;
extern	vector <vector <FLOAT>> est_spec;


void	TV_render(HWND hwndParent, TimedLevel *pLevels);
void	TV_release();
DWORD	WINAPI PostEstThreadFunc(void *param);
DWORD	WINAPI PreEstThreadFunc(void *param);
const	char *extr_fn(const char* str);
void	Record(int state, bool bClear);

