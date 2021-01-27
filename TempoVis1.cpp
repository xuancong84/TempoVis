//#define _ISOC99_SOURCE
//#define __STDC_WANT_DEC_FP__

#include	"stdafx.h"
#include	"resource.h"
#include	"parameters.h"
#include	"TempoVisdll.h"
#include	"TempoVis1.h"
#include	"TempoEst.h"

using namespace std;

//#define	NOGRAPHICS	1

class		TempoVis;
LONG		g_mode;			// bit0: show debug info, bit1: record audio?
TempoVis	*g_tempoVis=NULL;
int			g_rec_cmd=0, g_rec_write=0;
string		database_filename(string(getenv("SystemDrive")) + getenv("HOMEPATH") + "\\Music\\TempoVis.txt");
string		waveform_filename(string(getenv("SystemDrive")) + getenv("HOMEPATH") + "\\Music\\TempoVis.pcm");
char		*all_status[]={
	"",
	"Decompressing Audio File...",
	"Computing Tempo...",
	"Recording Audio...",
	"Recorder Initialization Failed!",
	"WaveInSelect Failed!",
};
char	*g_status = all_status[0];
char	*g_error = NULL;
FLOAT	CameraSpeed = DefaultCameraSpeed;
FLOAT	SceneRotSpeed = DefaultSceneRotSpeed;

bool	CheckFloat( float *f, int N=1 ){
	for( int x=0; x<N; x++ )
		if( f[x]>FLT_MAX || f[x]<-FLT_MAX || f[x]!=f[x] ){
//			__asm int 3
			return false;
		}
	return	true;
}

const char *extr_fn( const char* str ){
	const char *p = strrchr(str, '\\');
	return	p?(p+1):str;
}

const WAVEFORMATEX g_wavfmt={
	WAVE_FORMAT_PCM,	//format tag
	1,					//no. of channels
	RECORDERSR,			//samples per second
	RECORDERSR*2,		//byte rate
	4,					//block align
	16,					//bits per sample
	0					//no. of reserved bytes
};

void	CALLBACK WaveCBFunc( HWAVEIN hWaveIn, UINT msg, DWORD vInst, DWORD param1, DWORD param2 );
void	Record( int state, bool bClear );
LRESULT CALLBACK keyFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

vector <vector <FLOAT>> est_spec;
vector <FLOAT> est_fact, est_fact2;

int		WaveInSelect(char *controlName){
	int x,y;
	MIXERCAPS			mc;
	MIXERCONTROL		mxc[20];
	MIXERLINE			ml={sizeof(MIXERLINE)};
	MIXERLINECONTROLS	mlc={sizeof(MIXERLINECONTROLS)};
	MIXERCONTROLDETAILS	mcd={sizeof(MIXERCONTROLDETAILS)};
	MIXERCONTROLDETAILS_LISTTEXT mcd_lt[16];
	MIXERCONTROLDETAILS_UNSIGNED mcd_u[16];

	x = mixerGetID((HMIXEROBJ)0,(UINT*)&y,MIXER_OBJECTF_WAVEIN);
	x |= mixerGetDevCaps(y,&mc,sizeof(MIXERCAPS));
	ml.dwComponentType	=	MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
	x |= mixerGetLineInfo((HMIXEROBJ)0,&ml,
		MIXER_GETLINEINFOF_COMPONENTTYPE | MIXER_OBJECTF_WAVEIN);
	if(x)return 1;

	mlc.dwLineID	=	ml.dwLineID;
	mlc.cControls	=	ml.cControls;
	mlc.pamxctrl	=	&mxc[0];
	mlc.cbmxctrl	=	sizeof(MIXERCONTROL);
	x = mixerGetLineControls((HMIXEROBJ)0,&mlc,
		MIXER_GETLINECONTROLSF_ALL | MIXER_OBJECTF_WAVEIN);
	if(x)return 2;

	for(x=0; x<(int)ml.cControls; x++)
		if(!stricmp(mxc[x].szShortName, "WaveIn Select")
			||!stricmp(mxc[x].szShortName, "Mux")) goto pass1;
	x=0;
pass1:
	mcd.cChannels	=	ml.cChannels;
	mcd.dwControlID	=	mxc[x].dwControlID;
	mcd.cMultipleItems = mxc[x].cMultipleItems;
	mcd.cbDetails	=	sizeof(MIXERCONTROLDETAILS_LISTTEXT);
	mcd.paDetails	=	&mcd_lt[0];
	x = mixerGetControlDetails((HMIXEROBJ)0,&mcd,
		MIXER_GETCONTROLDETAILSF_LISTTEXT | MIXER_OBJECTF_WAVEIN);
	if(x)return 4;

	for(x=0; x<(int)mcd.cMultipleItems; x++){
		mcd_u[x].dwValue = strcmp(mcd_lt[x].szName,controlName)?0:1;
	}
	mcd.cbDetails	=	sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.paDetails	=	&mcd_u[0];
	x = mixerSetControlDetails((HMIXEROBJ)0,&mcd,
		MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_WAVEIN);
	return x?5:0;
}
/*
int	WAV2PCM( char *filename, char **pOut, DWORD *data_size ){
	typedef struct RIFF{
		char	ChunkID[4];
		DWORD	ChunkSize;
		char	Format[4];
		char	Subchunk1ID[4];
		DWORD	Subchunk1Size;
		WORD	AudioFormat;
		WORD	NumChannels;
		DWORD	SampleRate;
		DWORD	ByteRate;
		WORD	BlockAlign;
		WORD	BitsPerSample;
		char	Subchunk2ID[4];
		DWORD	Subchunk2Size;
	}RIFF;
	RIFF	riff;
	FILE	*fp=NULL;
	riff.SampleRate = 0;
	if( (fp=fopen(filename,"rb"))==NULL ) goto end;
	fseek(fp,0,SEEK_SET);
	if(fread(&riff, sizeof(RIFF)-4, 1, fp) != 1) goto end;
	if(memcmp(riff.ChunkID,"RIFF",4)) goto end;
	if(riff.AudioFormat != 1) goto end;
	while(strncmp(riff.Subchunk2ID,"data",4)){
		riff.Subchunk2ID[0] = riff.Subchunk2ID[2];
		riff.Subchunk2ID[1] = riff.Subchunk2ID[3];
		if(fread(&riff.Subchunk2ID[2],2,1,fp)!=1) goto end;
	}
	if(fread(&riff.Subchunk2Size,4,1,fp)!=1) goto end;
	int size = riff.Subchunk2Size;
	char *data = new char [size];
	if(fread(data, 1, size, fp)!= size) goto end;
	*data_size = size;
	*pOut = data;
end:
	if(fp) fclose(fp);
	return riff.SampleRate;
}*/

