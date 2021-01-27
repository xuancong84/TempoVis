#include "stdafx.h"

#include "parameters.h"
#include "VisualHelper.h"

using namespace std;

inline FLOAT *VectorCopy( FLOAT *vDst, FLOAT *vSrc ){
	memcpy( vDst, vSrc, sizeof(FLOAT)*3 );
	return	vDst;
}

void	Transpose( char *pDst, char *pSrc, int rows, int cols, int eachSize ){
	int	rowSize = rows*eachSize;
	for( int x=0; x<rows; x++ ){
		char *p = &pDst[eachSize*x];
		for( int y=0; y<cols; y++,pSrc+=eachSize,p+=rowSize ){
			memcpy( p, pSrc, eachSize);
		}
	}
}

void	expUpdate( FLOAT *data, FLOAT *newdata, int size, FLOAT prev_time, FLOAT curr_time, FLOAT half_life ){
	if( curr_time==prev_time ) return;
	if( !curr_time ){
		memcpy( data, newdata, sizeof(FLOAT)*size );
	}else{
		FLOAT	f1 = (FLOAT)exp(M_LOG_2*(prev_time-curr_time)/half_life);
		FLOAT	f2 = (FLOAT)exp(-M_LOG_2*curr_time/half_life);
		FLOAT	w1=f1-f2, w2=1-f1, w3=1.0f/(w1+w2);
		w1 *= w3;
		w2 *= w3;
		register int x = 0;
/*
		{// this block of AVX optimization code can be commented out if CPU/compiler does not support
			__m256 ymm1 = _mm256_broadcast_ss(&w1);
			__m256 ymm2 = _mm256_broadcast_ss(&w2);
			for (; x + 7 < size; ++x){
				__m256 ymm0 = _mm256_loadu_ps(&data[x]);
				__m256 ymm3 = _mm256_loadu_ps(&newdata[x]);
				ymm0 = _mm256_mul_ps(ymm0, ymm1);
				ymm3 = _mm256_mul_ps(ymm3, ymm2);
				ymm0 = _mm256_add_ps(ymm0, ymm3);
				_mm256_storeu_ps(&data[x], ymm0);
			}
		}
*/
		for(; x<size; x++){
			data[x] = (data[x]*w1 + newdata[x]*w2);
		}// weighted sum
	}
}// Update data array using negative exponential decay law

FLOAT* resizeData(	int srcWidth,int srcHeight,FLOAT *srcData,
			  		int dstWidth,int dstHeight,FLOAT *dstData	){
	if(srcWidth<=0 || srcHeight<=0 || dstWidth<=0 || dstHeight<=0)return dstData;
	if(srcData==NULL) return dstData;
	if(dstData==NULL) dstData = new FLOAT [dstWidth*dstHeight];

	int		x, y, posi;
	FLOAT	xStart, xEnd, yStart, yEnd, dx, dy, bandRadio;
	FLOAT	xRadio	=	(FLOAT)srcWidth/dstWidth;
	FLOAT	yRadio	=	(FLOAT)srcHeight/dstHeight;

	for(y=0; y<dstHeight; y++)for(x=0; x<dstWidth; x++){
		xStart	=	xRadio*x;
		xEnd	=	xRadio+xStart;
		yStart	=	yRadio*y;
		yEnd	=	yRadio+yStart;
		posi	=	y*dstWidth+x;
		dstData[posi]	=	0;
		for(dy=yStart; dy-yEnd<-0.01; floor(dy)==dy?dy++:dy=ceil(dy)){
			if(floor(dy)==dy)
				bandRadio = (dy+1>yEnd?yEnd-dy:1)/(yEnd-yStart);
			else
				bandRadio = (ceil(dy)>yEnd?yEnd-dy:ceil(dy)-dy)/(yEnd-yStart);
			for(dx=xStart; dx-xEnd<-0.01; floor(dx)==dx?dx++:dx=ceil(dx)){
				if(floor(dx)==dx)dstData[posi]+=bandRadio*srcData[(int)dy*srcWidth+(int)dx]
					*((dx+1>xEnd?xEnd-dx:1)/(xEnd-xStart));
				else	dstData[posi]+=bandRadio*srcData[(int)dy*srcWidth+(int)dx]
					*((ceil(dx)>xEnd?xEnd-dx:ceil(dx)-dx)/(xEnd-xStart));
			}
		}//-0.01 instead of 0 to prevent underflow.
	}
	return dstData;
}// Resize array using interpolation

