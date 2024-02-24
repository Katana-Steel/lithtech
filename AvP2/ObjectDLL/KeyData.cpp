// ----------------------------------------------------------------------- //
//
// MODULE  : KeyData.cpp
//
// PURPOSE : KeyData implementation for Keyframer class
//
// CREATED : 12/31/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyData.h"
#include "iltserver.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::KeyData()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyData::KeyData()
{
    m_rRot.Init();
	VEC_INIT(m_vPos);

	m_nKeyType			= POSITION_KEY;
	m_fDistToLastKey	= 0.0f;
	m_fTimeStamp		= 0.0f;
	m_fRealTime			= 0.0f;
	m_fSoundRadius		= 0.0f;
	m_hstrName			= NULL;
	m_hstrSoundName		= NULL;
	m_hstrMessageTarget = NULL;
	m_hstrMessageName	= NULL;
	m_hstrBPrintMessage = NULL;
	m_vPitchYawRoll.Init();
	m_bPrevValid = FALSE;
	m_BezierPrevCtrl.Init();
	m_bNextValid = FALSE;
	m_BezierNextCtrl.Init();
	m_LightFrames = 1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::~KeyData()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyData::~KeyData()
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
	if (pServerDE)
	{
		if (m_hstrName) pServerDE->FreeString (m_hstrName);
		if (m_hstrSoundName) pServerDE->FreeString (m_hstrSoundName);
		if (m_hstrMessageTarget) pServerDE->FreeString (m_hstrMessageTarget);
		if (m_hstrMessageName) pServerDE->FreeString (m_hstrMessageName);
		if (m_hstrBPrintMessage) pServerDE->FreeString (m_hstrBPrintMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::Copy()
//
//	PURPOSE:	Copy the Key object's data
//
// ----------------------------------------------------------------------- //

LTBOOL KeyData::Copy(Key* pKey)
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
    if (!pKey || !pServerDE) return LTFALSE;

	HOBJECT hKeyObj = pServerDE->ObjectToHandle(pKey);
	pServerDE->GetObjectRotation(hKeyObj, &m_rRot);
	pServerDE->GetObjectPos(hKeyObj, &m_vPos);

	m_fTimeStamp	= pKey->m_fTimeStamp;
	m_fSoundRadius	= pKey->m_fSoundRadius;
	m_vPitchYawRoll	= pKey->m_vPitchYawRoll;

	// Copy the name...

	char* pName = pServerDE->GetObjectName(hKeyObj);
	if (pName && pName[0])
	{
		m_hstrName = pServerDE->CreateString(pName);
	}


	// Copy the sound name if applicable...

	if (pKey->m_hstrSoundName)
	{
		m_hstrSoundName = pServerDE->CopyString(pKey->m_hstrSoundName);
		m_nKeyType |= SOUND_KEY;
	}


	// We only care about messages if we have both a target and a message...

	if (pKey->m_hstrMessageTarget && pKey->m_hstrMessageName)
	{
		m_hstrMessageTarget = pServerDE->CopyString(pKey->m_hstrMessageTarget);
		m_hstrMessageName = pServerDE->CopyString(pKey->m_hstrMessageName);
		m_nKeyType |= MESSAGE_KEY;
	}


	// Copy the BPrint message...

	if (pKey->m_hstrBPrintMessage)
	{
		m_hstrBPrintMessage = pServerDE->CopyString(pKey->m_hstrBPrintMessage);
		m_nKeyType |= BPRINT_KEY;
	}

	// Setup our bezier control points.
	// NOTE: it converts from tangent vectors to control points here.
	if( pKey->m_BezierPrevTangent.MagSqr( ) > 0.0f )
	{
		m_BezierPrevCtrl = m_vPos + pKey->m_BezierPrevTangent;
		m_bPrevValid = TRUE;
	}
	if( pKey->m_BezierNextTangent.MagSqr( ) > 0.0f )
	{
		m_BezierNextCtrl = m_vPos + pKey->m_BezierNextTangent;
		m_bNextValid = TRUE;
	}

	m_LightFrames = pKey->m_LightFrames;

    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyData::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	g_pLTServer->WriteToMessageHString(hWrite, m_hstrName);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrSoundName);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrMessageTarget);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrMessageName);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrBPrintMessage);

	*hWrite << m_rRot;          
	*hWrite << m_vPos;          
	*hWrite << m_vPitchYawRoll; 
	*hWrite << m_nKeyType;      
	*hWrite << m_fDistToLastKey;
	*hWrite << m_fTimeStamp;    
	*hWrite << m_fRealTime;     
	*hWrite << m_fSoundRadius;  

	*hWrite << m_bPrevValid;
	*hWrite << m_BezierPrevCtrl;
	*hWrite << m_bNextValid;
	*hWrite << m_BezierNextCtrl;

	*hWrite << m_LightFrames;	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyData::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	m_hstrName			= g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrSoundName		= g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrMessageTarget	= g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrMessageName	= g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrBPrintMessage	= g_pLTServer->ReadFromMessageHString(hRead);

	*hRead >> m_rRot;          
	*hRead >> m_vPos;          
	*hRead >> m_vPitchYawRoll; 
	*hRead >> m_nKeyType;      
	*hRead >> m_fDistToLastKey;
	*hRead >> m_fTimeStamp;    
	*hRead >> m_fRealTime;     
	*hRead >> m_fSoundRadius;  

	*hRead >> m_bPrevValid;
	*hRead >> m_BezierPrevCtrl;
	*hRead >> m_bNextValid;
	*hRead >> m_BezierNextCtrl;

	*hRead >> m_LightFrames;	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::CacheFiles
//
//	PURPOSE:	CacheFiles resources used by this object
//
// ----------------------------------------------------------------------- //

void KeyData::CacheFiles()
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

    char* pFile = LTNULL;
	if (m_hstrSoundName)
	{
		pFile = pServerDE->GetStringData(m_hstrSoundName);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}