#include "wmsdk.h"
bool	ASF2PCM(const string &filename, vector <char> &data, int &num_bytes, int &sampling_rate,
	int &num_channels, int &block_align ){	// return sampling rate

	IWMSyncReader*		wmreader=NULL;
	INSSBuffer*			inssbuf=NULL;
	IWMOutputMediaProps*outfmt=NULL;
	WM_MEDIA_TYPE*		pType=NULL;
	num_bytes = 0;

	QWORD	pcnsSampleTime, pcnsDuration;
	DWORD	pdwFlags, pdwOutputNum, len;
	WORD	pwStreamNum;
	BYTE	*buf;

	DWORD	size = MultiByteToWideChar( CP_ACP, 0, filename.data(), -1, NULL, 0 );
	wstring	w_filename(size,' ');
	MultiByteToWideChar( CP_ACP, 0, filename.data(), -1, (LPWSTR)w_filename.data(), size );

	if (WMCreateSyncReader(NULL, 0, &wmreader)) goto end;
	if( wmreader->Open(w_filename.data()) ) goto end;
	if( wmreader->SetReadStreamSamples( 1, false ) ) goto end;
	if( wmreader->GetOutputProps( 0, &outfmt ) ) goto end;
	if( outfmt->GetMediaType( NULL, &size ) ) goto end;
	pType = (WM_MEDIA_TYPE*) new char [size];
	if( outfmt->GetMediaType( pType, &size ) ) goto end;
	WAVEFORMATEX *wavfmt = (WAVEFORMATEX*) pType->pbFormat;
	if( wavfmt->wBitsPerSample != 16 ) goto end;
	if( wavfmt->wFormatTag != WAVE_FORMAT_PCM ) goto end;
	if( (num_channels=wavfmt->nChannels)<1 ) goto end;
	sampling_rate = wavfmt->nSamplesPerSec;
	block_align = wavfmt->nBlockAlign;

	// convert all data
	while( (wmreader->GetNextSample( 1, &inssbuf, &pcnsSampleTime, 
		&pcnsDuration, &pdwFlags, &pdwOutputNum, &pwStreamNum )) == S_OK ){
		inssbuf->GetBufferAndLength( &buf, &len );
		data.resize(num_bytes+len);
		memcpy(&data[num_bytes], buf, len);
		num_bytes += len;
		SafeRelease(inssbuf);
	}
end:
	SafeRelease( inssbuf );
	SafeRelease( outfmt );
	if( wmreader ){
		wmreader->Close();
		wmreader->Release();
	}
	if( pType )	delete	[] pType;
	return	num_bytes>0;
}// retrieve PCM wave data from a media file

#define WINVER _WIN32_WINNT_WIN7
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>

HRESULT GetAudioInfo(IMFSourceReader *pReader, int &sampling_rate, int &num_channels, int &block_align)
{
	IMFMediaType* pPartialType = NULL;
	IMFMediaType* pUncompressedAudioType = NULL;

	// Select the first audio stream, and deselect all other streams.
	HRESULT hr = pReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);

	hr = pReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

	// Create a partial media type that specifies uncompressed PCM audio.
	hr = MFCreateMediaType(&pPartialType);

	hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

	hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

	hr = pPartialType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);

	// Set this type on the source reader. The source reader will load the necessary decoder.
	hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pPartialType);
	if (FAILED(hr))
		return hr;

	// Get the complete uncompressed format.
	hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pUncompressedAudioType);

	// Ensure the stream is selected.
	if (SUCCEEDED(hr))
		hr = pReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

	if (SUCCEEDED(hr))
		hr = pUncompressedAudioType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, (UINT32*)&sampling_rate);

	if (SUCCEEDED(hr))
		hr = pUncompressedAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, (UINT32*)&num_channels);

	if (SUCCEEDED(hr))
		hr = pUncompressedAudioType->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (UINT32*)&block_align);

	if (pPartialType)
		pPartialType->Release();

	if (pUncompressedAudioType)
		pUncompressedAudioType->Release();

	return hr;
}

