/*
	Opengl.h
	OpenGL Graphics Engine Header file
Note:
	This API assumes that the user follows the convension that by default, the currently
	selected matrix is the modelview matrix. Unless otherwise specified, all functions
	do not preserve modelview matrix in order to boost performance. Users are liable to
	preserve matrix data prior to calling API functions if neccessary. Creating multiple
	GL windows is supported but not recommended, users are encouraged to implement their
	own windows callback functions in that case.

	By Wang Xuancong (NUS)
*/
#pragma once

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "math.h"
#include "gl\gl.h"

#define	MAXGLWINDOWS	16
#define	FONTSTART		0
#define	FONTCOUNT		128

#ifndef MAXWMHANDLERS
#define	MAXWMHANDLERS	64
#endif

#ifndef	PI
#define	PI				3.1415926535897932384626
#endif

#define	DEFAULT_TIMER_ID	 1000

#define GL_ARRAY_BUFFER_ARB 		0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB	0x8893
#define GL_STATIC_DRAW_ARB			0x88E4

extern	int isVBOsupported;
extern	void	APIENTRY glNullFunc();
extern	void	(APIENTRY*glGenBuffers)(int, DWORD*);
extern	void	(APIENTRY*glBindBuffer)(DWORD, DWORD);
extern	void	(APIENTRY*glBufferData)(DWORD, int, void*, DWORD);
extern	void	(APIENTRY*glDeleteBuffers)(int, DWORD*);
extern	void	(APIENTRY*glMultiDrawArrays)(GLenum, int*, int*, int);
extern	void	(APIENTRY*glPointParameterf)(GLenum, GLfloat);
extern	void	(APIENTRY*glPointParameterfv)(GLenum, const GLfloat*);
extern	void	(APIENTRY*glMultiDrawElements)(GLenum mode, int *count, GLenum type, const void **indices, int primcount);

class		GLwindow;
LRESULT		CALLBACK _wndProc(HWND, UINT, WPARAM, LPARAM);
void		CALLBACK _timerProc(HWND,UINT,UINT,DWORD);
bool		_DrawGLScene(GLwindow*);
bool		RunMsgLoop(DWORD = 0);

extern	GLwindow	*All_GLWindows[];
extern	int			N_GLWindows;
extern	char		_defaultWindowName[];

class GLwindow{
public:
	class Font2D{
		public:
		int 	base;
		HFONT   hFont;
		Font2D(int,HFONT);
		~Font2D();
	};
	class Font3D{
		public:
		int     base;
		HFONT   hFont;
		float   thickness;
		GLYPHMETRICSFLOAT   gmf[FONTCOUNT];
		Font3D(int,HFONT);
		~Font3D();
	};
	struct WMHandle{
		UINT	msg;
		WNDPROC	handleProc;
	}*pWMHandleList;
	int			N_WMHandles;

	char		className[16];
	char		*windowName;
	bool		fullScreen;
	bool		isActive;
	bool		bUse3D;
	bool		bExtDC;
	int			cbStack;
	WNDCLASSEX	wcex;
	DEVMODE		dmScreenSettings;
	HWND		hWindow,hParentWindow;
	HGLRC		hRC;
	HDC			hDC;
	int			Left,Top,Width,Height;
	int			BitsPerPixel;
	HMENU		ChildWindowID;			//Use wcex.lpszMenuName for menu
	DWORD		dwExStyle,dwStyle;
	DWORD		ticksPerFrame;
	DWORD		userData;
	int			fontBase;				//Display list offset of current font
	double		ddFOVY;					//Field of view angle along X-Z plane in degrees
	double		ddClipNear,ddClipFar;	//The near and far clipping plane
	PIXELFORMATDESCRIPTOR 	pfd;
	WNDPROC		oldWndProc;

