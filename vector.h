#ifndef	__VECTOR_H
#define	__VECTOR_H

#include "math.h"

#ifndef	__FLOAT_MIN_DIFF
#define	__FLOAT_MIN_DIFF	0.000003f
#endif

#ifndef	__DOUBLE_MIN_DIFF
#define	__DOUBLE_MIN_DIFF	0.000000000009
#endif

class	Vector3f;
class	Vector4f;

class Vector3f{
public:
	float		val[3];
	bool		operator	==	(Vector3f&);		//exact comparison (vector)
	bool		operator	==	(float);			//inexact comparison (magnitude)
	bool		operator	<	(Vector3f&);		//exact comparison (magnitude)
	bool		operator	>	(Vector3f&);		//exact comparison (magnitude)
	bool		operator	!=	(Vector3f&);		//inexact comparison (vector)
	bool		operator	<<	(Vector3f&);		//inexact comparison (magnitude)
	bool		operator	>>	(Vector3f&);		//inexact comparison (magnitude)
	Vector3f&	operator	=	(Vector3f&);
	Vector3f&	operator	<<=	(float);			//normalize vector
	float	&	operator	>>=	(float&);			//calculate magnitude
	Vector3f&	operator	+=	(Vector3f&);
	Vector3f&	operator	-=	(Vector3f&);
	Vector3f&	operator	^=	(Vector3f&);
	Vector3f&	operator	*=	(float);			//constant product
	Vector3f&	operator	/=	(float);			//constant product
	Vector3f	operator	+	(Vector3f&);
	Vector3f	operator	-	(Vector3f&);
	Vector3f	operator	^	(Vector3f&);		//cross product
	Vector3f	operator	*	(float);			//constant product
	Vector3f	operator	/	(float);			//constant product
	float		operator	*	(Vector3f&);		//dot product
	Vector3f(){}
	Vector3f(float x,float y,float z){
		val[0] = x;
		val[1] = y;
		val[2] = z;
	}
	Vector3f(float*p){
		memcpy(val,p,sizeof(Vector3f));
	}
};
bool	Vector3f::operator == (Vector3f& _rhs){
	return	memcmp(this,&_rhs,sizeof(float)*3)==0;
}
bool	Vector3f::operator == (float _rhs){
	float	norm;
	*this >>= norm;
	return	fabsf(norm-_rhs) < __FLOAT_MIN_DIFF;
}
bool	Vector3f::operator < (Vector3f& _rhs){
	float	lhs, rhs;
	*this >>= lhs;
	_rhs  >>= rhs;
	return	lhs<rhs;
}
bool	Vector3f::operator > (Vector3f& _rhs){
	float	lhs, rhs;
	*this >>= lhs;
	_rhs  >>= rhs;
	return	lhs>rhs;
}
bool	Vector3f::operator << (Vector3f& _rhs){
	float	lhs, rhs;
	*this >>= lhs;
	_rhs  >>= rhs;
	return	(rhs-lhs)>__FLOAT_MIN_DIFF;
}
bool	Vector3f::operator >> (Vector3f& _rhs){
	float	lhs, rhs;
	*this >>= lhs;
	_rhs  >>= rhs;
	return	(lhs-rhs)>__FLOAT_MIN_DIFF;
}
bool	Vector3f::operator != (Vector3f& _rhs){
	if(fabsf(this->val[0]-_rhs.val[0]) > __FLOAT_MIN_DIFF)return true;
	if(fabsf(this->val[1]-_rhs.val[1]) > __FLOAT_MIN_DIFF)return true;
	if(fabsf(this->val[2]-_rhs.val[2]) > __FLOAT_MIN_DIFF)return true;
	return	false;
}
Vector3f& Vector3f::operator = (Vector3f& _rhs){
	memcpy(this,&_rhs,sizeof(Vector3f));
	return	*this;
}
Vector3f& Vector3f::operator <<= (float _rhs){
	float	_mag;
	*this >>= _mag;
	_mag /=	_rhs;
	*this /= _mag;
	return	*this;
}
float& Vector3f::operator >>= (float& _rhs){
	float	_res[4];
#ifdef SIMD
	__asm{
		mov		eax,	this
		movups	xmm0,	[eax]
		movaps	xmm1,	xmm0
		mulps	xmm0,	xmm1
		movups	_res,	xmm0
	}
#else
	_res[0] = this->val[0]*this->val[0];
	_res[1] = this->val[1]*this->val[1];
	_res[2] = this->val[2]*this->val[2];
#endif
	_rhs = sqrtf(_res[0]+_res[1]+_res[2]);
	return	_rhs;
}
Vector3f& Vector3f::operator +=	(Vector3f& _rhs){
#ifdef SIMD
	__asm{
		mov		eax,	this
		mov		ebx,	_rhs
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		addps	xmm0,	xmm1
		movlps	[eax],	xmm0
		movhlps	xmm0,	xmm0
		movss	[eax+8],xmm0
	}
#else
	this->val[0] += _rhs.val[0];
	this->val[1] += _rhs.val[1];
	this->val[2] += _rhs.val[2];
#endif
	return	*this;
}
Vector3f& Vector3f::operator -=	(Vector3f& _rhs){
#ifdef SIMD
	__asm{
		mov		eax,	this
		mov		ebx,	_rhs
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		subps	xmm0,	xmm1
		movlps	[eax],	xmm0
		movhlps	xmm0,	xmm0
		movss	[eax+8],xmm0
	}
#else
	this->val[0] -= _rhs.val[0];
	this->val[1] -= _rhs.val[1];
	this->val[2] -= _rhs.val[2];
#endif
	return	*this;
}
Vector3f& Vector3f::operator *=	(float _rhs){
#ifdef SIMD
	__asm{
		movss	xmm1,	_rhs
		mov		eax,	this
		unpcklps	xmm1,	xmm1
		movups	xmm0,	[eax]
		movlhps	xmm1,	xmm1
		mulps	xmm0,	xmm1
		movlps	[eax],	xmm0
		movhlps	xmm0,	xmm0
		movss	[eax+8],xmm0
	}
#else
	this->val[0] *= _rhs;
	this->val[1] *= _rhs;
	this->val[2] *= _rhs;
#endif
	return	*this;
}
Vector3f& Vector3f::operator /=	(float _rhs){
#ifdef SIMD
	__asm{
		movss	xmm1,	_rhs
		mov		eax,	this
		unpcklps	xmm1,	xmm1
		movups	xmm0,	[eax]
		movlhps	xmm1,	xmm1
		divps	xmm0,	xmm1
		movlps	[eax],	xmm0
		movhlps	xmm0,	xmm0
		movss	[eax+8],xmm0
	}
#else
	this->val[0] *= _rhs;
	this->val[1] *= _rhs;
	this->val[2] *= _rhs;
#endif
	return	*this;
}
Vector3f& Vector3f::operator ^=	(Vector3f& _rhs){
#ifdef SIMD
	__asm{
		mov		ebx,	[_rhs]
		mov		eax,	[this]
		movups	xmm3,	[ebx]
		movups	xmm0,	[eax]
		movaps	xmm1,	xmm3
		movaps	xmm2,	xmm0
		psrldq	xmm1,	4
		psrldq	xmm2,	4
		movhps	xmm1,	[ebx]
		movhps	xmm2,	[eax]
		mulps	xmm0,	xmm1
		mulps	xmm2,	xmm3
		subps	xmm0,	xmm2
		movss	[eax+8],xmm0
		psrldq	xmm0,	4
		movlps	[eax],	xmm0
	}
#else	
	Vector3f _tmp = *this;
	this->val[0] = _tmp.val[1]*_rhs.val[2]-_tmp.val[2]*_rhs.val[1];
	this->val[1] = _tmp.val[2]*_rhs.val[0]-_tmp.val[0]*_rhs.val[2];
	this->val[2] = _tmp.val[0]*_rhs.val[1]-_tmp.val[1]*_rhs.val[0];
#endif
	return	*this;
}
Vector3f Vector3f::operator + (Vector3f& _rhs){
	Vector3f	_sum;
	float		_pad;
#ifdef SIMD
	__asm{
		mov		eax,	this
		mov		ebx,	_rhs
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		addps	xmm0,	xmm1
		movups	[_sum],	xmm0
	}
#else
	_sum = *this;
	_sum.val[0] += _rhs.val[0];
	_sum.val[1] += _rhs.val[1];
	_sum.val[2] += _rhs.val[2];
#endif
	return	_sum;
}
Vector3f Vector3f::operator - (Vector3f& _rhs){
	Vector3f	_sum;
	float		_pad;
#ifdef SIMD
	__asm{
		mov		eax,	this
		mov		ebx,	_rhs
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		subps	xmm0,	xmm1
		movups	[_sum],	xmm0
	}
#else
	_sum = *this;
	_sum.val[0] -= _rhs.val[0];
	_sum.val[1] -= _rhs.val[1];
	_sum.val[2] -= _rhs.val[2];
#endif
	return	_sum;
}
Vector3f Vector3f::operator ^ (Vector3f& _rhs){
	Vector3f _sum;
#ifdef SIMD
	__asm{
		mov		ebx,	[_rhs]
		mov		eax,	[this]
		movups	xmm3,	[ebx]
		movups	xmm0,	[eax]
		movaps	xmm1,	xmm3
		movaps	xmm2,	xmm0
		psrldq	xmm1,	4
		psrldq	xmm2,	4
		movhps	xmm1,	[ebx]
		movhps	xmm2,	[eax]
		mulps	xmm0,	xmm1
		mulps	xmm2,	xmm3
		subps	xmm0,	xmm2
		movss	[_sum+8],xmm0
		psrldq	xmm0,	4
		movlps	[_sum],	xmm0
	}
#else	
	_sum.val[0] = this->val[1]*_rhs.val[2]-this->val[2]*_rhs.val[1];
	_sum.val[1] = this->val[2]*_rhs.val[0]-this->val[0]*_rhs.val[2];
	_sum.val[2] = this->val[0]*_rhs.val[1]-this->val[1]*_rhs.val[0];
#endif
	return	_sum;
}
Vector3f Vector3f::operator * (float _rhs){
	Vector3f	_sum;
	float		_pad;
#ifdef SIMD
	__asm{
		movss	xmm1,	_rhs
		mov		eax,	this
		unpcklps	xmm1,	xmm1
		movups	xmm0,	[eax]
		movlhps	xmm1,	xmm1
		mulps	xmm0,	xmm1
		movups	[_sum],	xmm0
	}
#else
	_sum.val[0] = this->val[0]*_rhs;
	_sum.val[1] = this->val[1]*_rhs;
	_sum.val[2] = this->val[2]*_rhs;
#endif
	return	_sum;
}
Vector3f Vector3f::operator / (float _rhs){
	Vector3f	_sum;
	float		_pad;
#ifdef SIMD
	__asm{
		movss	xmm1,	_rhs
		mov		eax,	this
		unpcklps	xmm1,	xmm1
		movups	xmm0,	[eax]
		movlhps	xmm1,	xmm1
		divps	xmm0,	xmm1
		movups	[_sum],	xmm0
	}
#else
	_sum.val[0] = this->val[0]/_rhs;
	_sum.val[1] = this->val[1]/_rhs;
	_sum.val[2] = this->val[2]/_rhs;
#endif
	return	_sum;
}
float Vector3f::operator * (Vector3f& _rhs){
#ifdef SIMD
	float	res[4];
	__asm{
		mov		eax,	[this]
		mov		ebx,	[_rhs]
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		mulps	xmm0,	xmm1
		movups	[res],	xmm0
	}
	return	res[0]+res[1]+res[2];
#else
	return	 (this->val[0] * _rhs.val[0])
			+(this->val[1] * _rhs.val[1])
			+(this->val[2] * _rhs.val[2]);
#endif
}


