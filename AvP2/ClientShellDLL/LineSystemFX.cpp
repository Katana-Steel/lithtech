// ----------------------------------------------------------------------- //
//
// MODULE  : LineSystemFX.cpp
//
// PURPOSE : LineSystem special FX - Implementation
//
// CREATED : 4/12/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LineSystemFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "VarTrack.h"

#define MAX_LINES_PER_SECOND	1000			// Max lines to be created in a second
#define MAX_TOTAL_LINES			4000			// Max lines total (all systems)
#define MAX_SYSTEM_LINES		2000			// Max lines per system
#define MAX_VIEW_DIST_SQR		(10000*10000)	// Max global distance to add lines

extern CGameClientShell* g_pGameClientShell;
extern DVector g_vWorldWindVel;

int CLineSystemFX::m_snTotalLines = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::CLineSystemFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CLineSystemFX::CLineSystemFX() : CBaseLineSystemFX()
{
	m_bFirstUpdate	= DTRUE;
	m_bContinuous	= DFALSE;

	m_fLastTime		= 0.0f;
	m_fNextUpdate	= 0.01f;

	m_RemoveLineFn	= DNULL;
	m_pUserData		= DNULL;

	m_fMaxViewDistSqr = 1000.0f*1000.0f;
	m_nTotalNumLines  = 0;
	m_pLines		  = DNULL;

	m_vStartOffset.Init();
	m_vEndOffset.Init();

	m_fLinesToAdd = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::~CLineSystemFX
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CLineSystemFX::~CLineSystemFX()
{
	if (m_pLines)
	{
		for (int i=0; i < m_nTotalNumLines; i++)
		{
			RemoveLine(i);
		}

		delete [] m_pLines;
		m_pLines = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::Init
//
//	PURPOSE:	Init the line system
//
// ----------------------------------------------------------------------- //

DBOOL CLineSystemFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	LSCREATESTRUCT ls;

	ls.hServerObj = hServObj;
	ls.vDims = hRead->ReadVector();
	ls.vMinVel = hRead->ReadVector();
	ls.vMaxVel = hRead->ReadVector();
	ls.vStartColor = hRead->ReadVector();
	ls.vEndColor = hRead->ReadVector();
	ls.fStartAlpha		= hRead->ReadFloat();
	ls.fEndAlpha		= hRead->ReadFloat();
	ls.fBurstWait		= hRead->ReadFloat();
	ls.fBurstWaitMin	= hRead->ReadFloat();
	ls.fBurstWaitMax	= hRead->ReadFloat();
	ls.fLinesPerSecond	= hRead->ReadFloat();
	ls.fLineLifetime	= hRead->ReadFloat();
	ls.fLineLength 	    = hRead->ReadFloat();
	ls.fViewDist 	    = hRead->ReadFloat();

	return Init(&ls);
}

DBOOL CLineSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return DFALSE;

	// Set up our creation struct...

	LSCREATESTRUCT* pLS = (LSCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pLS;


	// Set our (parent's) flags...

	SetPos(m_cs.vPos);


	// Set our max viewable distance...

	m_fMaxViewDistSqr = m_cs.fViewDist*m_cs.fViewDist;
	m_fMaxViewDistSqr = m_fMaxViewDistSqr > MAX_VIEW_DIST_SQR ? MAX_VIEW_DIST_SQR : m_fMaxViewDistSqr;


	// Adjust velocities based on global wind values...

	m_cs.vMinVel += g_vWorldWindVel;
	m_cs.vMaxVel += g_vWorldWindVel;


	// Adjust slope of lines based on global wind values...

	m_vStartOffset.y = m_cs.fLineLength / 2.0f;
	m_vEndOffset.y	 = -m_cs.fLineLength / 2.0f;

	float fVal = (float)fabs(m_cs.vMaxVel.y);

	if (fVal > 0.0)
	{
		m_vEndOffset.x = (m_cs.fLineLength * (g_vWorldWindVel.x / fVal));
	}

	if (fVal > 0.0)
	{
		m_vEndOffset.z = (m_cs.fLineLength * (g_vWorldWindVel.z / fVal));
	}

	m_bContinuous = (m_cs.fBurstWait <= 0.001f);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CLineSystemFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	DBOOL bRet = CBaseLineSystemFX::CreateObject(pClientDE);

	if (bRet && m_hObject && m_hServerObject)
	{
		DDWORD dwUserFlags;
		pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);
		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			DDWORD dwFlags = pClientDE->GetObjectFlags(m_hObject);
			pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
		}
	}


	// Create lines...

	m_nTotalNumLines = int(m_cs.fLinesPerSecond * m_cs.fLineLifetime);
	if (m_nTotalNumLines > MAX_SYSTEM_LINES)
	{
		m_nTotalNumLines = MAX_SYSTEM_LINES;
	}

	m_pLines = new LSLineStruct[m_nTotalNumLines];

	SetupSystem();

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CLineSystemFX::Update()
{
	if (!m_hObject || !m_pClientDE || IsWaitingForRemove() ) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Hide/show the line system if necessary...

	if (m_hServerObject)
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			m_pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
			m_fLastTime = fTime;
			return DTRUE;
		}
		else
		{
			m_pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		}
	}

	if (m_bFirstUpdate)
	{
		m_fLastTime = fTime;
		m_bFirstUpdate = DFALSE;
	}
	else
	{
		UpdateSystem();
	}

	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
		return DTRUE;
	}


	// Ok, how many to add this frame....

	float fTimeDelta = fTime - m_fLastTime;

	// Make sure delta time is no less than 15 frames/sec if we're 
	// continuously adding lines...

	if (m_bContinuous)
	{
		fTimeDelta = fTimeDelta > 0.0666f ? 0.0666f : fTimeDelta;
	}

	m_fLinesToAdd += m_cs.fLinesPerSecond * fTimeDelta;

	// Cap it
	if(m_fLinesToAdd > MAX_LINES_PER_SECOND)
		m_fLinesToAdd = MAX_LINES_PER_SECOND;
	
	// Add new lines...
	if(m_fLinesToAdd > 1.0f)
	{
		AddLines((int)m_fLinesToAdd);
		m_fLinesToAdd -= (int)m_fLinesToAdd;
	}


	// Determine when next update should occur...

	if (m_cs.fBurstWait > 0.001f) 
	{
		m_fNextUpdate = m_cs.fBurstWait * GetRandom(m_cs.fBurstWaitMin, m_cs.fBurstWaitMax);
	}
	else 
	{
		m_fNextUpdate = 0.001f;
	}

	m_fLastTime = fTime;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::UpdateSystem