int	getInnerMeter( FLOAT pri_tempo, FLOAT *spec, int size, int *bAmbiguous ){
	FLOAT	level3, level21, level22;
	level3	= (calcPeakHeight( spec, size, pri_tempo*0.333333f )
			 +calcPeakHeight( spec, size, pri_tempo*0.666666f) )/2;
	level21	= calcPeakHeight( spec, size, pri_tempo*0.5f );
	level22	= (calcPeakHeight( spec, size, pri_tempo*0.25f )
			 +calcPeakHeight( spec, size, pri_tempo*0.75f ))/2;
	if( bAmbiguous ){
		if( abs(level3-level21)/max(level3,level21)	< MeterAmbiThreshold*2 )
			*bAmbiguous |= 1;
		if( abs(level3-level22)/max(level3,level22) < MeterAmbiThreshold*2 )
			*bAmbiguous |= 2;
	}
	return	(level3>level21 && level3>level22)?3:2;
}// determine whether it's a 3/3 meter inwards

int	getOuterMeter( FLOAT pri_tempo, FLOAT *spec, int size, int *bAmbiguous ){
	FLOAT	level3, level2, level4;
	level2	= calcPeakHeight( spec, size, pri_tempo*2 );
	level3	= calcPeakHeight( spec, size, pri_tempo*3 );
	level4	= calcPeakHeight( spec, size, pri_tempo*4 );
	if( bAmbiguous ){
		if( abs(level3-level2)/max(level3,level2) < MeterAmbiThreshold
			&& abs(level3-level4)/max(level3,level4) < MeterAmbiThreshold )
			*bAmbiguous = 1;
	}
	return	(level3>level2 && level3>level4)?3:2;
	//return	(level3>(level2+level4)*0.5f) ? 3 : 2;
	//return	(level3>level2 || level3>level4) ? 3 : 2;
}// determine whether it's a 3/3 meter outwards

bool testLower( FLOAT pri_tempo, FLOAT lo_tempo, FLOAT *TempoSpec, int size ){
	int		bAmbi;
	FLOAT	cur_tempo = pri_tempo;
	if(abs(cur_tempo-lo_tempo)/lo_tempo<0.0625f) return true;
	while( cur_tempo>lo_tempo ){
		bAmbi = 0;
		cur_tempo /= getInnerMeter(cur_tempo,TempoSpec,size,&bAmbi);
		if( bAmbi )	if( cur_tempo<2*CTempoPeriod*TempoPrecision || !(bAmbi&1) ) break;
		int	new_tempo = findPeakPosi(TempoSpec,size,(int)(cur_tempo+0.5f));
		if(abs(cur_tempo-new_tempo)/cur_tempo>0.0625f)
			return	false;
		cur_tempo = (FLOAT)new_tempo;
		if(abs(cur_tempo-lo_tempo)/lo_tempo<0.0625f) return true;
	}
	return	false;
}// test whether can goto lower harmonics

bool testUpper( FLOAT pri_tempo, FLOAT hi_tempo, FLOAT *TempoSpec, int size ){
	int		bAmbi;
	FLOAT	cur_tempo = pri_tempo;
	if(abs(cur_tempo-hi_tempo)/hi_tempo<0.0625f) return true;
	while( cur_tempo<hi_tempo ){
		if( cur_tempo*4>=size ) break;
		bAmbi = 0;
		cur_tempo *= getOuterMeter(cur_tempo,TempoSpec,size,&bAmbi);
		if( bAmbi ) break;
		int	new_tempo = findPeakPosi(TempoSpec,size,(int)(cur_tempo+0.5f));
		if(abs(cur_tempo-new_tempo)/cur_tempo>0.0625f)
			return	false;
		cur_tempo = (FLOAT)new_tempo;
		if(abs(cur_tempo-hi_tempo)/hi_tempo<0.0625f) return true;
	}
	return	false;
}// test whether can goto upper harmonics

