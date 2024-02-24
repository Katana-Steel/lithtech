// ----------------------------------------------------------------------- //
//
// MODULE  : LineSystemFX.h
//
// PURPOSE : LineSystem special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __LINE_SYSTEM_FX_H__
#define __LINE_SYSTEM_FX_H__

#include "BaseLineSystemFX.h"

struct LSCREATESTRUCT : public SFXCREATESTRUCT
{
	LSCREATESTRUCT::LSCREATESTRUCT();

	DBOOL		bContinuous;
	DVector		vStartColor;
	DVector		vEndColor;
	DVector		vDims;
	DVector		vMinVel;
	DVector		vMaxVel;
	DVector		vPos;
	DFLOAT		fStartAlpha;
	DFLOAT		fEndAlpha;
	DFLOAT		fBurstWait;
	DFLOAT		fBurstWaitMin;
	DFLOAT		fBurstWaitMax;
	DFLOAT		fLinesPerSecond;
	DFLOAT		fLineLifetime;
	DFLOAT		fLineLength;
	DFLOAT		fViewDist;
};

inline LSCREATESTRUCT::LSCREATESTRUCT()
{
	vStartColor.Init();
	vEndColor.Init();
	vDims.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vPos.Init();
	fStartAlpha		= 0.0f;
	fEndAlpha		= 0.0f;
	fBurstWait		= 0.0f;
	fLinesPerSecond	= 0.0f;
	fLineLifetime	= 0.0f;
	fLineLength		= 0.0f;
	fViewDist		= 0.0f;
	
	fBurstWaitMin	= 0.01f;
	fBurstWaitMax	= 1.0f;

	bContinuous		= DTRUE;
}

struct LSLineStruct
{
	LSLineStruct::LSLineStruct();

	HDELINE hDELine;
	float	fLifetime;
	DVector	vVel;
};

inline LSLineStruct::LSLineStruct()
{
	memset(this, 0, sizeof(LSLineStruct));
}

typedef void (*RemoveLineFn)(void *pUserData, LSLineStruct* pLine); 

class CLineSystemFX : public CBaseLineSystemFX
{
	public :

		CLineSystemFX();
		~CLineSystemFX();

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

		void SetRemoveLineFn(RemoveLineFn Fn, void* pUserData) { m_RemoveLineFn = Fn; m_pUserData = pUserData; }

	protected :

		LSCREATESTRUCT m_cs;				// Holds all initialization data

		DBOOL  m_bFirstUpdate;				// Is this the first update
		DFLOAT m_fNextUpdate;				// Time between updates
		DFLOAT m_fLastTime;					// When was the last time
		double m_fMaxViewDistSqr;			// Max distance lines are added (from camera)
		
		DVector m_vStartOffset;				// Top of line offset
		DVector m_vEndOffset;				// Bottom of line offset

		RemoveLineFn m_RemoveLineFn;		// Function to be called when a line is removed
		void*		 m_pUserData;			// Data passed to RemoveLineFn

		static int m_snTotalLines;			// Total number of lines in all CLineSystemFX

		LSLineStruct* m_pLines;				// Array of lines in system
		int m_nTotalNumLines;				// Num of lines in array

		DBOOL m_bContinuous;				// Do we continually add lines

		LTFLOAT	m_fLinesToAdd;				// How many lines to add this time...

		void UpdateSystem();
		void AddLines(int nToAdd);
		void AddLine(int nIndex, DBOOL bSetIntialPos=DFALSE);
		void RemoveLine(int nIndex);
		void SetupSystem();
};


#endif // __LINE_SYSTEM_FX_H__

