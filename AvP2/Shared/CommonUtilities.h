// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 5/4/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMON_UTILITIES_H__
#define __COMMON_UTILITIES_H__

#include "basetypes_de.h"
#include "common_de.h"
#include "modellt.h"
#include "physics_lt.h"
#include "transformlt.h"
#include "ClientServerShared.h"

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

#define WAVESTR_LINEAR	"LINEAR"
#define WAVESTR_SINE	"SINE"
#define WAVESTR_SLOWOFF	"SLOWOFF"
#define WAVESTR_SLOWON	"SLOWON"

#define DEFAULT_STAIRSTEP_HEIGHT	25.0

// Externs

extern MathLT*	g_pMathLT;
extern ModelLT*	g_pModelLT;
extern TransformLT*	g_pTransLT;
extern PhysicsLT* g_pPhysicsLT;

// Wave types.
typedef enum
{
	Wave_Linear=0,
	Wave_Sine,
	Wave_SlowOff,
	Wave_SlowOn,
	NUM_WAVETYPES
} WaveType;

typedef float (*WaveFn)(float val);


// Wave functions, pass in a # 0-1 and they return a number 0-1.
float WaveFn_Linear(float val);
float WaveFn_Sine(float val);
float WaveFn_SlowOff(float val);
float WaveFn_SlowOn(float val);

// Guaranteed to return a value function (one of the wave functions).
WaveFn GetWaveFn(WaveType type);

// Should be used by objects allowing the wave type to be specified.
WaveType ParseWaveType(char *pStr);

// Get the activate type for an object
ActivateType	GetActivateType(HOBJECT hObj);
void			SetActivateType(HOBJECT hObj, ActivateType eType);

D_WORD Color255VectorToWord( DVector *pVal );
void Color255WordToVector( D_WORD wVal, DVector *pVal );

inline int GetRandom() { return(rand()); }

inline int GetRandom(int range)
{
	if (range == -1)	// check for divide-by-zero case
	{
		return((rand() % 2) - 1);
	}
		
	return(rand() % (range + 1));
}

inline int GetRandom(int lo, int hi)
{
	if ((hi - lo + 1) == 0)		// check for divide-by-zero case
	{
		if (rand() & 1) return(lo);
		else return(hi);
	}

	return((rand() % (hi - lo + 1)) + lo);
}

inline float GetRandom(float min, float max)
{
	return min + (max - min) * ((float)rand() / (float)RAND_MAX);
}

// Compress/decompress a rotation into a single byte.  This only accounts for 
// rotation around the Y axis.
DBYTE CompressRotationByte(CommonLT *pCommon, DRotation *pRotation);
void UncompressRotationByte(CommonLT *pCommon, DBYTE rot, DRotation *pRotation);


#ifdef _CLIENTBUILD
#include "../ClientShellDLL/ClientUtilities.h"
#else
#include "../ObjectDLL/ServerUtilities.h"
#endif

inline DBOOL ObjListFilterFn(HOBJECT hTest, void *pUserData)
{
	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	HOBJECT *hList = (HOBJECT*)pUserData;
	while(hList && *hList)
	{
		if(hTest == *hList)
			return DFALSE;
		++hList;
	}
	return DTRUE;
}

#endif // __COMMON_UTILITIES_H__