void	DrawSpikeArray( FLOAT*vals, int size, float Vstart, float Vend, DWORD color, bool zeroMin ){
	FLOAT	fmin=*vals, fmax=*vals, fval;
	FLOAT	*data = new FLOAT [size*sizeof(FLOAT)*4], *pDst=data, *pEnd=&data[size*sizeof(FLOAT)*4];
	for( int x=0; x<size; x++ ){
		*pDst = (FLOAT)x;
		pDst += 2;
		*pDst++ = (FLOAT)x;
		*pDst++ = fval = vals[x];
		if( fval > fmax ) fmax = fval;
		else if( fval < fmin ) fmin = fval;
	}
	if( zeroMin ) fmin=0;
	for( pDst=&data[1]; pDst<pEnd; pDst+=4 ) *pDst = fmin;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,size,fmin-(fmax-fmin)*(1-Vend)/(Vend-Vstart),fmax+(fmax-fmin)*Vstart/(Vend-Vstart),-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, data );
	glColor4ubv((BYTE*)&color);
	glDrawArrays( GL_LINES, 0, size*2 );
	glPopClientAttrib();

	delete [] data;
	//for( int x=0;x<size;x++ ) if( vals[x]<0 ) __asm int 3
}

void	DrawPointArray( FLOAT*vals, int size, float Vstart, float Vend, DWORD color, bool zeroMin ){
	FLOAT	fmin=*vals, fmax=*vals;
	FLOAT	*data = new FLOAT [size*sizeof(FLOAT)*2], *pEnd=&data[size*sizeof(FLOAT)*2];
	register FLOAT fval, *pDst=data;

	for( int x=0; x<size; x++ ){
		*pDst++ = (FLOAT)x;
		*pDst++ = fval = vals[x];
		if( fval > fmax ) fmax = fval;
		else if( fval < fmin ) fmin = fval;
	}
	if( zeroMin ) fmin=0;
	for( pDst=&data[1]; pDst<pEnd; pDst+=4 ) *pDst = fmin;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,size,fmin-(fmax-fmin)*(1-Vend)/(Vend-Vstart),fmax+(fmax-fmin)*Vstart/(Vend-Vstart),-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, data );
	glColor4ubv((BYTE*)&color);
	glDrawArrays( GL_POINTS, 0, size );
	glPopClientAttrib();

	delete [] data;
	//for( int x=0;x<size;x++ ) if( vals[x]<0 ) __asm int 3
}

//#pragma warning(disable:4244)
void FFT(float* s, int invert)
{
   int ii,jj,n,nn,limit,m,j,inc,i;
   float wx,wr,wpr,wpi,wi,theta;
   float xre,xri,x;
   
   n=*(int*)s;
   nn=n / 2; j = 1;
   for (ii=1;ii<=nn;ii++) {
      i = 2 * ii - 1;
      if (j>i) {
         xre = s[j]; xri = s[j + 1];
         s[j] = s[i];  s[j + 1] = s[i + 1];
         s[i] = xre; s[i + 1] = xri;
      }
      m = n / 2;
      while (m >= 2  && j > m) {
         j -= m; m /= 2;
      }
      j += m;
   };
   limit = 2;
   while (limit < n) {
      inc = 2 * limit; theta = M_2PI / limit;
      if (invert) theta = -theta;
      x = sin(0.5 * theta);
      wpr = -2.0 * x * x; wpi = sin(theta); 
      wr = 1.0; wi = 0.0;
      for (ii=1; ii<=limit/2; ii++) {
         m = 2 * ii - 1;
         for (jj = 0; jj<=(n - m) / inc;jj++) {
            i = m + jj * inc;
            j = i + limit;
            xre = wr * s[j] - wi * s[j + 1];
            xri = wr * s[j + 1] + wi * s[j];
            s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
            s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
         }
         wx = wr;
         wr = wr * wpr - wi * wpi + wr;
         wi = wi * wpr + wx * wpi + wi;
      }
      limit = inc;
   }
   if (invert)
      for (i = 1;i<=n;i++) 
         s[i] = s[i] / nn;
   
}