bool Audio2PCM( const string &filename, vector <char> &data, int &num_bytes, int &sampling_rate, int &num_channels, int &block_align ){
	IMFSourceReader *pReader = NULL;
	IMFSample *pSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;
	num_bytes = 0;
	MFStartup(MF_VERSION);

	// convert file name
	int size = MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, NULL, 0);
	wstring w_filename(size, ' ');
	MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, (LPWSTR)w_filename.data(), size);

	if (FAILED(MFCreateSourceReaderFromURL(w_filename.data(), NULL, &pReader)))
		goto end;

	if (FAILED(GetAudioInfo(pReader, sampling_rate, num_channels, block_align)))
		goto end;

	while (true){
		DWORD dwFlags = 0;
		if (FAILED(pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &dwFlags, NULL, &pSample))) break;
		if (!pSample) break;
		if (FAILED(pSample->ConvertToContiguousBuffer(&pBuffer))) break;
		if (!pBuffer) break;
		DWORD cbBuffer;
		BYTE *pAudioData=NULL;
		if (FAILED(pBuffer->Lock(&pAudioData, NULL, &cbBuffer))) break;
		data.resize(num_bytes + cbBuffer);
		memcpy(&data[num_bytes], pAudioData, cbBuffer);
		num_bytes += cbBuffer;
		if (FAILED(pBuffer->Unlock())) break;
		SafeRelease(pSample);
		SafeRelease(pBuffer);
	}
end:
	SafeRelease(pSample);
	SafeRelease(pBuffer);
	SafeRelease(pReader);
	MFShutdown();
	return (num_bytes > 0);
}

TempoVis::TempoVis( HWND hwnd ){
	// Initialize variables
	bShowFPS	=	false;
	hWndParent	=	false;
	hWaveIn		=	NULL;
	counter		=	0;
	FPS			=	25;
	wavbufs[0]  	= wavbufs[1] = NULL;
	last_state		= -1;
	record_state	= 0;
	curr_media_duration	= 0;

	// Create OpenGL Rendering context
	srand( GetTickCount() );
	GLmain	=	new GLwindow();
	GLmain->bUse3D = true;
	if( IsWindow(hwnd) ){
		if( GLmain->CreateGLWindow( hwnd, true ) ){
			delete GLmain;
			GLmain = NULL;
			return;
		}
	}else{
		if( GLmain->CreateGLWindow( (HDC)hwnd ) ){
			delete GLmain;
			GLmain = NULL;
			return;
		}
	}
	GLmain->userData = (DWORD)this;

	// could not receive WM_MOUSEWHEEL message from parent
	vector <UINT> msgs = { WM_LBUTTONDOWN, WM_LBUTTONUP, WM_MBUTTONDOWN, WM_MBUTTONUP, WM_MOUSEMOVE };
	for (UINT msg: msgs)
		GLmain->RegisterWMHandler( msg, keyFunc );

	// Load point sprite bitmap
	HBITMAP		hBitmap = LoadBitmap( GetModuleHandle("TempoVis.dll"), (char*)IDB_SPRITE );
	char		*bmpInfoBuf[sizeof(BITMAPINFO)+128];
	memset( bmpInfoBuf, 0, sizeof(BITMAPINFO)+128 );
	BITMAPINFO	&bmpInfo=*(BITMAPINFO*)bmpInfoBuf;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits( GLmain->hDC, hBitmap, 0, 64, NULL, &bmpInfo, DIB_RGB_COLORS );
	bmpData = new BYTE [bmpInfo.bmiHeader.biSizeImage];
	GetDIBits( GLmain->hDC, hBitmap, 0, 64, bmpData, &bmpInfo, DIB_RGB_COLORS );

	// Create objects
	pVisual = new Visualization( GLmain, bmpData );
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	//GLmain->SetFont(-16, -6, 1, false, false, false, NULL);
	//hFontBg	= GLmain->BuildFont2D(GLmain->Width/16, GLmain->Height/16, 4, false, false, false, "Times New Roman");
	//hFontSm	= GLmain->BuildFont2D(16,0,FW_DONTCARE,false,false,false);

	// Load tempo database
	LoadTempoData();

	// To deal with WMP notify new media before creating this
	if( !i_media_name.empty() ){
		g_tempoVis = this;		// avoid PreEstThreadFunc calling g_tempoVis(NULL)->*
		pVisual->g_nChannels  = i_nChannels;
		pVisual->g_sampleRate = i_sampleRate;
		curr_media_duration	  = i_curr_media_duration;
		GetPresetTempo(extr_fn(i_media_name.c_str()));
		if( pVisual->preset_tempo<0 && g_mode&2 && curr_media_duration<MAXWAVLENGTH ){
			DWORD	thread_id;
			HANDLE	hThread;
			SetThreadPriority(
				hThread = CreateThread( NULL, 0, PreEstThreadFunc, new string(i_media_name), 0, (DWORD*)&thread_id ),
				THREAD_PRIORITY_IDLE
			);
			thread_id2handles[thread_id] = hThread;
		}else if( pVisual->preset_tempo>=0 ) g_status = all_status[0];
		pVisual->reset( NULL, 0 );
	}
}
TempoVis::~TempoVis(){
	for (auto mit : thread_id2handles)
		TerminateThread(mit.second, 0);
	
	if( hWaveIn ){
		stop_rec();
		for( int x=0; x<2; x++ ){
			waveInUnprepareHeader( hWaveIn, &wavhdrs[x], sizeof(WAVEHDR) );
			if( wavbufs[x] ) delete wavbufs[x];
		}
		waveInClose( hWaveIn );
	}
	if( pVisual )	delete	pVisual;
	if( GLmain  )	delete	GLmain;
	if( bmpData )	delete	bmpData;
}

