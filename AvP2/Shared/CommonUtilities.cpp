// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CommonUtilities.h"
#include "basedefs_de.h"

// Model and transform interface pointers

MathLT*				g_pMathLT = DNULL;
ModelLT*			g_pModelLT = DNULL;
TransformLT*		g_pTransLT = DNULL;
PhysicsLT*			g_pPhysicsLT = DNULL;

// Temp buffer...

char s_FileBuffer[_MAX_PATH];



//-------------------------------------------------------------------------------------------
// WAVE FUNCTIONS.
//-------------------------------------------------------------------------------------------
WaveFn GetWaveFn(WaveType waveType)
{
	if(waveType == Wave_Linear)
		return WaveFn_Linear;
	else if(waveType == Wave_Sine)
		return WaveFn_Sine;
	else if(waveType == Wave_SlowOff)
		return WaveFn_SlowOff;
	else
		return WaveFn_SlowOn;
}

WaveType ParseWaveType(char *pStr)
{
	if(stricmp(pStr, WAVESTR_LINEAR) == 0)
	{
		return Wave_Linear;
	}
	else if(stricmp(pStr, WAVESTR_SINE) == 0)
	{
		return Wave_Sine;
	}
	else if(stricmp(pStr, WAVESTR_SLOWOFF) == 0)
	{
		return Wave_SlowOff;
	}
	else if(stricmp(pStr, WAVESTR_SLOWON) == 0)
	{
		return Wave_SlowOn;
	}
	else
	{
		// Default..
		return Wave_Linear;
	}
}

float WaveFn_Linear(float val)
{
	return val;
}

float WaveFn_Sine(float val)
{
	val = MATH_HALFPI + val * MATH_PI;	// PI/2 to 3PI/2
	val = (float)sin(val);				// 1 to -1
	val = -val;							// -1 to 1
	val = (val + 1.0f) * 0.5f;			// 0 to 1
	return val;
}

float WaveFn_SlowOff(float val)
{
	if(val < 0.5f)
		return WaveFn_Linear(val);
	else
		return WaveFn_Sine(val);
}

float WaveFn_SlowOn(float val)
{
	if(val < 0.5f)
		return WaveFn_Sine(val);
	else
		return WaveFn_Linear(val);
}


//-------------------------------------------------------------------------------------------
// Color255VectorToWord
//
// Converts a color in vector format to a word in 5-6-5 format.  Color ranges are 0-255.
// Arguments:
//		pVal - Color vector
// Return:
//		D_WORD - converted color.
//-------------------------------------------------------------------------------------------
D_WORD Color255VectorToWord( DVector *pVal )
{
	D_WORD wColor;

	// For red, multiply by 5 bits and divide by 8, which is a net division of 3 bits.  Then shift it
	// to the left 11 bits to fit into result, which is a net shift of 8 to left.
	wColor = ( D_WORD )(((( DDWORD )pVal->x & 0xFF ) << 8 ) & 0xF800 );

	// For green, multiply by 6 bits and divide by 8, which is a net division of 2 bits.  Then shift it
	// to the left 5 bits to fit into result, which is a net shift of 3 to left.
	wColor |= ( D_WORD )(((( DDWORD )pVal->y & 0xFF ) << 3 ) & 0x07E0 );

	// For blue, multiply by 5 bits and divide by 8 = divide by 3.
	wColor |= ( D_WORD )((( DDWORD )pVal->z & 0xFF ) >> 3 );

	return wColor;
}

//-------------------------------------------------------------------------------------------
// Color255WordToVector
//
// Converts a color in word format to a vector in 5-6-5 format.  Color ranges are 0-255.
// Arguments:
//		wVal - color word
//		pVal - Color vector
// Return:
//		void
//-------------------------------------------------------------------------------------------
void Color255WordToVector( D_WORD wVal, DVector *pVal )
{
	// For red, divide by 11 bits then multiply by 8 bits and divide by 5 bits = divide by 8 bits...
	pVal->x = ( DFLOAT )(( wVal & 0xF800 ) >> 8 );

	// For green, divide by 5 bits, multiply by 8 bits, divide by 6 bits = divide by 3 bits.
	pVal->y = ( DFLOAT )(( wVal & 0x07E0 ) >> 3 );

	// For blue, divide by 5 bits, multiply by 8 bits = multiply by 3 bits
	pVal->z = ( DFLOAT )(( wVal & 0x001F ) << 3 );
}

DBYTE CompressRotationByte(CommonLT *pCommon, DRotation *pRotation)
{
	DVector up, right, forward;
	float angle;
	char cAngle;

	pCommon->GetRotationVectors(*pRotation, up, right, forward);

	angle = (float)atan2(forward.x, forward.z);
	cAngle = (char)(angle * (127.0f / MATH_PI));
	return (DBYTE)cAngle;
}


void UncompressRotationByte(CommonLT *pCommon, DBYTE rot, DRotation *pRotation)
{
	float angle;

	angle = (float)(char)rot / 127.0f;
	angle *= MATH_PI;
	pCommon->SetupEuler(*pRotation, 0.0f, angle, 0.0f);
}

//----------------------------------------------------------------------
//
//	Misc functions
// 
 
ActivateType GetActivateType(HOBJECT hObj)
{
	uint32 dwFlags;

	#ifdef _CLIENTBUILD
		g_pLTClient->GetObjectUserFlags(hObj, &dwFlags);
	#else
		dwFlags = g_pLTServer->GetObjectUserFlags(hObj);
	#endif

	dwFlags >>= AT_BITSHIFT;
	dwFlags &= 0xf;
	return (ActivateType) dwFlags;
}

void SetActivateType(HOBJECT hObj, ActivateType eType)
{
	uint32 dwFlags;
	uint32 dwType = (uint32) eType;

	#ifdef _CLIENTBUILD
		g_pLTClient->GetObjectUserFlags(hObj, &dwFlags);
	#else
		dwFlags = g_pLTServer->GetObjectUserFlags(hObj);
	#endif

	//do the mask to be sure we are not messing with any other bits
	dwType &= 0xf;
	dwType <<= AT_BITSHIFT;

	//be sure to clear all the old flags
	dwFlags &= ~(0xf << AT_BITSHIFT);

	#ifdef _CLIENTBUILD
		g_pLTClient->SetObjectUserFlags(hObj, dwFlags | dwType);
	#else
		g_pLTServer->SetObjectUserFlags(hObj, dwFlags | dwType);
	#endif
}