void Realft (float* s)
{
   int n, n2, i, i1, i2, i3, i4;
   float xr1, xi1, xr2, xi2, wrs, wis;
   float yr, yi, yr2, yi2, yr0, theta, x;

   n=*(int*)s / 2; n2 = n/2;
   theta = PI / n;
   FFT(s, FALSE);
   x = sin(0.5 * theta);
   yr2 = -2.0 * x * x;
   yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
   for (i=2; i<=n2; i++) {
      i1 = i + i - 1;      i2 = i1 + 1;
      i3 = n + n + 3 - i2; i4 = i3 + 1;
      wrs = yr; wis = yi;
      xr1 = (s[i1] + s[i3])/2.0; xi1 = (s[i2] - s[i4])/2.0;
      xr2 = (s[i2] + s[i4])/2.0; xi2 = (s[i3] - s[i1])/2.0;
      s[i1] = xr1 + wrs * xr2 - wis * xi2;
      s[i2] = xi1 + wrs * xi2 + wis * xr2;
      s[i3] = xr1 - wrs * xr2 + wis * xi2;
      s[i4] = -xi1 + wrs * xi2 + wis * xr2;
      yr0 = yr;
      yr = yr * yr2 - yi  * yi2 + yr;
      yi = yi * yr2 + yr0 * yi2 + yi;
   }
   xr1 = s[1];
   s[1] = xr1 + s[2];
   s[2] = 0.0;
}


int	LoopBuffer::AddFrame(FLOAT timeStamp, FLOAT *currFrame){
	FLOAT	lastStamp = time_stamp;
	time_stamp = timeStamp;

	if( lastStamp == time_stamp ){
		memcpy( last_frame_data, currFrame, frame_size );
		return	0;
	}
	assert(timeStamp > lastStamp);

	FLOAT	*lastFrame = (FLOAT*)last_frame_data;
	FLOAT	lastTime=lastStamp*time_factor, currTime=timeStamp*time_factor;
	int		lastIndex=(int)lastTime, currIndex=(int)currTime;
	for( int index=lastIndex+1; index<=currIndex; index++ ){
		FLOAT	b=(index-lastTime)/(currTime-lastTime), a=1.0f-b;
		if( ++current_frame == N_total_frames ){
			memmove( data, &data[data_size-draw_size], draw_size );
			current_frame = N_draw_frames;
		}// scroll back
		FLOAT	*pDst = (FLOAT*)&data[frame_size*current_frame];
		for( int y=0,z=frame_size/sizeof(FLOAT); y<z; y++ ){
			pDst[y] = lastFrame[y]*a+currFrame[y]*b;
		}// do linear interpolation
		if (++N_valid_frames > N_draw_frames)
			N_valid_frames = N_draw_frames;
	}
	memcpy( lastFrame, currFrame, frame_size );

	if(N_draw_frames>1)
		return	(currIndex-lastIndex)%N_draw_frames;
	return	(currIndex-lastIndex);
}// constant scrolling speed

int LoopBuffer::AddFrame(FLOAT timeStamp, FLOAT val){
	FLOAT	lastStamp = time_stamp;
	time_stamp = timeStamp;

	if( lastStamp == time_stamp ){
		*(FLOAT*)last_frame_data = val;
		return	0;
	}

	FLOAT	lastTime=lastStamp*time_factor, currTime=timeStamp*time_factor;
	int		lastIndex=(int)lastTime, currIndex=(int)currTime;
	FLOAT	fstart = *(FLOAT*)last_frame_data;
	FLOAT	finc = (val-fstart)/(currTime-lastTime);
	int		last_frame = current_frame;
	fstart += finc*(lastIndex+1-lastTime);

	for( int index=lastIndex+1; index<=currIndex; index++ ){
		if( ++current_frame >= N_total_frames ){
			memmove( data, &data[data_size-draw_size], draw_size );
			current_frame = N_draw_frames;
		}// scroll back
		((FLOAT*)data)[current_frame] = fstart;
		fstart += finc;
		if (++N_valid_frames > N_draw_frames)
			N_valid_frames = N_draw_frames;
	}
	*(FLOAT*)last_frame_data = val;
	if(N_draw_frames>1)
		return	(currIndex-lastIndex)%N_draw_frames;
	return	currIndex-lastIndex;
}