	DWORD		CreateGLWindow( bool=false );
	DWORD		CreateGLWindow( HWND, bool=false );	//Create OpenGL context from HWND
	DWORD		CreateGLWindow( HDC );				//Create OpenGL context from HDC
	DWORD		DestroyGLWindow();
	bool		SetFont(int,int,int,bool,bool,bool,char* =NULL);
	HFONT       SetFont(HFONT);
	Font2D*	    BuildFont2D(int,int,int,bool,bool,bool,char* =NULL);
	Font2D*	    BuildFont2D(HFONT);
	Font3D*	    BuildFont3D(HFONT,float,int=WGL_FONT_POLYGONS);
	int			GetExtFunc();
	bool        DestroyFont(int);
	bool		SetFPS(int FPS = 0);
	void		ResizeGLScene(int,int);
	void		glPrintf(char*,...);
	void		glPrintf(Font3D*,char*,...);
	void        glPrintf(Font2D*,char*,...);
	void        glXYPrintf(int,int,WORD,char*,...);
	void        glXYPrintf(Font2D*,int,int,WORD,char*,...);
	void        glXYZPrintf(Font3D*,int,int,int,DWORD,char*,...);
	bool		(*DrawGLScene)(GLwindow*);

	WNDPROC		RegisterWMHandler(UINT,WNDPROC);
	WNDPROC		inline	UnregisterWMHandler(UINT msg){return RegisterWMHandler(msg,NULL);}
	LRESULT	CALLBACK	wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	GLwindow();
	~GLwindow();
};

bool	RunMsgLoop(DWORD N_cycles);
LRESULT CALLBACK _wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void	CALLBACK _timerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
bool	_DrawGLScene(GLwindow* pWin);

bool InvertMatrix(FLOAT *m, FLOAT *invOut);
float* ComputeNormalA(float* pVertex, float* pNormal, int width, int height);
float* ComputeNormalB(float* pVertex, float* pNormal, int width, int height);
extern	float* (*ComputeNormal)(float* pVertex, float* pNormal, int width, int height);

inline FLOAT *MatrixMulVector4(FLOAT *mat, FLOAT *vin, FLOAT *vout){
#ifdef	USESIMD
	__asm{
		mov			eax, [mat]
			mov			ebx, [vin]
			movups		xmm0, [eax]
			movss		xmm4, [ebx]
			movups		xmm1, [eax + 16]
			movss		xmm5, [ebx + 4]
			movups		xmm2, [eax + 32]
			movss		xmm6, [ebx + 8]
			movups		xmm3, [eax + 48]
			movss		xmm7, [ebx + 12]
			unpcklps	xmm4, xmm4
			unpcklps	xmm5, xmm5
			unpcklps	xmm6, xmm6
			unpcklps	xmm7, xmm7
			movlhps		xmm4, xmm4
			movlhps		xmm5, xmm5
			movlhps		xmm6, xmm6
			movlhps		xmm7, xmm7
			mulps		xmm0, xmm4
			mulps		xmm1, xmm5
			mulps		xmm2, xmm6
			mulps		xmm3, xmm7
			addps		xmm0, xmm1
			addps		xmm0, xmm2
			addps		xmm0, xmm3
			mov			eax, [vout]
			movups[eax], xmm0
	}
#else
	for (int x = 0; x<4; x++){
		vout[x] = 0;
		for (int y = 0; y<4; y++) vout[x] += mat[(y << 2) + x] * vin[y];
	}
#endif
	return	vout;
}

/*
inline FLOAT *MatrixMulMatrix4( FLOAT *mat1, FLOAT *mat2, FLOAT *mout ){
int	x, y;
for(x=0; x<4; x++){
vout[x] = 0;
for(y=0; y<4; y++)	vout[x] += mat[(y<<2)+x]*vin[y];
}
return	vout;
}
*/

inline FLOAT VectorLength(float *v){
#ifdef USESIMD
	float	res;
	__asm{
		mov			eax, [v]
			movups		xmm0, [eax]
			mulps		xmm0, xmm0
			movhlps		xmm4, xmm0
			unpcklps	xmm0, xmm0
			movhlps		xmm1, xmm0
			addss		xmm0, xmm1
			addss		xmm0, xmm4
			sqrtss		xmm0, xmm0
			movss[res], xmm0
	}
	return	res;
#else
	return  (FLOAT)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
#endif
}

