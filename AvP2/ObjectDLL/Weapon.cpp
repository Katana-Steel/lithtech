// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.cpp
//
// PURPOSE : Weapon class - implementation
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Weapon.h"
#include "ServerUtilities.h"
#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "MsgIDs.h"
#include "HHWeaponModel.h"
#include "PlayerObj.h"
#include "WeaponFXTypes.h"
#include "GameServerShell.h"
#include "Weapons.h"
#include "WeaponMgr.h"
#include "FXButeMgr.h"

extern DBOOL g_bInfiniteAmmo;

const uint32 INFINITE_AMMO_AMOUNT = 1000;

D_WORD g_wIgnoreFX = 0;
DBYTE g_nRandomWeaponSeed = 255;

#ifdef _DEBUG
#include "DebugLineSystem.h"
#include "AIUtils.h"
//#define WEAPON_DEBUG
//#define AMMO_DEBUG
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CWeapon
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CWeapon::CWeapon(CWeapons* pParent, HOBJECT hOwner, int nWeaponId, int nBarrelId, int nAmmoId)
	: m_hOwner(hOwner),
	
	  m_nAmmoInClip(1),  // This must be a non-zero value for dagger, claw, and maybe other weapons to fire.
	  m_nCurrentFlashSocket(0),

	  m_nWeaponId(nWeaponId),
	  m_nBarrelId(nBarrelId),
	  m_nAmmoId(nAmmoId),

	  m_pParent(pParent),

	  m_pWeaponData(g_pWeaponMgr->GetWeapon(nWeaponId)),
	  m_pBarrelData(g_pWeaponMgr->GetBarrel(nBarrelId)),
	  m_pAmmoData(g_pWeaponMgr->GetAmmo(nAmmoId)),
	  m_bImpactSoundPlayed(LTFALSE)
{
	_ASSERT( m_pParent );
	_ASSERT( m_pWeaponData );
	_ASSERT( m_pBarrelData );
	_ASSERT( m_pAmmoData );
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ReloadClip
//
//	PURPOSE:	Fill the clip
//
// ----------------------------------------------------------------------- //

void CWeapon::ReloadClip(int nNewAmmo)
{
	if (!m_pParent || !m_pBarrelData) return;

	const int nAmmo = nNewAmmo >= 0 ? nNewAmmo : m_pParent->GetAmmoCount(m_nAmmoId);
	const int nShotsPerClip = m_pBarrelData->nShotsPerClip;


	if (m_nAmmoInClip == nShotsPerClip)
	{
		// Clip is full...
		return;
	}

	if (nAmmo > 0 && nShotsPerClip > 0)
	{
		m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Fire
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::Fire(const WFireInfo & info)
{
	if( !m_pParent || !m_pWeaponData || !m_pBarrelData || !m_pAmmoData ) 
		return LTFALSE;

	if (!info.hFiredFrom || !g_pWeaponMgr )
		return LTFALSE;

	g_nRandomWeaponSeed = info.nSeed;

	//see if this is the shoulder cannon
	if(stricmp(m_pWeaponData->szName, "Shoulder_Cannon")==0 || stricmp(m_pWeaponData->szName, "Shoulder_Cannon_MP")==0 )
	{
		//see if we are pre-charged
		if(IsCharacter(m_hOwner))
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hOwner);

			if(pCharacter)
			{
				if(pCharacter->GetCannonChargeLevel())
				{
					//switch up the barrels and relieve charge level
					m_pBarrelData = g_pWeaponMgr->GetBarrel(m_pWeaponData->aBarrelTypes[PRIMARY_BARREL]);

					for(int i=0; i<pCharacter->GetCannonChargeLevel() && m_pBarrelData->szNextBarrel[0]; i++)
					{
						m_pBarrelData = g_pWeaponMgr->GetBarrel(m_pBarrelData->szNextBarrel);
					}

					//reset ammo data
					m_pAmmoData = g_pWeaponMgr->GetAmmo(m_pBarrelData->nAmmoType);

					m_nAmmoId = m_pAmmoData->nId;

					pCharacter->SetCannonChargeLevel(0);

					if(IsPlayer(m_hOwner))
					{
						CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hOwner);
						pPlayer->UpdateInterface(LTTRUE);
					}
				}
				else
				{
					// force the initial barrel here...
					m_pBarrelData = g_pWeaponMgr->GetBarrel(m_pWeaponData->aBarrelTypes[PRIMARY_BARREL]);

					//reset ammo data
					m_pAmmoData = g_pWeaponMgr->GetAmmo(m_pBarrelData->nAmmoType);

					m_nAmmoId = m_pAmmoData->nId;
				}
			}
		}
	}


	// Make sure we always have ammo if we should...

	DBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pBarrelData->bInfiniteAmmo );
	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : m_pParent->GetAmmoCount(m_nAmmoId);

