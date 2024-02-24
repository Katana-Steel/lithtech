// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalMgr.cpp
//
// PURPOSE : Implementations of global definitions
//
// CREATED : 7/07/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GlobalMgr.h"

#include "WeaponMgr.h"
#include "ModelButeMgr.h"
#include "SurfaceMgr.h"
#include "MissionMgr.h"
#include "CharacterButeMgr.h"
#include "DebrisMgr.h"
#include "PickupButeMgr.h"
#include "SoundButeMgr.h"
#include "ObjectivesButeMgr.h"
#include "FXButeMgr.h"
#include "AIAnimButeMgr.h"

#ifdef _CLIENTBUILD

	#include "ClientButeMgr.h"
	#include "ClientSoundMgr.h"

	#ifndef VISION_MODE_BUTE_MGR_H
		#include "VisionModeButeMGR.h"
	#endif

#else

	#include "AnimationButeMgr.h"
	#include "AIButeMgr.h"
	#include "AttachButeMgr.h"
	#include "ServerSoundMgr.h"
	#include "PropTypeMgr.h"
	#include "SpawnButeMgr.h"

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //
static void FillErrorMsg(char * pErrorBuf, int nBufSize, const char * szTitle, const char * szFileName, CString strError)
{
	if( strError.IsEmpty() )
	{
		strError = "Unknown.";
	}

	_snprintf(pErrorBuf, nBufSize, "ERROR in CGlobalMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!\n Error : %s", 
		szTitle, szFileName, strError );
}


DBOOL CGlobalMgr::Init(ILTCSBase *pInterface)
{
	// Be sure everything is clear before initializing.
	Term();

	m_pFXButeMgr			= new CFXButeMgr();
	m_pWeaponMgr			= new CWeaponMgr();
	m_pModelButeMgr			= new CModelButeMgr();
	m_pSurfaceMgr			= new CSurfaceMgr();
	m_pMissionMgr			= new CMissionMgr();
	m_pCharacterButeMgr		= new CCharacterButeMgr();
	m_pDebrisMgr			= new CDebrisMgr();
	m_pPickupButeMgr		= new CPickupButeMgr();
	m_pSoundButeMgr			= new CSoundButeMgr();
	m_pObjectiveButeMgr		= new CObjectiveButeMgr();

	const int nErrorBufSize = 256;
	char szErrorBuf[nErrorBufSize];
	szErrorBuf[nErrorBufSize - 1] = '\0';

#ifdef _CLIENTBUILD

	m_pClientButeMgr	= new CClientButeMgr();
	m_pClientSoundMgr	= new CClientSoundMgr();
	m_pVisionButeMgr	= new VisionMode::ButeMGR();

	if (!m_pObjectiveButeMgr || !m_pObjectiveButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "ObjectiveButeMgr", OBJECTIVE_BUTES_DEFAULT_FILE, m_pObjectiveButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pSoundButeMgr || !m_pSoundButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "SoundButeMgr", SOUND_BUTES_DEFAULT_FILE, m_pSoundButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pDebrisMgr || !m_pDebrisMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "DebrisMgr", DEBRISMGR_DEFAULT_FILE, m_pDebrisMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pFXButeMgr || !m_pFXButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "FXButeMgr", FXBMGR_DEFAULT_FILE, m_pFXButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}	

	if (!m_pWeaponMgr || !m_pWeaponMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "WeaponMgr", WEAPON_DEFAULT_FILE, m_pWeaponMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}	

	if (!m_pModelButeMgr || !m_pModelButeMgr->Init(pInterface)) // ModeButeMgr must be initialized before CharacterButeMgr.
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "ModelButeMgr", MBMGR_DEFAULT_FILE, m_pModelButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pSurfaceMgr || !m_pSurfaceMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "SurfaceMgr", SRFMGR_DEFAULT_FILE, m_pSurfaceMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pMissionMgr || !m_pMissionMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "MissionMgr", MISSION_DEFAULT_FILE, m_pMissionMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pCharacterButeMgr || !m_pCharacterButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "CharacterButeMgr", CHARACTER_BUTES_DEFAULT_FILE, m_pCharacterButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pPickupButeMgr || !m_pPickupButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "PickupButeMgr", PICKUP_BUTES_DEFAULT_FILE, m_pPickupButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}


	// Client only managers
	if (!m_pClientButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "ClientButeMgr", CBMGR_DEFAULT_FILE, m_pClientButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pClientSoundMgr || !m_pClientSoundMgr->Init(pInterface))		
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "ClientSoundMgr", CSNDMGR_DEFAULT_FILE, m_pClientSoundMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}


	// Vision modes
	if( !m_pVisionButeMgr || !m_pVisionButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "VisionButeMgr", CBMGR_DEFAULT_FILE, m_pVisionButeMgr->GetErrorString());
		g_pInterface->ShutdownWithMessage(szErrorBuf);
		return LTFALSE;
	}