//
//	PURPOSE:	Update the lines in the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::UpdateSystem()
{
	if (!m_pLines) return;

	// Make sure delta time is no less than 15 frames/sec...

	DFLOAT fDeltaTime = m_pClientDE->GetFrameTime();
	fDeltaTime = fDeltaTime > 0.0666f ? 0.0666f : fDeltaTime;

	// Update all the lines...

	DELine line;

	for (int i=0; i < m_nTotalNumLines; i++)
	{
		if (m_pLines[i].hDELine)
		{
			m_pClientDE->GetLineInfo(m_pLines[i].hDELine, &line);

			line.m_Points[0].m_Pos += (m_pLines[i].vVel * fDeltaTime);
			line.m_Points[1].m_Pos += (m_pLines[i].vVel * fDeltaTime);

			m_pClientDE->SetLineInfo(m_pLines[i].hDELine, &line);
			m_pLines[i].fLifetime -= fDeltaTime;
			
			// Remove dead lines...

			if (m_pLines[i].fLifetime <= 0.0f)
			{
				if (m_RemoveLineFn)
				{
					m_RemoveLineFn(m_pUserData, &(m_pLines[i]));
				}

				RemoveLine(i);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::SetupSystem
//
//	PURPOSE:	Setup the system (add ALL possible lines)
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::SetupSystem()
{
	if (!m_hServerObject) return;

	if (m_bContinuous)
	{
		for (int i=0; i < m_nTotalNumLines; i++)
		{
			if (m_snTotalLines < MAX_TOTAL_LINES)
			{
				AddLine(i, DTRUE);
			}
			else
			{
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::AddLines
//
//	PURPOSE:	Add lines to the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::AddLines(int nToAdd)
{
	if (!m_hServerObject || !m_pLines) return;

	int nNumAdded = 0;

	for (int i=0; nNumAdded < nToAdd && i < m_nTotalNumLines; i++)
	{
		if (!m_pLines[i].hDELine || m_pLines[i].fLifetime <= 0.0f)
		{
			if (m_snTotalLines < MAX_TOTAL_LINES)
			{
				AddLine(i);
				nNumAdded++;
			}
			else
			{
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::AddLine
//
//	PURPOSE:	Add a line to the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::AddLine(int nIndex, DBOOL bSetInitialPos)
{
	if (!m_hServerObject || !m_pLines || nIndex < 0 || nIndex >= m_nTotalNumLines) return;

	DVector vPos, vCamPos, vObjPos, vDist, vVel;
	DELine line;

	m_pClientDE->GetObjectPos(m_hServerObject, &vObjPos);

	// Get the camera's position...If the camera is too far away from
	// the line being added, don't add it :)

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	if (!hCamera) return;

	m_pClientDE->GetObjectPos(hCamera, &vCamPos);
	vCamPos.y = 0.0f;  // Only take x/z into account...

	vPos.x = GetRandom(-m_cs.vDims.x, m_cs.vDims.x);
	vPos.y = GetRandom(-m_cs.vDims.y, m_cs.vDims.y);
	vPos.z = GetRandom(-m_cs.vDims.z, m_cs.vDims.z);

	DFLOAT fLifetime = m_cs.fLineLifetime;

	vVel.x = GetRandom(m_cs.vMinVel.x, m_cs.vMaxVel.x);
	vVel.y = GetRandom(m_cs.vMinVel.y, m_cs.vMaxVel.y);
	vVel.z = GetRandom(m_cs.vMinVel.z, m_cs.vMaxVel.z);

	
	// If we need to set the initial line pos...

	if (bSetInitialPos)
	{
		fLifetime = GetRandom(0.0f, m_cs.fLineLifetime);

		if (fLifetime >= 0.0f)
		{
			vPos += (vVel * (m_cs.fLineLifetime - fLifetime));
		}
		else
		{
			fLifetime = m_cs.fLineLifetime;
		}
	}


	vObjPos += vPos;
	vObjPos.y = 0.0f; // Only take x/z into account

	vDist = vCamPos - vObjPos;

	if (vDist.MagSqr() < m_fMaxViewDistSqr || bSetInitialPos)
	{
		line.m_Points[0].m_Pos = vPos + m_vStartOffset;
		line.m_Points[0].r	   = m_cs.vStartColor.x;
		line.m_Points[0].g	   = m_cs.vStartColor.y;
		line.m_Points[0].b	   = m_cs.vStartColor.z;
		line.m_Points[0].a	   = m_cs.fStartAlpha;

		line.m_Points[1].m_Pos = vPos + m_vEndOffset;
		line.m_Points[1].r	   = m_cs.vEndColor.x;
		line.m_Points[1].g	   = m_cs.vEndColor.y;
		line.m_Points[1].b	   = m_cs.vEndColor.z;
		line.m_Points[1].a	   = m_cs.fEndAlpha;

		if (m_pLines[nIndex].hDELine)
		{
			m_pClientDE->SetLineInfo(m_pLines[nIndex].hDELine, &line);
		}
		else
		{
			m_pLines[nIndex].hDELine = m_pClientDE->AddLine(m_hObject, &line);

			m_snTotalLines++;
		}

		m_pLines[nIndex].fLifetime	= fLifetime;
		m_pLines[nIndex].vVel		= vVel;
	}
	else
	{
		RemoveLine(nIndex);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::RemoveLine
//
//	PURPOSE:	Remove the line from the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::RemoveLine(int nIndex)
{
	if (!m_pLines || nIndex < 0 || nIndex >= m_nTotalNumLines) return;

	if (m_pLines[nIndex].hDELine)
	{
		m_pClientDE->RemoveLine(m_hObject, m_pLines[nIndex].hDELine);
		m_pLines[nIndex].hDELine = DNULL;
		m_snTotalLines--;
	}
}