#ifdef AMMO_DEBUG
	AICPrint(m_hOwner, "Ammo is %d", nAmmo);
#endif

	AmmoType eAmmoType = m_pAmmoData->eType;

	if (nAmmo > 0)
	{
		// Ignore the exit surface, silenced, and alt-fire fx...

		g_wIgnoreFX = WFX_EXITSURFACE | WFX_ALTFIRESND;

		if (info.bAltFire)
		{
			g_wIgnoreFX &= ~WFX_ALTFIRESND;  // Do alt-fire
			g_wIgnoreFX |= WFX_FIRESOUND;	 // Don't do fire
		}

		// If the player fired this and it is the appropriate weapon type,
		// don't worry about playing the fire sound (the player already played it)...

		if (IsPlayer(info.hFiredFrom))
		{
			if (eAmmoType == PROJECTILE) 
			{
				g_wIgnoreFX |= WFX_FIRESOUND | WFX_ALTFIRESND;
			}

			// Since all clip info is handled on the client just set this to 1
			m_nAmmoInClip = 1;
		}

		int nShotsPerClip = m_pBarrelData->nShotsPerClip;

		if( m_nAmmoInClip > 0 )
		{
			// Create a projectile for every vector...

			int nVectorsPerShot = m_pBarrelData->nVectorsPerRound;

			//DBYTE nNewRandomSeed = GetRandom(2, 255);

			WeaponPath wp;
			WFireInfo shot_info = info;
			DRotation rRot;

			DVector vU, vR, vF;

			g_pServerDE->GetObjectRotation(shot_info.hFiredFrom, &rRot);
			g_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

			wp.nBarrelId = (uint8)m_nBarrelId;
			wp.fPerturbR = shot_info.fPerturbR;
			wp.fPerturbU = shot_info.fPerturbU;

			//use the fire info to re-set the up and right vecs
			//and align the rotation
			wp.vU = vR.Cross(shot_info.vPath);
			wp.vR = vR;
			g_pLTServer->AlignRotation(&rRot, const_cast<LTVector*>(&shot_info.vPath), &wp.vU);

#ifdef WEAPON_DEBUG
			LineSystem::GetSystem(this,"WeaponPath") << LineSystem::Clear();
#endif
			// Reset the sound bool
			m_bImpactSoundPlayed = LTFALSE;

			for (int i=0; i < nVectorsPerShot; i++)
			{
				srand(g_nRandomWeaponSeed);
				g_nRandomWeaponSeed = GetRandom(2, 255);

				wp.vPath = info.vPath;
				g_pWeaponMgr->CalculateWeaponPath(wp);
				shot_info.vPath = wp.vPath;

#ifdef WEAPON_DEBUG

				LTVector vFireTo = shot_info.vFirePos + shot_info.vPath*100.0f;
				if( shot_info.hTestObj )
				{
					g_pLTServer->GetObjectPos(shot_info.hTestObj, &vFireTo);
				}

				LineSystem::GetSystem(this,"WeaponPath")
					<< LineSystem::Line(shot_info.vFirePos, vFireTo);
#endif
				// Shoot the weapon...

				if (eAmmoType == PROJECTILE)
				{
					if (!CreateProjectile(rRot, shot_info))
					{
						return LTFALSE;
					}
				}
				else if (eAmmoType == VECTOR || eAmmoType == CANNON)
				{
					// Don't actually create an object, just use our
					// parent's projectile object to do vector calculations...

					CProjectile* pProj = m_pParent->GetVecProjectile();
					if (pProj)
					{
						// Should we do an impact sound?
						if(m_pBarrelData->bSquelchMultipleImpacts && m_bImpactSoundPlayed)
							shot_info.bIgnoreImpactSound = LTTRUE;
						pProj->Setup(this, shot_info);
					}

					if(m_pAmmoData->eInstDamageType == DT_SLICE || m_pAmmoData->eInstDamageType == DT_ALIEN_CLAW || (_stricmp(m_pAmmoData->szAmmoPool, "Blades")==0))
					{
						if(IsCharacter(m_hOwner))
						{
							CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hOwner);

							if(pCharacter)
							{
								if(pCharacter->IsNetted())
									pCharacter->HandleNetSlash(m_pAmmoData->nInstDamage);
							}
						}
					}
				}
				else if(eAmmoType == ALIEN_POUNCE)
				{
					//send message to shooter to go to pounce mode
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(LTNULL, shot_info.hFiredFrom, MID_FORCE_POUNCE);
					hMessage->WriteObject(info.hTestObj);
					g_pServerDE->EndMessage(hMessage);
				}
				else if(eAmmoType == FACE_HUG)
				{
					//send message to shooter to go to pounce mode
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(LTNULL, shot_info.hFiredFrom, MID_FORCE_FACEHUG);
					hMessage->WriteObject(info.hTestObj);
					g_pServerDE->EndMessage(hMessage);
				}


				// If we are shooting multiple vectors ignore some special
				// fx after the first vector...

				g_wIgnoreFX |= WFX_FIRESOUND | WFX_ALTFIRESND | WFX_SHELL |	WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;
			}

			//srand(nNewRandomSeed);

			if (nShotsPerClip > 0) 
			{
				m_nAmmoInClip--;
			}

			if (!bInfiniteAmmo)
			{
				m_pParent->DecrementAmmo(m_nAmmoId, m_pAmmoData->nAmmoPerShot);
			}
		}

		// The player always reloads.

		if( IsPlayer(info.hFiredFrom) )
		{
			if (nShotsPerClip > 0 && m_nAmmoInClip <= 0) 
			{
				ReloadClip();
			}
		}
	} 
	else  // NO AMMO
	{
		return LTFALSE;
	}

	// Change our flash socket.
	if( m_pBarrelData->nNumFlashSockets > 1 )
	{
		++m_nCurrentFlashSocket;
		if( m_nCurrentFlashSocket >= m_pBarrelData->nNumFlashSockets )
		{
			m_nCurrentFlashSocket = 0;
		}
	}

	// See if we need to turn off cloaking
	if(m_pBarrelData->bDecloak && IsCharacter(m_hOwner))
	{
		CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hOwner);

		if(pCharacter)
		{
			pCharacter->ForceCloakOff(LTTRUE);
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::HitFire
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::HitFire(const WFireInfo & info, LTBOOL& bHidePVFX)
{
	if( !m_pParent || !m_pWeaponData || !m_pBarrelData || !m_pAmmoData ) 
		return;
	
	if (!info.hFiredFrom || !g_pWeaponMgr )
		return;
	
	// Make sure we always have ammo if we should...
	
	DBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pBarrelData->bInfiniteAmmo );
	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : m_pParent->GetAmmoCount(m_nAmmoId);
	
	AmmoType eAmmoType = m_pAmmoData->eType;
	
	if(nAmmo <= 0) return;
	
	
	// Ignore the exit surface, silenced, and alt-fire fx...
	
	g_wIgnoreFX = WFX_EXITSURFACE | WFX_ALTFIRESND;

	if(bHidePVFX)
		g_wIgnoreFX |=	WFX_FIRESOUND | WFX_ALTFIRESND | WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;

	if (info.bAltFire)
	{
		g_wIgnoreFX &= ~WFX_ALTFIRESND;  // Do alt-fire
		g_wIgnoreFX |= WFX_FIRESOUND;	 // Don't do fire
	}
	
	// Shoot the weapon...
	// Don't actually create an object, just use our
	// parent's projectile object to do vector calculations...
	
	CProjectile* pProj = m_pParent->GetVecProjectile();
	if (pProj)
	{
		pProj->ForcedHitSetup(*this, info);
	}
	
	if (!bInfiniteAmmo)
	{
		m_pParent->DecrementAmmo(m_nAmmoId, m_pAmmoData->nAmmoPerShot);
	}

	
	// Change our flash socket.
	if( m_pBarrelData && m_pBarrelData->nNumFlashSockets > 1 )
	{
		++m_nCurrentFlashSocket;
		if( m_nCurrentFlashSocket >= m_pBarrelData->nNumFlashSockets )
		{
			m_nCurrentFlashSocket = 0;
		}
	}
	
	// See if we need to turn off cloaking
	if(m_pBarrelData && m_pBarrelData->bDecloak)
	{
		CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hOwner);
		
		if(pCharacter)
		{
			pCharacter->ForceCloakOff(LTTRUE);
		}
	}
	
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateProjectile
//
//	PURPOSE:	Create the approprite projectile to fire.
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::CreateProjectile(const DRotation & rRot, const WFireInfo & info)
{
	if ( !m_pAmmoData || !m_pAmmoData->pProjectileFX || !(m_pAmmoData->pProjectileFX->szClass[0])) return DFALSE;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-Projectile", m_pAmmoData->pProjectileFX->szName );
#endif

	ROT_COPY(theStruct.m_Rotation, rRot);
	theStruct.m_Pos = info.vFirePos;
	theStruct.m_UserData = LTTRUE;

	HCLASS hClass = g_pServerDE->GetClass(m_pAmmoData->pProjectileFX->szClass);

	if (hClass)
	{
		CProjectile* pProj = (CProjectile*)g_pServerDE->CreateObject(hClass, &theStruct);
		if (pProj)
		{
			pProj->Setup(this, info);
			return DTRUE;
		}
	}

	return DFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SetModelObject()
//
//	PURPOSE:	Set the object associated with the weapon model
//
// ----------------------------------------------------------------------- //

void CWeapon::SetModelObject(const LTSmartLink & hObj)
{
	m_hModelObject = hObj;

	if (m_hModelObject) 
	{
		g_pServerDE->SetModelLooping(m_hModelObject, DFALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetFlashSocketName()
//
//	PURPOSE:	Gives the "current" flash socket.  Is alternated by shots.
//
// ----------------------------------------------------------------------- //

const char * CWeapon::GetFlashSocketName() const
{ 
	if( !m_pBarrelData ) 
		return LTNULL;
	
	_ASSERT( m_nCurrentFlashSocket < (uint32)m_pBarrelData->nNumFlashSockets );
	if( m_nCurrentFlashSocket < (uint32)m_pBarrelData->nNumFlashSockets )
		return m_pBarrelData->szFlashSocket[m_nCurrentFlashSocket];

	return LTNULL;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetFlashSocketNames()
//
//	PURPOSE:	Gives the n-th flash socket.
//
// ----------------------------------------------------------------------- //

const char * CWeapon::GetFlashSocketName(uint32 n) const
{
	if( !m_pBarrelData ) 
		return LTNULL;
	
	_ASSERT( n < (uint32)m_pBarrelData->nNumFlashSockets );
	if( n < (uint32)m_pBarrelData->nNumFlashSockets )
	{
		return m_pBarrelData->szFlashSocket[n];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetNumFlashSockets()
//
//	PURPOSE:	Returns the number of flash sockets.
//
// ----------------------------------------------------------------------- //

uint8 CWeapon::GetNumFlashSockets() const
{
	if( !m_pBarrelData ) 
		return 0;
	
	return m_pBarrelData->nNumFlashSockets;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	if (!hWrite) return;

//	m_hOwner set by constructor.
	*hWrite << m_hModelObject;

	*hWrite << m_bHave;
	hWrite->WriteDWord(m_nAmmoInClip);
	hWrite->WriteByte(m_nCurrentFlashSocket);

	*hWrite << m_nWeaponId;
	*hWrite << m_nBarrelId;
	*hWrite << m_nAmmoId;

	// m_pParent set by constructor.

	// m_pWeaponData restored through g_pWeaponMgr.
	// m_pBarrelData restored through g_pWeaponMgr.
	// m_pAmmoData restored through g_pWeaponMgr.
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	if (!hRead) return;

//	m_hOwner set by constructor.
	*hRead >> m_hModelObject;

	*hRead >> m_bHave;
	m_nAmmoInClip = hRead->ReadDWord();
	m_nCurrentFlashSocket = hRead->ReadByte();

	*hRead >> m_nWeaponId;
	*hRead >> m_nBarrelId;
	*hRead >> m_nAmmoId;

	// m_pParent set by constructor.

	m_pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	m_pBarrelData = g_pWeaponMgr->GetBarrel(m_nBarrelId);
	m_pAmmoData = g_pWeaponMgr->GetAmmo(m_nAmmoId);
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CacheFiles
//
//	PURPOSE:	Cache files used by weapon.
//
// ----------------------------------------------------------------------- //

void CWeapon::CacheFiles()
{
	// Too late to cache files?

	if (!g_pWeaponMgr || !(g_pServerDE->GetServerFlags() & SS_CACHING)) return;

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (pWeaponData)
	{
		pWeaponData->Cache(g_pWeaponMgr);
	}
}