/*
//For 4-Vectors
class Vector4f{
public:
	float val[4];
	operator	<<=		(Vector4f&);
	operator	>>=		(Vector4f&);
	bool		operator == (Vector3f&);
	Vector3f	&		operator =	(Vector4f&);
	Vector3f	&		operator += (Vector4f&);
	Vector3f	&		operator -= (Vector4f&);
	Vector3f	&		operator ^= (Vector4f&);
	Vector3f	&		operator *= (float&);
	Vector3f	&		operator /= (float&);
	Vector3f	operator + (Vector4f&);
	Vector3f	operator - (Vector4f&);
	Vector3f	operator ^ (Vector4f&);		//cross product
	Vector3f	operator * (float);			//constant product
	Vector3f	operator / (float);			//constant product
	float		operator * (Vector4f&);		//dot product
};
Vector3f::operator	<<=	(Vector4f& _rhs){
	memcpy(this,&_rhs,sizeof(float)*4);
}
bool	Vector3f::operator == (Vector4f& _rhs){
	return memcmp(this,&_rhs,sizeof(float)*4)==0;
}
Vector4f& Vector4f::operator =	(Vector4f& _rhs){
#ifdef SIMD
	__asm{
		mov		eax,	_rhs
		movups	xmm0,	[eax]
		mov		eax,	this
		movups	[eax],	xmm0
	}
#else
	if(this == &_rhs)return *this;
	memcpy(this,&_rhs,sizeof(Vector4f));
#endif
	return	*this;
}
Vector4f& Vector4f::operator +=	(Vector4f& _rhs){
#ifdef SIMD
	__asm{
		mov		eax,	_rhs
		movups	xmm1,	[eax]
		mov		eax,	this
		movups	xmm0,	[eax]
		addps	xmm0,	xmm1
		movups	[eax],	xmm0
	}
#else
	this->val[0] += _rhs.val[0];
	this->val[1] += _rhs.val[1];
	this->val[2] += _rhs.val[2];
#endif
	return	*this;
}
Vector4f& Vector4f::operator -=	(Vector4f& _rhs){
#ifdef SIMD
	__asm{
		mov		eax,	_rhs
		movups	xmm1,	[eax]
		mov		eax,	this
		movups	xmm0,	[eax]
		subps	xmm0,	xmm1
		movups	[eax],	xmm0
	}
#else
	this->val[0] -= _rhs.val[0];
	this->val[1] -= _rhs.val[1];
	this->val[2] -= _rhs.val[2];
#endif
	return	*this;
}
Vector4f& Vector4f::operator *=	(float& _rhs){
	this->val[0] *= _rhs;
	this->val[1] *= _rhs;
	this->val[2] *= _rhs;
	return	*this;
}
Vector4f& Vector4f::operator /=	(float& _rhs){
	this->val[0] /= _rhs;
	this->val[1] /= _rhs;
	this->val[2] /= _rhs;
	return	*this;
}
Vector4f& Vector4f::operator ^=	(Vector4f& _rhs){
#ifdef SIMD
	__asm{
		mov		ebx,	[_rhs]
		mov		eax,	[this]
		movups	xmm4,	[ebx]
		movups	xmm0,	[eax]
		movaps	xmm1,	xmm4
		movaps	xmm2,	xmm0
		psrldq	xmm1,	4
		psrldq	xmm2,	4
		movhps	xmm1,	[ebx]
		movhps	xmm2,	[eax]
		mulps	xmm0,	xmm1
		mulps	xmm2,	xmm4
		subps	xmm0,	xmm2
		movss	[eax+8],xmm0
		psrldq	xmm0,	4
		movlps	[eax],	xmm0
	}
#else	
	Vector4f _tmp = *this;
	this->val[0] = _tmp.val[1]*_rhs.val[2]-_tmp.val[2]*_rhs.val[1];
	this->val[1] = _tmp.val[2]*_rhs.val[0]-_tmp.val[0]*_rhs.val[2];
	this->val[2] = _tmp.val[0]*_rhs.val[1]-_tmp.val[1]*_rhs.val[0];
#endif
	return	*this;
}
Vector4f Vector4f::operator + (Vector4f& _rhs){
	Vector4f _sum = *this;
#ifdef SIMD
	__asm{
		mov		eax,	_rhs
		movups	xmm1,	[eax]
		movups	xmm0,	[_sum]
		addps	xmm0,	xmm1
		movups	[_sum],	xmm0
	}
#else
	_sum.val[0] += _rhs.val[0];
	_sum.val[1] += _rhs.val[1];
	_sum.val[2] += _rhs.val[2];
#endif
	return	_sum;
}
Vector4f Vector4f::operator - (Vector4f& _rhs){
	Vector4f _sum = *this;
#ifdef SIMD
	__asm{
		mov		eax,	_rhs
		movups	xmm1,	[eax]
		movups	xmm0,	[_sum]
		subps	xmm0,	xmm1
		movups	[_sum],	xmm0
	}
#else
	_sum.val[0] -= _rhs.val[0];
	_sum.val[1] -= _rhs.val[1];
	_sum.val[2] -= _rhs.val[2];
#endif
	return	_sum;
}
Vector4f Vector4f::operator ^ (Vector4f& _rhs){
	Vector4f _sum;
#ifdef SIMD
	__asm{
		mov		ebx,	[_rhs]
		mov		eax,	[this]
		movups	xmm4,	[ebx]
		movups	xmm0,	[eax]
		movaps	xmm1,	xmm4
		movaps	xmm2,	xmm0
		psrldq	xmm1,	4
		psrldq	xmm2,	4
		movhps	xmm1,	[ebx]
		movhps	xmm2,	[eax]
		mulps	xmm0,	xmm1
		mulps	xmm2,	xmm4
		subps	xmm0,	xmm2
		movss	[_sum+8],xmm0
		psrldq	xmm0,	4
		movlps	[_sum],	xmm0
	}
#else	
	_sum.val[0] = this->val[1]*_rhs.val[2]-this->val[2]*_rhs.val[1];
	_sum.val[1] = this->val[2]*_rhs.val[0]-this->val[0]*_rhs.val[2];
	_sum.val[2] = this->val[0]*_rhs.val[1]-this->val[1]*_rhs.val[0];
#endif
	return	_sum;
}
Vector4f Vector4f::operator * (float _rhs){
	Vector4f _sum;
	_sum.val[0] = this->val[0]*_rhs;
	_sum.val[1] = this->val[1]*_rhs;
	_sum.val[2] = this->val[2]*_rhs;
	return	_sum;
}
Vector4f Vector4f::operator / (float _rhs){
	Vector4f _sum;
	_sum.val[0] = this->val[0]/_rhs;
	_sum.val[1] = this->val[1]/_rhs;
	_sum.val[2] = this->val[2]/_rhs;
	return	_sum;
}
float Vector4f::operator * (Vector4f& _rhs){
#ifdef SIMD
	__asm{
		mov		eax,	[_rhs]
		movups	xmm1,	[eax]
		mov		eax,	[this]
		movups	xmm0,	[eax]
		mulps	xmm0,	xmm1
		movups	[eax],	xmm0
	}
	return	this->val[0]+this->val[1]+this->val[2];
#else
	return	 (this->val[0] * _rhs.val[0])
			+(this->val[1] * _rhs.val[1])
			+(this->val[2] * _rhs.val[2]);
#endif
}
*/
#endif