void	TempoVis::LoadTempoData(){
	ifstream	fin(database_filename.data());
	string		str, str2;
	float		fval;
	while (fin.good()){
		getline(fin, str);
		getline(fin, str2);
		if (!fin.fail()) if (sscanf(str2.c_str(), "%f", &fval) == 1)
			tempo_values[str] = fval ? (60.0f / fval) : 0;
	}
}

void	TempoVis::StoreTempoData(){
	ofstream	fout(database_filename.data());
	map<string, float>::iterator mip;
	for (mip = tempo_values.begin(); mip != tempo_values.end(); ++mip){
		fout << mip->first << endl;
		fout << (mip->second ? (60.0f / mip->second) : 0) << endl;
	}
}

float	TempoVis::GetPresetTempo(const char* bstrTitle){
	float	preset_tempo = -1;
	curr_media_name.assign(bstrTitle);
	map <string, float>::iterator mip = tempo_values.find(curr_media_name);
	if (mip != tempo_values.end())	preset_tempo = mip->second;
	if (!strlen(bstrTitle)) preset_tempo = 0;
	pVisual->preset_meter = 2;
	if (preset_tempo != -1)if (preset_tempo < 0){
		pVisual->preset_meter = 3;
		preset_tempo = abs(preset_tempo);
	}
	// check for invalid preset tempo causing crash
	if (preset_tempo*TempoPrecision > TempoMaxShift)
		preset_tempo = 0;
	pVisual->preset_tempo = preset_tempo;
	return	preset_tempo;
}

int	TempoVis::get_rec_posi(){
	DWORD	ret_posi;
	char	*pData1 = wavhdrs[(last_hdr == &wavhdrs[0]) ? 1 : 0].lpData, *pData2;
	MMTIME	mmtime = { TIME_BYTES };
	waveInGetPosition(hWaveIn, &mmtime, sizeof(MMTIME));
	ret_posi = (mmtime.u.cb%RecBufSize) & 0xfffffffc;
	pData2 = &pData1[ret_posi] - 4;
	if (pData2 < pData1) return	0;
	__asm{
		mov		ecx, ret_posi
			mov		edi, pData2
			shr		ecx, 2
			pushf
			push	es
			push	ds
			pop		es
			xor		eax, eax
			std
			repz	scasd
			pop		es
			popf
			mov		pData2, edi
	}
	if (pData2 - pData1 + 4 < 0 || pData2 - pData1 + 4 > RecBufSize)__asm int 3
		return	pData2 - pData1 + 4;
}

int	TempoVis::init_rec(){
	if (waveInOpen(&hWaveIn, !waveInGetNumDevs() ? WAVE_MAPPER : 0, &g_wavfmt,
		(DWORD)WaveCBFunc, 0, WAVE_MAPPED | CALLBACK_FUNCTION)) return 1;
	else{
		if (WaveInSelect("Stereo Mix")) g_error = all_status[5];
		for (int x = 0; x < 2; x++){
			wavbufs[x] = new char[RecBufSize + WAVSIZE4];
			memset(&wavhdrs[x], 0, sizeof(WAVEHDR));
			memset(wavbufs[x], 0, RecBufSize + WAVSIZE4);
			wavhdrs[x].dwBufferLength = RecBufSize;
			wavhdrs[x].lpData = &wavbufs[x][WAVSIZE4];
			wavhdrs[x].lpNext = &wavhdrs[x ^ 1];
			int y = 0;
			y |= waveInPrepareHeader(hWaveIn, &wavhdrs[x], sizeof(WAVEHDR));
			if (y) return 2;
		}
	}
	return	0;
}

int TempoVis::start_rec(){
	for (int x = 0; x < 2; x++){
		if (waveInAddBuffer(hWaveIn, &wavhdrs[x], sizeof(WAVEHDR))) return 1;
		last_hdr = &wavhdrs[x];
	}
	g_rec_cmd = 1;
	record_state = 1;
	return	waveInStart(hWaveIn);
}