LoopBuffer::LoopBuffer(int		_N_total_frames,
			int		_N_draw_frames,
			int		_frame_size,
			FLOAT	_fps	){
	ResizeBuffer( _N_total_frames, _N_draw_frames, _frame_size, _fps );
}
LoopBuffer::~LoopBuffer(){
	if( data ) free( data );
}
void LoopBuffer::ResizeBuffer(int		_N_total_frames,
					int		_N_draw_frames,
					int		_frame_size,
					FLOAT	_fps ){
	memset( this, 0, sizeof(LoopBuffer) );
	N_total_frames	= _N_total_frames;
	N_draw_frames	= _N_draw_frames;
	frame_size		= _frame_size;
	time_factor		= _fps;
	data			= (char*)realloc( data, N_total_frames*frame_size );
	last_frame_data	= (char*)realloc( last_frame_data, frame_size );
	Reset(0);
}
void	LoopBuffer::Reset(FLOAT _time_stamp){
	current_frame	= N_draw_frames-1;
	draw_size		= N_draw_frames*frame_size;
	data_size		= N_total_frames*frame_size;
	status			= 0;
	time_stamp		= _time_stamp;
	N_valid_frames	= 0;
	memset( data, 0, data_size );
	memset( last_frame_data, 0, frame_size );
}
FLOAT	*LoopBuffer::getCurrentPtrFront(){
	return	(FLOAT*)&data[frame_size*(current_frame-N_draw_frames+1)];
}
FLOAT	*LoopBuffer::getCurrentPtrBack(){
	return	(FLOAT*)&data[frame_size*current_frame];
}



Camera::Camera(GLwindow *pWin){
	GLmain = pWin;
	reset();
}
Camera::~Camera(){}