inline FLOAT VectorDot(float *v1, float *v2){
#ifdef USESIMD
	float	res;
	__asm{
		mov			eax, [v1]
			mov			ebx, [v2]
			movups		xmm0, [eax]
			movups		xmm1, [ebx]
			mulps		xmm0, xmm1
			movhlps		xmm4, xmm0
			unpcklps	xmm0, xmm0
			movhlps		xmm1, xmm0
			addss		xmm0, xmm1
			addss		xmm0, xmm4
			movss[res], xmm0
	}
	return	res;
#else
	return	v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2];
#endif
}

inline FLOAT Vector4Dot(float *v1, float *v2){
#ifdef USESIMD
	float	res;
	__asm{
		mov		eax, [v1]
			mov		ebx, [v2]
			movups	xmm0, [eax]
			movups	xmm1, [ebx]
			mulps	xmm0, xmm1
			haddps	xmm0, xmm0
			haddps	xmm0, xmm0
			movss[res], xmm0
	}
	return	res;
#else
	return	v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2];
#endif
}

inline FLOAT *VectorNorm(float *v){
#ifdef USESIMD
	__asm{
		mov			eax, [v]
			movups		xmm0, [eax]
			movaps		xmm2, xmm0
			mulps		xmm0, xmm0
			movhlps		xmm4, xmm0
			unpcklps	xmm0, xmm0
			movhlps		xmm1, xmm0
			addss		xmm0, xmm1
			addss		xmm0, xmm4
			rsqrtss		xmm0, xmm0
			unpcklps	xmm0, xmm0
			movlhps		xmm0, xmm0
			mulps		xmm0, xmm2
			movlps[eax], xmm0
			movhlps		xmm0, xmm0
			movss[eax + 8], xmm0
	}
	return	v;
#else
	FLOAT	len = 1.0f / VectorLength(v);
	v[0] *= len;
	v[1] *= len;
	v[2] *= len;
	return	v;
#endif
}

inline FLOAT *VectorMul(float *vin, float rhs, float *vout){
#ifdef USESIMD
	__asm{
		movss		xmm1, rhs
			mov			eax, vin
			unpcklps	xmm1, xmm1
			movups		xmm0, [eax]
			movlhps		xmm1, xmm1
			mulps		xmm0, xmm1
			mov			eax, vout
			movlps[eax], xmm0
			movhlps		xmm0, xmm0
			movss[eax + 8], xmm0
	}
#else
	vout[0] = vin[0] * rhs;
	vout[1] = vin[1] * rhs;
	vout[2] = vin[2] * rhs;
#endif
	return	vout;
}

inline FLOAT *Vector4Mul(float *vin, float rhs, float *vout){
#ifdef USESIMD
	__asm{
		movss		xmm1, rhs
			mov			eax, vin
			unpcklps	xmm1, xmm1
			movups		xmm0, [eax]
			movlhps		xmm1, xmm1
			mulps		xmm0, xmm1
			mov			eax, vout
			movups[eax], xmm0
	}
#else
	vout[0] = vin[0] * rhs;
	vout[1] = vin[1] * rhs;
	vout[2] = vin[2] * rhs;
	vout[3] = vin[3] * rhs;
#endif
	return	vout;
}

inline float *VectorCross(float *v1, float *v2, float *vOut){
#ifdef USESIMD
	__asm{
		mov		ebx, [v2]
			mov		eax, [v1]
			mov		edx, [vOut]
			movups	xmm3, [ebx]
			movups	xmm0, [eax]
			movaps	xmm1, xmm3
			movaps	xmm2, xmm0
			psrldq	xmm1, 4
			psrldq	xmm2, 4
			movhps	xmm1, [ebx]
			movhps	xmm2, [eax]
			mulps	xmm0, xmm1
			mulps	xmm2, xmm3
			subps	xmm0, xmm2
			movss[edx + 8], xmm0
			psrldq	xmm0, 4
			movlps[edx], xmm0
	}
#else	
	vOut[0] = v1[1] * v2[2] - v1[2] * v2[1];
	vOut[1] = v1[2] * v2[0] - v1[0] * v2[2];
	vOut[2] = v1[0] * v2[1] - v1[1] * v2[0];
#endif
	return	vOut;
}