int	TempoVis::stop_rec(){
	g_rec_cmd = 0;
	int y = waveInReset(hWaveIn);
	y |= waveInStop(hWaveIn);
	if (y) return y;
	record_state = 0;
	return	0;
}

void	TempoVis::Record(int state, bool bClear){

	if( !hWaveIn ){
		if( init_rec() ){
			g_error = all_status[4];
			return;
		}
		fclose( fopen( waveform_filename.data(),"wb" ) );
	}else if( bClear )
		fclose( fopen( waveform_filename.data(),"wb" ) );

	switch( state ){
		case play_state:
			if( record_state ) stop_rec();
			if( g_tempoVis->curr_media_duration < MAXWAVLENGTH )
				if( start_rec() ) record_state=0;	// start record failed
			break;
		case pause_state:
			if( record_state ) g_rec_write = get_rec_posi();
		case stop_state:
			stop_rec();
	}
}// Toggle and mark break for recording status

bool	TempoVis::DrawFrame(TimedLevel *pLevels){
	RECT	rect;
	static hyper time_stamp_offset=0;

	// Handle window resize
	GetWindowRect( GLmain->hWindow, &rect );
	if( rect.right-rect.left!=GLmain->Width || rect.bottom-rect.top!=GLmain->Height ){
		GLmain->ResizeGLScene( rect.right-rect.left, rect.bottom-rect.top );
	}
		
	switch (pLevels->state){
	case pause_state:
		if (last_state != pause_state){
			last_state = pause_state;
			time_stamp_offset = last_frame_stamp;
		}
		pLevels->timeStamp = last_frame_stamp;
		break;
	case stop_state:
		return false;
	case play_state:
		if (last_state == pause_state){
			last_state = play_state;
			time_stamp_offset = pLevels->timeStamp - time_stamp_offset;
		}
		pLevels->timeStamp -= time_stamp_offset;
	}
	/* deprecated, no longer works after WinXP
	switch( pLevels->state ){
		case pause_state:
			if( last_state!=pause_state ){
				last_state = pause_state;
				Record( pause_state, false );
			}

			if( pLevels->timeStamp>=0 ){
				if( paused_frames.empty() ){
					paused_frames.push( *pLevels );
				}else if( paused_frames.back().timeStamp != pLevels->timeStamp ){
					paused_frames.push( *pLevels );
				}
				FPS = pVisual->FPS;
				frameData = paused_frames.front();
			}
			pLevels = &frameData;
			pLevels->state = pause_state;
			break;

		case play_state:
			if( last_state!=play_state ){
				if( last_state==stop_state )
					g_tempoVis->GetPresetTempo( (char*)g_tempoVis->curr_media_name.c_str() );
				Record( play_state, last_state==stop_state );
				last_state = play_state;
			}
			if( pLevels->timeStamp < last_stamp )
				while( !paused_frames.empty() ) paused_frames.pop();
			if( (last_stamp=pLevels->timeStamp) < 0 ) goto record;

			if( !paused_frames.empty() ){
				if( pLevels->timeStamp != paused_frames.back().timeStamp )
					paused_frames.push( *pLevels );
				counter += 1/FPS;
				if( counter > (paused_frames.front().timeStamp-last_frame_stamp)*1e-7f ){
					counter -= (paused_frames.front().timeStamp-last_frame_stamp)*1e-7f;
					frameData = paused_frames.front();
					paused_frames.pop();
					pLevels = &frameData;
				}else pLevels = &paused_frames.front();
				pLevels->state = play_state;
			}else counter = 0;
			break;

		case stop_state:
			if( last_state!=stop_state ){
				if( g_mode&2 && pVisual->preset_tempo==-1 )
					if( g_tempoVis->record_state ){
						g_tempoVis->Record(pause_state, false);
						FILE *fp = fopen( waveform_filename, "rb" );
						if( fp ){
							fseek( fp, 0, SEEK_END );
							long fsize = ftell( fp );
							fclose( fp );
							if( fsize > curr_media_duration*g_wavfmt.nAvgBytesPerSec*0.8f 
								&& curr_media_duration > MINWAVLENGTH ){
									SetThreadPriority(
										CreateThread( NULL, 0, PostEstThreadFunc, new string(g_tempoVis->curr_media_name), 0, (DWORD*)&fsize ),
										THREAD_PRIORITY_IDLE
									);
							}
						}
					}
				last_state	= stop_state;
				last_posi	= 0;
				ss_stamp	= 0;
				frameData	= *pLevels;
				while( !paused_frames.empty() ) paused_frames.pop();
				pVisual->preset_tempo	= 0;
				pVisual->g_nChannels	= 1;
				pVisual->g_sampleRate	= RECORDERSR;
				pVisual->reset( pLevels );
				//curr_media_duration		= 0;
			}
record:
			int	 posi = get_rec_posi();
			if( posi!=last_posi ){
				float	f_pcm[2052], *f_pcm1=&f_pcm[1], mul=(float)M_PI/(WAVSIZE-1);
				BYTE	*b_pcm = &frameData.waveform[0][0];
				short	*p_pcm = (short*)&wavhdrs[last_hdr==&wavhdrs[0]?1:0].lpData[posi]-WAVSIZE2;

				for( int x=0; x<WAVSIZE2; x++ ){
					short	v = p_pcm[x];
					b_pcm[x]  = (BYTE)((v+32768)>>8);
					f_pcm1[x] = (float)v*sin(x*mul);
				}

				*(int*)f_pcm = WAVSIZE2;
				Realft( f_pcm );
				f_pcm1	 = &f_pcm[3];
				b_pcm	 = &frameData.frequency[0][0];
				*b_pcm++ = 0;
				//float sum = 0;
				for( register FLOAT *pEnd=&f_pcm[WAVSIZE2]; f_pcm1<pEnd; f_pcm1+=2,b_pcm++ ){
					float fv = (float)(log(hypot(*f_pcm1,*(f_pcm1+1))+1)*16);
					if( fv>255 ){
						//__asm int 3
						fv = 255;
					}
					//sum += exp(fv*FFTEXPFACTOR);
					//sum += fv;
					*b_pcm = (BYTE)fv;
				}
				//memset( b_pcm, (BYTE)(sum/FFTSIZE_2), FFTSIZE_2 );//log(sum/FFTSIZE_2)/FFTEXPFACTOR

				if( posi<last_posi ) ss_stamp += RecBufSize;
				frameData.state		= stop_state;
				frameData.timeStamp	= (__int64)((ss_stamp+posi)*1e7/g_wavfmt.nAvgBytesPerSec);
				last_posi = posi;
			}
			pLevels = &frameData;
	}*/

	// Start drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw quad strip FFT
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	pVisual->DrawAll( pLevels, pVisual->addData( pLevels ) );
	glPopClientAttrib();
	glPopAttrib();

	glColor4ub(0xff,0xff,0xff,0xff);
	if( bShowFPS || g_mode&1 ){
		// Show FPS
		GLmain->glXYPrintf(0,16,(WORD)0,"%c FCnt:%d FPS=%.2f DPS=%.2f freq_bin_size=%d %s", hWndParent?17:18,
			pVisual->FPS_cnt, pVisual->FPS, pVisual->Data_rate, pVisual->bins_per_bin, g_status);

		GLmain->glXYPrintf(0,32,(WORD)0,"0 1 2 3 4 5 6 7 8 :SET_BIN_SIZE");

		char ind[20]="";
		if( g_tempoVis->pVisual->freq_bin_cutoff < 0 ){
			int	y = g_tempoVis->pVisual->bins_per_bin;
			for( int x=0; x<y; x++ ) sprintf( &ind[strlen(ind)], "%d ", x );
		}
		GLmain->glXYPrintf(0,32,(WORD)0,strcat(ind,"_"));
		/*
		char str[1000]={0},ch[2]={1,0};
		for(int y=0;y<16;y++){
			str[0]=0;
			for(int x=0;x<16;x++,ch[0]++)strcat(str,ch);
			GLmain->glXYPrintf(0,16*(y+3),(WORD)0,"%s",str);
		}*/
	}
	if( g_mode&1 ){
		GLmain->glXYPrintf(0,48,(WORD)0,"Timestamp=%I64d, State=%d, Rec_state=%d, Total_added=%d, Tempo=%d/%f [%f]",
			pLevels->timeStamp, last_state, record_state, total_added, pVisual->last_phase_index,
			pVisual->last_tempo_index+pVisual->tempo_period_frac, pVisual->preset_tempo);
		GLmain->glXYPrintf(0,64,(WORD)0,"phase_enhance_tempo=%f, tempo_enhance_phase=%f",
			pVisual->tempo_enhance_factor, pVisual->phase_enhance_factor);
		GLmain->glXYPrintf(0,80,(WORD)0,"freq_bin_cutoff=%f, power_level=%f",
			pVisual->freq_bin_cutoff, pVisual->power_level);
		if( g_error ) GLmain->glXYPrintf(0,96,(WORD)0,"%s",g_error);
	}
	SwapBuffers( GLmain->hDC );
	last_frame_stamp = pLevels->timeStamp;
	return	true;
}