void Camera::reset(){
	GLwindow *pGLwin=GLmain;
	memset( this, 0, sizeof(Camera) );
	GLmain	= pGLwin;
	eyev[2] = 1.0f;
	upv[1]	= 1.0f;
	viewAngle = 45.0f;
	updateCamera( eyev, centerv, upv );
}
void Camera::setCamera(){
	glPushAttrib( GL_TRANSFORM_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( viewMat );
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( projMat );
	glPopAttrib();
}
void Camera::rotateHV(FLOAT h, FLOAT v){
	FLOAT center2eye[3], center2right[3], center2up[3], v1[3],v2[3],v3[3],v4[3];
	VectorSub(eyev, centerv, center2eye);

	// Horizontal rotation
	VectorNorm(upv);
	VectorCross(upv, center2eye, center2right);
	VectorMul(center2eye, cos(h), v1);
	VectorMul(center2right, sin(h), v2);
	VectorAdd(v1, v2, v3);
	VectorMul(center2eye, -sin(h), v1);
	VectorMul(center2right, cos(h), v2);
	VectorAdd(v1, v2, center2right);
	VectorCopy(center2eye, v3);
	VectorAdd(centerv, center2eye, eyev);
	VectorAdd(centerv, center2right, rightv);
	/*
	// Vertical rotation
	VectorNorm(rightv);
	VectorCross(center2eye, rightv, center2up);
	VectorMul(center2eye, cos(v), v1);
	VectorMul(center2up, sin(v), v2);
	VectorAdd(v1, v2, v3);
	VectorMul(center2eye, -sin(v), v1);
	VectorMul(center2up, cos(v), v2);
	VectorAdd(v1, v2, center2up);
	VectorCopy(center2eye, v3);
	VectorAdd(centerv, center2eye, eyev);
	VectorNorm(center2up);
	VectorCopy(upv, center2up);*/
}
void Camera::updateCamera(FLOAT *eyev, FLOAT *objv, FLOAT *topv){
	GLfloat	dirv[3];

	glPushAttrib( GL_TRANSFORM_BIT );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();

	// Compute view matrix
	VectorCopy( this->eyev, eyev );
	VectorCopy( centerv, objv );
	VectorSub( topv, eyev, upv );
	VectorSub( centerv, eyev, dirv );
	VectorCross( dirv, upv, rightv );
	VectorCross( rightv, dirv, upv );
	glLoadIdentity();
	gluLookAt(	eyev[0], eyev[1], eyev[2],
				centerv[0], centerv[1], centerv[2],
				upv[0], upv[1], upv[2] );
	glGetFloatv( GL_MODELVIEW_MATRIX, viewMat );

	// Compute projection matrix
	glLoadIdentity();
	gluPerspective( viewAngle, (GLdouble)GLmain->Width/GLmain->Height, 0.01, VIEWDISTANCE );
	glGetFloatv( GL_MODELVIEW_MATRIX, projMat );

	glPopMatrix();
	glPopAttrib();
}
void Camera::shiftUp(FLOAT dist){
	FLOAT	mov[3];
	VectorNorm( upv );
	VectorMul( upv, dist, mov );
	VectorAdd( eyev, mov, eyev );
	VectorAdd( centerv, mov, centerv );
	glPushAttrib( GL_TRANSFORM_BIT );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(	eyev[0], eyev[1], eyev[2],
				centerv[0], centerv[1], centerv[2],
				upv[0], upv[1], upv[2] );
	glGetFloatv( GL_MODELVIEW_MATRIX, viewMat );
	glPopMatrix();
	glPopAttrib();
}// Shift camera upward


void rotateBy(FLOAT *center2eye, FLOAT *center2right, FLOAT h){
	FLOAT upv[3], v1[3], v2[3], v3[3];
	VectorCross(center2eye, center2right, upv);
	VectorNorm(upv);
	VectorCross(upv, center2eye, center2right);
	VectorMul(center2eye, cos(h), v1);
	VectorMul(center2right, sin(h), v2);
	VectorAdd(v1, v2, v3);
	VectorMul(center2eye, -sin(h), v1);
	VectorMul(center2right, cos(h), v2);
	VectorAdd(v1, v2, center2right);
	VectorCopy(center2eye, v3);
}

void HV_rotation(FLOAT h, FLOAT v){
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Compute view matrix
	FLOAT m[16], m1[16], m2[16];
	FLOAT eyev[3] = { 0, 0, 1 };
	FLOAT centerv[3] = { 0, -0.33, 0 };
	FLOAT upv[3] = { 0, 1, 0 };
	FLOAT rightv[3] = { 1, 0, 0 };

	glLoadIdentity();
	gluLookAt(eyev[0], eyev[1], eyev[2], centerv[0], centerv[1], centerv[2], upv[0], upv[1], upv[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, m1);

	rotateBy(eyev, rightv, h);
	rotateBy(eyev, upv, v);

	glLoadIdentity();
	gluLookAt(eyev[0], eyev[1], eyev[2], centerv[0], centerv[1], centerv[2], upv[0], upv[1], upv[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, m2);

	InvertMatrix(m1, m1);
	glLoadMatrixf(m2);
	glMultMatrixf(m1);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);

	glPopMatrix();

	glGetFloatv(GL_MODELVIEW_MATRIX, m1);
	glLoadMatrixf(m);
	glMultMatrixf(m1);
	glPopAttrib();
}

void HV_shift(FLOAT h, FLOAT v){
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Compute view matrix
	FLOAT m[16], m1[16], m2[16];
	FLOAT eyev[3] = { 0, 0, 1 };
	FLOAT centerv[3] = { 0, -0.33, 0 };
	FLOAT upv[3] = { 0, 1, 0 };
	FLOAT rightv[3] = { 1, 0, 0 };

	glLoadIdentity();
	gluLookAt(eyev[0], eyev[1], eyev[2], centerv[0], centerv[1], centerv[2], upv[0], upv[1], upv[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, m1);

	rotateBy(eyev, rightv, h);
	rotateBy(eyev, upv, v);

	glLoadIdentity();
	gluLookAt(eyev[0], eyev[1], eyev[2], centerv[0], centerv[1], centerv[2], upv[0], upv[1], upv[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, m2);

	InvertMatrix(m1, m1);
	glLoadMatrixf(m1);
	glMultMatrixf(m2);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);

	glPopMatrix();

	glGetFloatv(GL_MODELVIEW_MATRIX, m1);
	glLoadMatrixf(m);
	glMultMatrixf(m1);
	glPopAttrib();
}