inline float *VectorCrossAdd(float *v1, float *v2, float *vOut){
#ifdef USESIMD
	__asm{
		mov		ebx, [v2]
			mov		eax, [v1]
			mov		edx, [vOut]
			movups	xmm3, [ebx]
			movups	xmm0, [eax]
			movups	xmm4, [edx]
			movaps	xmm1, xmm3
			movaps	xmm2, xmm0
			psrldq	xmm1, 4
			psrldq	xmm2, 4
			movhps	xmm1, [ebx]
			movhps	xmm2, [eax]
			mulps	xmm0, xmm1
			mulps	xmm2, xmm3
			subps	xmm0, xmm2
			movss	xmm1, xmm0
			psrldq	xmm0, 4
			movlhps	xmm0, xmm1
			addps	xmm0, xmm4
			movss[edx], xmm0
			psrldq	xmm0, 4
			movlps[edx + 4], xmm0
	}
#else	
	vOut[0] = v1[1] * v2[2] - v1[2] * v2[1];
	vOut[1] = v1[2] * v2[0] - v1[0] * v2[2];
	vOut[2] = v1[0] * v2[1] - v1[1] * v2[0];
#endif
	return	vOut;
}

inline float *VectorSub(float *v1, float *v2, float *vOut){
#ifdef USESIMD
	__asm{
		mov		eax, v1
			mov		ebx, v2
			mov		edx, vOut
			movups	xmm0, [eax]
			movups	xmm1, [ebx]
			subps	xmm0, xmm1
			movlps[edx], xmm0
			movhlps	xmm0, xmm0
			movss[edx + 8], xmm0
	}
#else
	vOut[0] = v1[0] - v2[0];
	vOut[1] = v1[1] - v2[1];
	vOut[2] = v1[2] - v2[2];
#endif
	return	vOut;
}

inline float *Vector4Inter(float *v1, float *v2, float s_factor, float *vout){
#ifdef USESIMD
	float	one = 1;
	__asm{
		movss		xmm2, s_factor
			movss		xmm3, one
			unpcklps	xmm2, xmm2
			unpcklps	xmm3, xmm3
			movlhps		xmm2, xmm2
			movlhps		xmm3, xmm3
			mov			eax, v1
			mov			ebx, v2
			movups		xmm0, [eax]
			movups		xmm1, [ebx]
			subps		xmm3, xmm2
			mulps		xmm0, xmm2
			mulps		xmm1, xmm3
			addps		xmm0, xmm1
			mov			eax, vout
			movups[eax], xmm0
	}
#else
	vout[0] = v1[0] * s_factor + v2[0] * (1 - s_factor);
	vout[1] = v1[1] * s_factor + v2[1] * (1 - s_factor);
	vout[2] = v1[2] * s_factor + v2[2] * (1 - s_factor);
	vout[3] = v1[3] * s_factor + v2[3] * (1 - s_factor);
#endif
	return	vout;
}

inline float *VectorAdd(float *v1, float *v2, float *vOut){
#ifdef USESIMD
	__asm{
		mov		eax, v1
			mov		ebx, v2
			mov		edx, vOut
			movups	xmm0, [eax]
			movups	xmm1, [ebx]
			addps	xmm0, xmm1
			movlps[edx], xmm0
			movhlps	xmm0, xmm0
			movss[edx + 8], xmm0
	}
#else
	vOut[0] = v1[0] + v2[0];
	vOut[1] = v1[1] + v2[1];
	vOut[2] = v1[2] + v2[2];
#endif
	return	vOut;
}

inline float *Vector4Add(float *v1, float v, float *vOut){
#ifdef USESIMD
	__asm{
		mov			eax, v1
			movss		xmm1, v
			movups		xmm0, [eax]
			unpcklps	xmm1, xmm1
			movlhps		xmm1, xmm1
			mov			eax, vOut
			addps		xmm0, xmm1
			movups[eax], xmm0
	}
#else
	vOut[0] = v1[0] + v;
	vOut[1] = v1[1] + v;
	vOut[2] = v1[2] + v;
	vOut[3] = v1[3] + v;
#endif
	return	vOut;
}