void TempoVis::m_WaveCBFunc(WAVEHDR *pWavHdr){
	last_hdr = pWavHdr;
	if( last_state!=stop_state && g_mode&2 && g_tempoVis->pVisual->preset_tempo==-1 ){
		FILE	*fp;
		if( (fp=fopen( waveform_filename.data(), "ab" )) == NULL ){
			return;
		}
		if( g_rec_write ){
			pWavHdr->dwBytesRecorded = g_rec_write;
			g_rec_write = 0;
		}
		fwrite( pWavHdr->lpData, 1, pWavHdr->dwBytesRecorded, fp );
		fclose( fp );
	}
	memcpy( &wavhdrs[pWavHdr==&wavhdrs[0]?1:0].lpData[-WAVSIZE4], &pWavHdr->lpData[RecBufSize-WAVSIZE4], WAVSIZE4 );
	memset( pWavHdr->lpData, 0, pWavHdr->dwBufferLength );
	if( g_rec_cmd ) waveInAddBuffer(hWaveIn,pWavHdr,sizeof(WAVEHDR));
}

LRESULT CALLBACK keyFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if (!g_tempoVis)
		return 0;
	static int last_x, last_y, last_move_x, last_move_y;
	static int last_mid_x, last_mid_y, last_mid_move_x, last_mid_move_y;
	switch (uMsg){
	case WM_LBUTTONUP:
		if (last_x == LOWORD(lParam) && last_y == HIWORD(lParam)){
			bool bToggle = true;
			if (g_tempoVis->bShowFPS || g_mode & 1){
				WORD	Xpos = LOWORD(lParam), Ypos = HIWORD(lParam);
				if (Xpos < 8 && Ypos < 16){
					HWND	&hWndParent = g_tempoVis->hWndParent;
					RECT	&rect = g_tempoVis->win_rect;
					if (hWndParent){	// if already fullscreen
						SetParent(hWnd, hWndParent);
						hWndParent = NULL;
						SetWindowPos(hWnd, HWND_TOP, 0, 0, rect.right - rect.left,
							rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
					}
					else{				// if not fullscreen
						GetWindowRect(hWnd, &rect);
						hWndParent = SetParent(hWnd, NULL);
						SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, g_tempoVis->GLmain->dmScreenSettings.dmPelsWidth,
							g_tempoVis->GLmain->dmScreenSettings.dmPelsHeight, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
					}
				}
				else if (Ypos <= 32 && Ypos >= 16){
					SIZE	fsize;
					GetTextExtentPoint32(g_tempoVis->GLmain->hDC, "0 1 2 3 4 5 6 7 8 ", 18, &fsize);
					int	bin_size = (int)(Xpos*9.0 / fsize.cx + 0.5);
					if (!bin_size){
						g_tempoVis->pVisual->freq_bin_cutoff = (float)g_tempoVis->pVisual->bins_per_bin * 128;
						bToggle = false;
					}
					else if (bin_size <= 8){
						g_tempoVis->pVisual->freq_bin_cutoff = -1;
						g_tempoVis->pVisual->bins_per_bin = bin_size;
						bToggle = false;
					}
				}
			}
			if (bToggle){
				g_tempoVis->bShowFPS = !g_tempoVis->bShowFPS;
				ComputeNormal = g_tempoVis->bShowFPS ? ComputeNormalA : ComputeNormalB;
			}
		}
	case WM_LBUTTONDOWN:
		last_move_x = last_x = LOWORD(lParam);
		last_move_y = last_y = HIWORD(lParam);
		break;
	case WM_MBUTTONDOWN:
		last_mid_move_x = last_mid_x = LOWORD(lParam);
		last_mid_move_y = last_mid_y = HIWORD(lParam);
		break;
	case WM_MBUTTONUP:
		if (last_mid_x == LOWORD(lParam) && last_mid_y == HIWORD(lParam)){
			g_tempoVis->pVisual->global_H_shift = 0;
			g_tempoVis->pVisual->global_V_shift = 0;
			g_tempoVis->pVisual->global_H_rot = 0;
			g_tempoVis->pVisual->global_V_rot = 0;
			CameraSpeed = DefaultCameraSpeed;
			SceneRotSpeed = DefaultSceneRotSpeed;
		}
		break;
	case WM_MOUSEMOVE:
		if (wParam&MK_LBUTTON){
			FLOAT x_delta = int(LOWORD(lParam)) - last_move_x;
			FLOAT y_delta = int(HIWORD(lParam)) - last_move_y;
			g_tempoVis->pVisual->global_H_rot -= x_delta / g_tempoVis->GLmain->Width;
			g_tempoVis->pVisual->global_V_rot += y_delta / g_tempoVis->GLmain->Height;
			last_move_x = LOWORD(lParam);
			last_move_y = HIWORD(lParam);
			if (x_delta!=0||y_delta!=0)
				SceneRotSpeed = CameraSpeed = 0;
		}else if (wParam&MK_MBUTTON){
			FLOAT x_delta = int(LOWORD(lParam)) - last_mid_move_x;
			FLOAT y_delta = int(HIWORD(lParam)) - last_mid_move_y;
			g_tempoVis->pVisual->global_H_shift += x_delta / g_tempoVis->GLmain->Width;
			g_tempoVis->pVisual->global_V_shift -= y_delta / g_tempoVis->GLmain->Height;
			last_mid_move_x = LOWORD(lParam);
			last_mid_move_y = HIWORD(lParam);
			if (x_delta != 0 || y_delta != 0)
				SceneRotSpeed = CameraSpeed = 0;
		}
		break;
	}
	return	0;
}

