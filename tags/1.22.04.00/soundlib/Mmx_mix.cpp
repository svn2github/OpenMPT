/*
 * MMX_Mix.cpp
 * -----------
 * Purpose: Accelerated mixer code for rendering samples.
 * Notes  : 
 *          x86 ( AMD/INTEL ) based low level based mixing functions:
 *          This file contains critical code. The basic X86 functions are
 *          defined at the bottom of the file. #define's are used to isolate
 *          the different flavours of functionality:
 *          ENABLE_MMX, ENABLE_3DNOW, ENABLE_SSE flags must be set to
 *          to compile the optimized sections of the code. In both cases the 
 *          X86_xxxxxx functions will compile.
 *
 *          NOTE: Resonant filter mixing has been changed to use floating point
 *          precision. The MMX code hasn't been updated for this, so the filtered
 *          MMX functions mustn't be used anymore!
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"


////////////////////////////////////////////////////////////////////////////////////
// 3DNow! optimizations

#ifdef ENABLE_X86_AMD

// Convert integer mix to floating-point
static void AMD_StereoMixToFloat(const int *pSrc, float *pOut1, float *pOut2, UINT nCount, const float _i2fc)
//-----------------------------------------------------------------------------------------------------------
{
	_asm {
	movd mm0, _i2fc
	mov edx, pSrc
	mov edi, pOut1
	mov ebx, pOut2
	mov ecx, nCount
	punpckldq mm0, mm0
	inc ecx
	shr ecx, 1
mainloop:
	movq mm1, qword ptr [edx]
	movq mm2, qword ptr [edx+8]
	add edi, 8
	pi2fd mm1, mm1
	pi2fd mm2, mm2
	add ebx, 8
	pfmul mm1, mm0
	pfmul mm2, mm0
	add edx, 16
	movq mm3, mm1
	punpckldq mm3, mm2
	punpckhdq mm1, mm2
	dec ecx
	movq qword ptr [edi-8], mm3
	movq qword ptr [ebx-8], mm1
	jnz mainloop
	emms
	}
}

static void AMD_FloatToStereoMix(const float *pIn1, const float *pIn2, int *pOut, UINT nCount, const float _f2ic)
//---------------------------------------------------------------------------------------------------------------
{
	_asm {
	movd mm0, _f2ic
	mov eax, pIn1
	mov ebx, pIn2
	mov edx, pOut
	mov ecx, nCount
	punpckldq mm0, mm0
	inc ecx
	shr ecx, 1
	sub edx, 16
mainloop:
	movq mm1, [eax]
	movq mm2, [ebx]
	add edx, 16
	movq mm3, mm1
	punpckldq mm1, mm2
	punpckhdq mm3, mm2
	add eax, 8
	pfmul mm1, mm0
	pfmul mm3, mm0
	add ebx, 8
	pf2id mm1, mm1
	pf2id mm3, mm3
	dec ecx
	movq qword ptr [edx], mm1
	movq qword ptr [edx+8], mm3
	jnz mainloop
	emms
	}
}


static void AMD_FloatToMonoMix(const float *pIn, int *pOut, UINT nCount, const float _f2ic)
//-----------------------------------------------------------------------------------------
{
	_asm {
	movd mm0, _f2ic
	mov eax, pIn
	mov edx, pOut
	mov ecx, nCount
	punpckldq mm0, mm0
	add ecx, 3
	shr ecx, 2
	sub edx, 16
mainloop:
	movq mm1, [eax]
	movq mm2, [eax+8]
	add edx, 16
	pfmul mm1, mm0
	pfmul mm2, mm0
	add eax, 16
	pf2id mm1, mm1
	pf2id mm2, mm2
	dec ecx
	movq qword ptr [edx], mm1
	movq qword ptr [edx+8], mm2
	jnz mainloop
	emms
	}
}


static void AMD_MonoMixToFloat(const int *pSrc, float *pOut, UINT nCount, const float _i2fc)
//------------------------------------------------------------------------------------------
{
	_asm {
	movd mm0, _i2fc
	mov eax, pSrc
	mov edx, pOut
	mov ecx, nCount
	punpckldq mm0, mm0
	add ecx, 3
	shr ecx, 2
	sub edx, 16
mainloop:
	movq mm1, qword ptr [eax]
	movq mm2, qword ptr [eax+8]
	add edx, 16
	pi2fd mm1, mm1
	pi2fd mm2, mm2
	add eax, 16
	pfmul mm1, mm0
	pfmul mm2, mm0
	dec ecx
	movq qword ptr [edx], mm1
	movq qword ptr [edx+8], mm2
	jnz mainloop
	emms
	}
}

#endif // ENABLE_X86_AMD

///////////////////////////////////////////////////////////////////////////////////////
// SSE Optimizations

#ifdef ENABLE_SSE

static void SSE_StereoMixToFloat(const int *pSrc, float *pOut1, float *pOut2, UINT nCount, const float _i2fc)
//-----------------------------------------------------------------------------------------------------------
{
	_asm {
	movss xmm0, _i2fc
	mov edx, pSrc
	mov eax, pOut1
	mov ebx, pOut2
	mov ecx, nCount
	shufps xmm0, xmm0, 0x00
	xorps xmm1, xmm1
	xorps xmm2, xmm2
	inc ecx
	shr ecx, 1
mainloop:
	cvtpi2ps xmm1, [edx]
	cvtpi2ps xmm2, [edx+8]
	add eax, 8
	add ebx, 8
	movlhps xmm1, xmm2
	mulps xmm1, xmm0
	add edx, 16
	shufps xmm1, xmm1, 0xD8
	dec ecx
	movlps qword ptr [eax-8], xmm1
	movhps qword ptr [ebx-8], xmm1
	jnz mainloop
	}
}


static void SSE_MonoMixToFloat(const int *pSrc, float *pOut, UINT nCount, const float _i2fc)
//------------------------------------------------------------------------------------------
{
	_asm {
	movss xmm0, _i2fc
	mov edx, pSrc
	mov eax, pOut
	mov ecx, nCount
	shufps xmm0, xmm0, 0x00
	xorps xmm1, xmm1
	xorps xmm2, xmm2
	add ecx, 3
	shr ecx, 2
mainloop:
	cvtpi2ps xmm1, [edx]
	cvtpi2ps xmm2, [edx+8]
	add eax, 16
	movlhps xmm1, xmm2
	mulps xmm1, xmm0
	add edx, 16
	dec ecx
	movups [eax-16], xmm1
	jnz mainloop
	}
}

#endif // ENABLE_SSE



#ifdef ENABLE_X86

// Convert floating-point mix to integer

static void X86_FloatToStereoMix(const float *pIn1, const float *pIn2, int *pOut, UINT nCount, const float _f2ic)
//---------------------------------------------------------------------------------------------------------------
{
	_asm {
	mov esi, pIn1
	mov ebx, pIn2
	mov edi, pOut
	mov ecx, nCount
	fld _f2ic
mainloop:
	fld dword ptr [ebx]
	add edi, 8
	fld dword ptr [esi]
	add ebx, 4
	add esi, 4
	fmul st(0), st(2)
	fistp dword ptr [edi-8]
	fmul st(0), st(1)
	fistp dword ptr [edi-4]
	dec ecx
	jnz mainloop
	fstp st(0)
	}
}


// Convert integer mix to floating-point

static void X86_StereoMixToFloat(const int *pSrc, float *pOut1, float *pOut2, UINT nCount, const float _i2fc)
//-----------------------------------------------------------------------------------------------------------
{
	_asm {
	mov esi, pSrc
	mov edi, pOut1
	mov ebx, pOut2
	mov ecx, nCount
	fld _i2fc
mainloop:
	fild dword ptr [esi]
	fild dword ptr [esi+4]
	add ebx, 4
	add edi, 4
	fmul st(0), st(2)
	add esi, 8
	fstp dword ptr [ebx-4]
	fmul st(0), st(1)
	fstp dword ptr [edi-4]
	dec ecx
	jnz mainloop
	fstp st(0)
	}
}


static void X86_FloatToMonoMix(const float *pIn, int *pOut, UINT nCount, const float _f2ic)
//-----------------------------------------------------------------------------------------
{
	_asm {
	mov edx, pIn
	mov eax, pOut
	mov ecx, nCount
	fld _f2ic
	sub eax, 4
R2I_Loop:
	fld DWORD PTR [edx]
	add eax, 4
	fmul ST(0), ST(1)
	dec ecx
	lea edx, [edx+4]
	fistp DWORD PTR [eax]
	jnz R2I_Loop
	fstp st(0)
	}
}


static void X86_MonoMixToFloat(const int *pSrc, float *pOut, UINT nCount, const float _i2fc)
//------------------------------------------------------------------------------------------
{
	_asm {
	mov edx, pOut
	mov eax, pSrc
	mov ecx, nCount
	fld _i2fc
	sub edx, 4
I2R_Loop:
	fild DWORD PTR [eax]
	add edx, 4
	fmul ST(0), ST(1)
	dec ecx
	lea eax, [eax+4]
	fstp DWORD PTR [edx]
	jnz I2R_Loop
	fstp st(0)
	}
}

#endif // ENABLE_X86



static void C_FloatToStereoMix(const float *pIn1, const float *pIn2, int *pOut, UINT nCount, const float _f2ic)
//-------------------------------------------------------------------------------------------------------------
{
	for(UINT i=0; i<nCount; ++i)
	{
		*pOut++ = (int)(*pIn1++ * _f2ic);
		*pOut++ = (int)(*pIn2++ * _f2ic);
	}
}


static void C_StereoMixToFloat(const int *pSrc, float *pOut1, float *pOut2, UINT nCount, const float _i2fc)
//---------------------------------------------------------------------------------------------------------
{
	for(UINT i=0; i<nCount; ++i)
	{
		*pOut1++ = *pSrc++ * _i2fc;
		*pOut2++ = *pSrc++ * _i2fc;
	}
}


static void C_FloatToMonoMix(const float *pIn, int *pOut, UINT nCount, const float _f2ic)
//---------------------------------------------------------------------------------------
{
	for(UINT i=0; i<nCount; ++i)
	{
		*pOut++ = (int)(*pIn++ * _f2ic);
	}
}


static void C_MonoMixToFloat(const int *pSrc, float *pOut, UINT nCount, const float _i2fc)
//----------------------------------------------------------------------------------------
{
	for(UINT i=0; i<nCount; ++i)
	{
		*pOut++ = *pSrc++ * _i2fc;
	}
}



void StereoMixToFloat(const int *pSrc, float *pOut1, float *pOut2, UINT nCount, const float _i2fc)
{

	#ifdef ENABLE_SSE
		if(GetProcSupport() & PROCSUPPORT_SSE)
		{
			SSE_StereoMixToFloat(pSrc, pOut1, pOut2, nCount, _i2fc);
			return;
		}
	#endif // ENABLE_SSE
	#ifdef ENABLE_X86_AMD
		if(GetProcSupport() & PROCSUPPORT_AMD_3DNOW)
		{
			AMD_StereoMixToFloat(pSrc, pOut1, pOut2, nCount, _i2fc);
			return;
		}
	#endif // ENABLE_X86_AMD

	#ifdef ENABLE_X86
		X86_StereoMixToFloat(pSrc, pOut1, pOut2, nCount, _i2fc);
	#else // !ENABLE_X86
		C_StereoMixToFloat(pSrc, pOut1, pOut2, nCount, _i2fc);
	#endif // ENABLE_X86

}


void FloatToStereoMix(const float *pIn1, const float *pIn2, int *pOut, UINT nCount, const float _f2ic)
{

	#ifdef ENABLE_X86_AMD
		if(GetProcSupport() & PROCSUPPORT_AMD_3DNOW)
		{
			AMD_FloatToStereoMix(pIn1, pIn2, pOut, nCount, _f2ic);
			return;
		}
	#endif // ENABLE_X86_AMD

	#ifdef ENABLE_X86
		X86_FloatToStereoMix(pIn1, pIn2, pOut, nCount, _f2ic);
	#else // !ENABLE_X86
		C_FloatToStereoMix(pIn1, pIn2, pOut, nCount, _f2ic);
	#endif // ENABLE_X86

}


void MonoMixToFloat(const int *pSrc, float *pOut, UINT nCount, const float _i2fc)
{

	#ifdef ENABLE_SSE
		if(GetProcSupport() & PROCSUPPORT_SSE)
		{
			SSE_MonoMixToFloat(pSrc, pOut, nCount, _i2fc);
			return;
		}
	#endif // ENABLE_SSE
	#ifdef ENABLE_X86_AMD
		if(GetProcSupport() & PROCSUPPORT_AMD_3DNOW)
		{
			AMD_MonoMixToFloat(pSrc, pOut, nCount, _i2fc);
			return;
		}
	#endif // ENABLE_X86_AMD

	#ifdef ENABLE_X86
		X86_MonoMixToFloat(pSrc, pOut, nCount, _i2fc);
	#else // !ENABLE_X86
		C_MonoMixToFloat(pSrc, pOut, nCount, _i2fc);
	#endif // ENABLE_X86

}


void FloatToMonoMix(const float *pIn, int *pOut, UINT nCount, const float _f2ic)
{

	#ifdef ENABLE_X86_AMD
		if(GetProcSupport() & PROCSUPPORT_AMD_3DNOW)
		{
			AMD_FloatToMonoMix(pIn, pOut, nCount, _f2ic);
			return;
		}
	#endif // ENABLE_X86_AMD

	#ifdef ENABLE_X86
		X86_FloatToMonoMix(pIn, pOut, nCount, _f2ic);
	#else // !ENABLE_X86
		C_FloatToMonoMix(pIn, pOut, nCount, _f2ic);
	#endif // ENABLE_X86

}

