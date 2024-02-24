// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponSoundFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 7/28/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponSoundFX.h"
#include "GameClientShell.h"
#include "cpp_client_de.h"
#include "MsgIds.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponSoundFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponSoundFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	if (!CSpecialFX::Init(hServObj, hRead)) return DFALSE;

	m_nType			= hRead->ReadByte();
	m_nWeaponId		= hRead->ReadByte();
	m_nBarrelId		= hRead->ReadByte();
	m_nClientId		= hRead->ReadByte();
	m_vPos = hRead->ReadCompPos();

	return DTRUE;
}

DBOOL CWeaponSoundFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	WSOUNDCREATESTRUCT* pWS = (WSOUNDCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vPos, pWS->vPos);
	m_nClientId = pWS->nClientId;
	m_nType		= pWS->nType;
	m_nWeaponId	= pWS->nWeaponId;
	m_nBarrelId	= pWS->nBarrelId;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponSoundFX::CreateObject
//
//	PURPOSE:	Create object associated with the CWeaponSoundFX
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponSoundFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	DDWORD dwId;
	if (m_pClientDE->GetLocalClientID(&dwId) != DE_OK) return DFALSE;

	// Don't play sounds for this client...

	if (int(dwId) == m_nClientId) return DFALSE;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel(m_nBarrelId);

	if (!pWeapon || !pBarrel) return DFALSE;

	char* pBuf = DNULL;

	switch (m_nType)
	{
		case WMKS_FIRE:
			pBuf = pBarrel->szFireSound;
		break;

		case WMKS_ALT_FIRE:
			pBuf = pBarrel->szAltFireSound;
		break;

//		case WMKS_SILENCED_FIRE:
//			pBuf = pWeapon->szSilencedFireSound;
//		break;

		case WMKS_DRY_FIRE:
			pBuf = pBarrel->szDryFireSound;
		break;

		case WMKS_MISC0:
		case WMKS_MISC1:
		case WMKS_MISC2:
			pBuf = pBarrel->szMiscSounds[m_nType];
		break;

		case WMKS_SELECT:
			pBuf = pWeapon->szSelectSound;
		break;

		case WMKS_DESELECT:
			pBuf = pWeapon->szDeselectSound;
		break;

		case WMKS_INVALID:
		default : break;
	}

	if (pBuf && pBuf[0])
	{
		g_pClientSoundMgr->PlaySoundFromPos(m_vPos, pBuf, WEAPON_SOUND_RADIUS, SOUNDPRIORITY_PLAYER_MEDIUM);
	}

	return DFALSE;  // Delete me, I'm done :)
}

//-------------------------------------------------------------------------------------------------
// SFX_WeaponSoundFactory
//-------------------------------------------------------------------------------------------------

const SFX_WeaponSoundFactory SFX_WeaponSoundFactory::registerThis;

CSpecialFX* SFX_WeaponSoundFactory::MakeShape() const
{
	return new CWeaponSoundFX();
}