void	TV_release(){
	if( g_tempoVis ){
		delete g_tempoVis;
		g_tempoVis = NULL;
	}
}

void	TV_render( HWND hwndParent, TimedLevel *pLevels ){
	if( !hwndParent || !pLevels ){
		//Beep(1000,200);
		TV_release();
	}
re:
	if( !g_tempoVis ) if( hwndParent ){
		//Beep(2000,200);
		g_tempoVis = new TempoVis( hwndParent );
		if( !g_tempoVis->GLmain ){
			TV_release();
		}
	}
	if( g_tempoVis ){
		if( g_tempoVis->GLmain->hWindow != hwndParent ){
			//Beep(3000,200);
			TV_release();
			goto re;
		}
		g_tempoVis->DrawFrame( pLevels );
	}
}

void	CALLBACK WaveCBFunc( HWAVEIN hWaveIn, UINT msg, DWORD vInst, DWORD param1, DWORD param2 ){
	if( msg==WIM_DATA && hWaveIn==g_tempoVis->hWaveIn ){
		g_tempoVis->m_WaveCBFunc( (WAVEHDR*)param1 ); 
	}
}// OOP connector

void	Record( int state, bool bClear ){
	g_tempoVis->Record( state, bClear );
}

DWORD	WINAPI PostEstThreadFunc( void *param ){
	while (g_tempoVis->thread_id2handles.size() > 1)
		Sleep(4000);

	// Load file
	string	*media_name = (string*)param;
	const char	*media_key = extr_fn(media_name->c_str());
	char	*data = NULL;
	float	*fdata = NULL;

	FILE *fp = fopen( waveform_filename.data(), "rb" );
	fseek( fp, 0, SEEK_END );
	int	fsize = ftell( fp );
	data = new char [fsize];
	fseek( fp, 0, SEEK_SET );
	if( fread(data,1,(int)fsize,fp)!=fsize ) goto end;

	// extract from 10%~90%, head and tail part may be noisy or silence, so discard
	DWORD	size = fsize/sizeof(short);
	fsize = (DWORD)(size*0.8+0.5);
	fdata = new float [fsize];
	short	*wdata = &(((short*)data)[4+(int)(size*0.1)]);
	for( int x=0; x<fsize; x++ ) fdata[x] = (float)wdata[x];
	delete [] data;
	data = NULL;

	// invoke tempo estimator
	g_status = all_status[2];
	float	tempo = ComputeTempo( fdata, fsize, g_wavfmt.nSamplesPerSec, g_tempoVis->pVisual->pri_tempoIndex );
	g_status = all_status[0];

	// write to database file
	if( g_tempoVis ){
		g_tempoVis->tempo_values[string(media_key)] = tempo;
		g_tempoVis->StoreTempoData();
	}

end:
	// delete pre-allocated data buffer
	if(fdata) delete [] fdata;
	if(data) delete [] data;
	delete	media_name;
	
	g_tempoVis->thread_id2handles.erase(GetCurrentThreadId());
	return	0;
}// Thread for computing and saving tempo values