#else

	m_pAIButeMgr		= new CAIButeMgr();
	m_pAttachButeMgr	= new CAttachButeMgr();
	m_pServerSoundMgr	= new CServerSoundMgr();
	m_pPropTypeMgr		= new CPropTypeMgr();
	m_pSpawnButeMgr		= new CSpawnButeMgr();
	m_pAnimationButeMgr	= new CAnimationButeMgr();
	m_pAIAnimButeMgr	= new CAIAnimButeMgr();

	// Currently not checking these on the server...
	if( m_pSoundButeMgr )		m_pSoundButeMgr->Init(pInterface);
	if( m_pDebrisMgr )			m_pDebrisMgr->Init(pInterface);
	if( m_pFXButeMgr )			m_pFXButeMgr->Init(pInterface);
	if( m_pWeaponMgr )			m_pWeaponMgr->Init(pInterface);
	if( m_pModelButeMgr )		m_pModelButeMgr->Init(pInterface);
	if( m_pSurfaceMgr )			m_pSurfaceMgr->Init(pInterface);
	if( m_pMissionMgr )			m_pMissionMgr->Init(pInterface);
	if( m_pCharacterButeMgr )	m_pCharacterButeMgr->Init(pInterface);
	if( m_pPickupButeMgr )		m_pPickupButeMgr->Init(pInterface);
	if( m_pAnimationButeMgr )	m_pAnimationButeMgr->Init(pInterface);
	if( m_pObjectiveButeMgr )	m_pObjectiveButeMgr->Init(pInterface);

	// Server only managers
	if( !m_pAIButeMgr || !m_pAIButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "AIButeMgr", g_szAIButeMgrFile, m_pAIButeMgr->GetErrorString());
		g_pInterface->CPrint(szErrorBuf);
		return LTFALSE;
	}

	if( !m_pAttachButeMgr || !m_pAttachButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "AttachButeMgr", g_szAttachButeMgrFile, m_pAttachButeMgr->GetErrorString());
		g_pInterface->CPrint(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pServerSoundMgr || !m_pServerSoundMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "ServerSoundMgr", SSNDMGR_DEFAULT_FILE, m_pServerSoundMgr->GetErrorString());
		g_pInterface->CPrint(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pPropTypeMgr || !m_pPropTypeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "PropTypeMgr", PTMGR_DEFAULT_FILE, m_pPropTypeMgr->GetErrorString());
		g_pInterface->CPrint(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pSpawnButeMgr || !m_pSpawnButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "SpawnButeMgr", SPAWN_BUTES_DEFAULT_FILE, m_pSpawnButeMgr->GetErrorString());
		g_pInterface->CPrint(szErrorBuf);
		return LTFALSE;
	}

	if (!m_pAIAnimButeMgr || !m_pAIAnimButeMgr->Init(pInterface))
	{
		FillErrorMsg(szErrorBuf, nErrorBufSize - 1, "AIAnimButeMgr", g_szAIAnimButeMgrFile, m_pAIAnimButeMgr->GetErrorString());
		g_pInterface->CPrint(szErrorBuf);
		return LTFALSE;
	}

#endif

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Term()
//
//	PURPOSE:	Terminate
//
// ----------------------------------------------------------------------- //

void CGlobalMgr::Term()
{
	if( m_pFXButeMgr )
	{
		m_pFXButeMgr->Term();
		delete m_pFXButeMgr;
		m_pFXButeMgr = LTNULL;
	}

	if( m_pWeaponMgr )
	{
		m_pWeaponMgr->Term();
		delete m_pWeaponMgr;
		m_pWeaponMgr = LTNULL;
	}

	if( m_pModelButeMgr )
	{
		m_pModelButeMgr->Term();
		delete m_pModelButeMgr;
		m_pModelButeMgr = LTNULL;
	}

	if( m_pSurfaceMgr )
	{
		m_pSurfaceMgr->Term();
		delete m_pSurfaceMgr;
		m_pSurfaceMgr = LTNULL;
	}

	if( m_pMissionMgr )
	{
		m_pMissionMgr->Term();
		delete m_pMissionMgr;
		m_pMissionMgr = LTNULL;
	}

	if( m_pCharacterButeMgr )
	{
		m_pCharacterButeMgr->Term();
		delete m_pCharacterButeMgr;
		m_pCharacterButeMgr = LTNULL;
	}

	if( m_pSoundButeMgr )
	{
		m_pSoundButeMgr->Term();
		delete m_pSoundButeMgr;
		m_pSoundButeMgr = LTNULL;
	}

	if( m_pObjectiveButeMgr )
	{
		m_pObjectiveButeMgr->Term();
		delete m_pObjectiveButeMgr;
		m_pObjectiveButeMgr = LTNULL;
	}

#ifdef _CLIENTBUILD

	if( m_pClientButeMgr )
	{
		m_pClientButeMgr->Term();
		delete m_pClientButeMgr;
		m_pClientButeMgr = LTNULL;
	}

	if( m_pClientSoundMgr )
	{
		m_pClientSoundMgr->Term();
		delete m_pClientSoundMgr;
		m_pClientSoundMgr = LTNULL;
	}

	if( m_pVisionButeMgr )
	{
		delete m_pVisionButeMgr;
		m_pVisionButeMgr = LTNULL;
	}

#else

	if( m_pAIButeMgr )
	{
		m_pAIButeMgr->Term();
		delete m_pAIButeMgr;
		m_pAIButeMgr = LTNULL;
	}

	if( m_pAttachButeMgr )
	{
		m_pAttachButeMgr->Term();
		delete m_pAttachButeMgr;
		m_pAttachButeMgr = LTNULL;
	}

	if( m_pServerSoundMgr )
	{
		m_pServerSoundMgr->Term();
		delete m_pServerSoundMgr;
		m_pServerSoundMgr = LTNULL;
	}

	if( m_pSpawnButeMgr )
	{
		m_pSpawnButeMgr->Term();
		delete m_pSpawnButeMgr;
		m_pSpawnButeMgr = LTNULL;
	}

#endif

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::~CGlobalMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGlobalMgr::~CGlobalMgr()
{
	Term();
}

