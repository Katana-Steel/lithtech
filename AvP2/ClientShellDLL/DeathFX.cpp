// ----------------------------------------------------------------------- //
//
// MODULE  : DeathFX.cpp
//
// PURPOSE : Death special FX - Implementation
//
// CREATED : 6/14/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DeathFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "GibFX.h"
#include "SFXMsgIds.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathFX::Init
//
//	PURPOSE:	Init the death fx
//
// ----------------------------------------------------------------------- //

DBOOL CDeathFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	DEATHCREATESTRUCT d;

	d.hServerObj	= hServObj;
	d.nCharacterSet	= hRead->ReadDWord();
	d.nDeathType	= hRead->ReadByte();
	d.vPos = hRead->ReadVector();
	d.vDir = hRead->ReadVector();

	return Init(&d);
}

DBOOL CDeathFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	DEATHCREATESTRUCT* pD = (DEATHCREATESTRUCT*)psfxCreateStruct;

	m_nDeathType	= pD->nDeathType;
	m_nCharacterSet = pD->nCharacterSet;
	m_vPos			= pD->vPos;
	m_vDir			= pD->vDir;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

DBOOL CDeathFX::CreateObject(CClientDE* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return DFALSE;

	// Determine what container the sfx is in...

	HLOCALOBJ objList[1];
	DDWORD dwNum = m_pClientDE->GetPointContainers(&m_vPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
			D_WORD dwCode;
			if (m_pClientDE->GetContainerCode(objList[0], &dwCode))
			{
				m_eCode = (ContainerCode)dwCode;
			}
		}
	}


	CreateDeathFX();


	return DFALSE;  // Delete me, I'm done :)
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathFX::CreateDeathFX
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

void CDeathFX::CreateDeathFX()
{
	CreateHumanDeathFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathFX::CreateHumanDeathFX
//
//	PURPOSE:	Create human specific death fx
//
// ----------------------------------------------------------------------- //

void CDeathFX::CreateHumanDeathFX()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings || !pSettings->Gore()) return;

	GIBCREATESTRUCT gib;

	m_pClientDE->AlignRotation(&(gib.rRot), &m_vDir, DNULL);

	DFLOAT fDamage = VEC_MAG(m_vDir);

	VEC_COPY(gib.vPos, m_vPos);
	VEC_SET(gib.vMinVel, 50.0f, 100.0f, 50.0f);
	VEC_MULSCALAR(gib.vMinVel, gib.vMinVel, fDamage);
	VEC_SET(gib.vMaxVel, 100.0f, 200.0f, 100.0f);
	VEC_MULSCALAR(gib.vMaxVel, gib.vMaxVel, fDamage);
	gib.fLifeTime		= 20.0f;
	gib.fFadeTime		= 7.0f;
	gib.nGibFlags		= 0;
	gib.bRotate			= DTRUE;
	gib.nCode			= m_eCode;
	gib.nCharacterSet	= (uint8)m_nCharacterSet;
	gib.bSubGibs		= DTRUE;
	gib.bBloodSplats	= DTRUE;

	SetupGibTypes(gib);

	psfxMgr->CreateSFX(SFX_GIB_ID, &gib);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathFX::SetupGibTypes
//
//	PURPOSE:	Setup gib types
//
// ----------------------------------------------------------------------- //

void CDeathFX::SetupGibTypes(GIBCREATESTRUCT & gib)
{
	int nCase = GetRandom(0, 3);

	switch (nCase)
	{
		case 0:  // The whole body!!! (human only)
		{
			gib.nNumGibs =1;
			gib.eGibTypes[0] = GT_BODY;
		}
		break;

		case 1:	// Head, arm(s), and leg(s)...
		{
			gib.nNumGibs = GetRandom(3, 5);
			gib.eGibTypes[0] = GT_HEAD;
			gib.eGibTypes[1] = GetRandom(0,1) == 0 ? GT_LEFT_ARM : GT_RIGHT_ARM;
			gib.eGibTypes[2] = GetRandom(0,1) == 0 ? GT_LEFT_LEG : GT_RIGHT_LEG;

			// Add another arm?...

			if (gib.nNumGibs == 4)
			{
				gib.eGibTypes[3] = (gib.eGibTypes[1] == GT_LEFT_ARM) ? GT_RIGHT_ARM : GT_LEFT_ARM;
			}

			// Add another leg?...

			if (gib.nNumGibs == 5)
			{
				gib.eGibTypes[4] = (gib.eGibTypes[2] == GT_LEFT_LEG) ? GT_RIGHT_LEG : GT_LEFT_LEG;
			}
		}
		break;

		case 2:  // Upper body, randomly a leg or two...
		{
			gib.nNumGibs = GetRandom(1, 3);
			gib.eGibTypes[0] = GT_UPPER_BODY;

			if (gib.nNumGibs == 2)  // Add a leg
			{
				gib.eGibTypes[1] = GetRandom(0,1) == 0 ? GT_LEFT_LEG : GT_RIGHT_LEG;
			}
			else if (gib.nNumGibs == 3)  // Add em both
			{
				gib.eGibTypes[1] = GT_LEFT_LEG;
				gib.eGibTypes[2] = GT_RIGHT_LEG;
			}
		}
		break;
		
		case 3:	// Lower body, randomly a head, an arm or two...
		{
			gib.nNumGibs = GetRandom(1, 4);
			gib.eGibTypes[0] = GT_LOWER_BODY;

			if (gib.nNumGibs == 2)
			{
				if (GetRandom(0,1) == 0)  // Add a head or an arm...
				{
					gib.eGibTypes[1] = GetRandom(0,1) == 0 ? GT_LEFT_ARM : GT_RIGHT_ARM;
				}
				else
				{
					gib.eGibTypes[1] = GT_HEAD;
				}
			}
			else if (gib.nNumGibs == 3)  // Add a head and an arm, or two arms...
			{
				if (GetRandom(0,1) == 0)  // Add a head and an arm...
				{
					gib.eGibTypes[1] = GetRandom(0,1) == 0 ? GT_LEFT_ARM : GT_RIGHT_ARM;
					gib.eGibTypes[2] = GT_HEAD;
				}
				else  // Add the arms
				{
					gib.eGibTypes[1] = GT_LEFT_ARM;
					gib.eGibTypes[2] = GT_RIGHT_ARM;
				}
			}
			else if (gib.nNumGibs == 4)  // Add ema ll
			{
				gib.eGibTypes[1] = GT_LEFT_ARM;
				gib.eGibTypes[2] = GT_RIGHT_ARM;
				gib.eGibTypes[3] = GT_HEAD;
			}
		}
		break;
		
		case 4:  // Smoking boots?
		break;
		
		default :
		break;
	}
}

//-------------------------------------------------------------------------------------------------
// SFX_DeathFactory
//-------------------------------------------------------------------------------------------------

const SFX_DeathFactory SFX_DeathFactory::registerThis;

CSpecialFX* SFX_DeathFactory::MakeShape() const
{
	return new CDeathFX();
}