DWORD	WINAPI PreEstThreadFunc( void *param ){
	while (g_tempoVis->thread_id2handles.size() > 1)
		Sleep(4000);

	// Load file
	string	*media_name = (string*)param;
	const char	*media_key = extr_fn(media_name->c_str());
	int		num_bytes, sampling_rate, num_channels, block_align;
	vector <char> data;
	vector <float> fdata;
	g_tempoVis->pVisual->preset_tempo = 0;
	g_status = all_status[1];
	if (!ASF2PCM(*media_name, data, num_bytes, sampling_rate, num_channels, block_align))
		if (!Audio2PCM(*media_name, data, num_bytes, sampling_rate, num_channels, block_align))
		goto fail;

	// extract from 10%~90%, head and tail part may be noisy or silence, so discard
	if (!num_bytes || num_channels<1){
fail:
		g_tempoVis->pVisual->preset_tempo = -1;
		g_status = all_status[3];
		goto end;
	}
	int	num_samples = num_bytes/block_align;
	int first_sample = (int)round(num_samples*0.1);
	int last_sample = (int)round(num_samples*0.9);
	num_samples = last_sample - first_sample + 1;
	fdata.resize(num_samples);
	char *pdata = &data[first_sample*block_align];
	if (num_channels>1)
		for (int x = 0; x < num_samples; x++,pdata+=block_align){
			int v = 0;
			for (int y = 0; y < num_channels; ++y)
				v += *(short*)&pdata[y*sizeof(short)];
			fdata[x] = v/num_channels;
		}
	else
		for (int x = 0; x < num_samples; x++, pdata += block_align){
			fdata[x] = *(short*)pdata;
		}

	// invoke tempo estimator
	g_status = all_status[2];
	float	tempo = ComputeTempo(fdata.data(), num_samples, sampling_rate, g_tempoVis->pVisual->pri_tempoIndex);
	g_status = all_status[0];

	// write to database file
	if( g_tempoVis ){
		g_tempoVis->tempo_values[string(media_key)] = tempo;
		if( !strcmp(g_tempoVis->curr_media_name.c_str(), media_key) ){
			g_tempoVis->GetPresetTempo( media_key );
		}
		g_tempoVis->StoreTempoData();
	}

end:
	delete	media_name;

	g_tempoVis->thread_id2handles.erase(GetCurrentThreadId());
	return	0;
}// Thread for computing and saving tempo values
