// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.cpp
//
// PURPOSE : Weapons container object - Implementation
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponMgr.h"
#include "Weapons.h"
#include "PlayerObj.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "HHWeaponModel.h"
#include "PredShoulderCannon.h"
#include "Attachments.h"
#include "GameServerShell.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CWeapons()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CWeapons::CWeapons()
{
	m_pWeapons	 = DNULL;
	m_pAmmoCount    = DNULL;
	m_pInfinitePool = DNULL;
	m_nCurWeapon = -1;
	m_hCharacter	 = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::~CWeapons()
//
//	PURPOSE:	Destructor - deallocate weapons
//
// ----------------------------------------------------------------------- //

CWeapons::~CWeapons()
{
	DeleteAllocations();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Init()
//
//	PURPOSE:	Initialize weapons
//
// ----------------------------------------------------------------------- //

DBOOL CWeapons::Init(HOBJECT hCharacter, HOBJECT hWeaponModel, int nNumWeapons /* = -1 */, int nNumAmmoPools /* = -1 */)
{
	m_hCharacter = hCharacter;
	m_hWeaponModel = hWeaponModel;

	DeleteAllocations();

	if( nNumWeapons < 0 )
	{
		nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	}

	if( nNumAmmoPools < 0 )
	{
		nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();
	}

	if (nNumWeapons > 0)
	{
		m_pWeapons = new CWeapon* [nNumWeapons];
		if (m_pWeapons)
		{
			memset(m_pWeapons, 0, sizeof(CWeapon*) * nNumWeapons);
		}
		else
		{
			return DFALSE;
		}

	}

	if (nNumAmmoPools > 0)
	{
		m_pAmmoCount = new int [nNumAmmoPools];
		m_pInfinitePool = new LTBOOL [nNumAmmoPools];
		if (m_pAmmoCount && m_pInfinitePool)
		{
			memset(m_pAmmoCount, 0, sizeof(int) * nNumAmmoPools);
			memset(m_pInfinitePool, 0, sizeof(LTBOOL) * nNumAmmoPools);
		}
		else
		{
			delete m_pWeapons;  m_pWeapons = LTNULL;
			delete m_pAmmoCount; m_pAmmoCount = LTNULL;
			delete m_pInfinitePool;  m_pInfinitePool = LTNULL;

			return DFALSE;
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //
		
DDWORD CWeapons::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DBYTE)lData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DBYTE)lData);
		}
		break;
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CWeapons::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_ADDWEAPON:
		{	
			if (!AddWeapon(pObject, hSender, hRead)) return 0;
		}
		break;

		case MID_AMMO:
		{	
			if (!HandleAmmoPickup(pObject, hSender, hRead)) return 0;
		}
		break;

		default : break;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::FillAllClips
//
//	PURPOSE:	Send a message to client to fill all the clips
//
// ----------------------------------------------------------------------- //

void CWeapons::FillAllClips()
{
	if (IsPlayer(m_hCharacter))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hCharacter);
		if (!pPlayer) return;

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
			g_pServerDE->WriteToMessageByte(hMessage, IC_FILL_ALL_CLIPS_ID);
			g_pServerDE->EndMessage(hMessage);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::FillClip
//
//	PURPOSE:	Send a message to client to fill a clip
//
// ----------------------------------------------------------------------- //

void CWeapons::FillClip(DBYTE nWeaponId)
{
	if (IsPlayer(m_hCharacter))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hCharacter);
		if (!pPlayer) return;

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
			g_pServerDE->WriteToMessageByte(hMessage, IC_FILL_CLIP_ID);
			g_pServerDE->WriteToMessageByte(hMessage, nWeaponId);
			g_pServerDE->EndMessage(hMessage);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddWeapon
//
//	PURPOSE:	Add a new weapon
//
// ----------------------------------------------------------------------- //

DBOOL CWeapons::AddWeapon(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{

	DBOOL	bHadIt	  = DTRUE;
	DBOOL   bPickedUp = DFALSE;
	DBYTE	nWeaponId = g_pServerDE->ReadFromMessageByte(hRead);
	DBYTE	nAmmoPoolId   = g_pServerDE->ReadFromMessageByte(hRead);
	int		nAmmo	  = (int) g_pServerDE->ReadFromMessageFloat(hRead);
	DBOOL	bClientHUDNotify  = g_pServerDE->ReadFromMessageByte(hRead);

	if (IsValidIndex(nWeaponId))
	{
		if (m_pWeapons && m_pWeapons[nWeaponId] )
		{
			bPickedUp = AddAmmoToPool(nAmmoPoolId, nAmmo);
		}
		else
		{
			bHadIt	  = DFALSE;
			bPickedUp = DTRUE;
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if(!pWeapon) return LTFALSE;
			BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[PRIMARY_BARREL]);
			if(!pBarrel) return LTFALSE;

			// Aquire the weapon and ammo
			ObtainWeapon(nWeaponId, pBarrel->nId, nAmmoPoolId, nAmmo, LTFALSE);
		}
	}

	if (bPickedUp)
	{
		// Send the appropriate message to the client...

		if (IsPlayer(m_hCharacter))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hCharacter);
			if (!pPlayer) return DFALSE;

			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
				g_pServerDE->WriteToMessageByte(hMessage, IC_WEAPON_PICKUP_ID);
				g_pServerDE->WriteToMessageByte(hMessage, nWeaponId);
				g_pServerDE->WriteToMessageByte(hMessage, nAmmoPoolId);
				g_pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)GetAmmoPoolCount(nAmmoPoolId));
				g_pServerDE->WriteToMessageByte(hMessage, bClientHUDNotify);
				g_pServerDE->EndMessage(hMessage);
			}
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::HandleAmmoPickup
//
//	PURPOSE:	Collect ammo from a box
//
// ----------------------------------------------------------------------- //

DBOOL CWeapons::HandleAmmoPickup(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	DBYTE	nAmmoPoolId;
	int		nAmmo;

	nAmmoPoolId	= g_pServerDE->ReadFromMessageByte(hRead);
	nAmmo		= (int) g_pServerDE->ReadFromMessageFloat(hRead);
	DBOOL		bClientHUDNotify  = g_pServerDE->ReadFromMessageByte(hRead);

	int taken = AddAmmoToPool(nAmmoPoolId, nAmmo);

	// Send the appropriate message to the client...
	if (IsPlayer(m_hCharacter))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hCharacter);
		if (!pPlayer) return DFALSE;

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
			g_pServerDE->WriteToMessageByte(hMessage, IC_WEAPON_PICKUP_ID);
			g_pServerDE->WriteToMessageByte(hMessage, WMGR_INVALID_ID);
			g_pServerDE->WriteToMessageByte(hMessage, nAmmoPoolId);
			g_pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)taken);
			g_pServerDE->WriteToMessageByte(hMessage, bClientHUDNotify);
			g_pServerDE->EndMessage(hMessage);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddAmmo
//
//	PURPOSE:	Add ammo to a specific ammo type
//
// ----------------------------------------------------------------------- //

int CWeapons::AddAmmo(int nAmmoId, int nAmount)
{
	//get ammo data
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);

	//test
	if (!pAmmo) return 0;

	//get pool data
	AMMO_POOL* pPool =  g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

	//test
	if(!pPool) return 0;

	//handle adding the ammo to the pool
	return AddAmmoToPool(pPool->nId, nAmount);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddAmmoToPool
//
//	PURPOSE:	Add ammo to a specific ammo pool using POOL ID
//
// ----------------------------------------------------------------------- //

int CWeapons::AddAmmoToPool(int nAmmoPoolId, int nAmount)
{
	//get pool data
	AMMO_POOL* pPool =  g_pWeaponMgr->GetAmmoPool(nAmmoPoolId);

	//test
	if (!pPool) return 0;

	//get some data
	int nMaxAmmo = pPool->GetMaxAmount(m_hCharacter);
	int nMaxTaken = nMaxAmmo - m_pAmmoCount[nAmmoPoolId];
	int taken = Min(nAmount,nMaxTaken);

	//add the ammo taken to the pool
	m_pAmmoCount[nAmmoPoolId] += taken;

	//bounds check, this really isn't necessary but just in case
	//something goes really wacky.  In addition this only gets called once per
	//power-up so it's really no big deal at all.
	if (m_pAmmoCount[nAmmoPoolId] > nMaxAmmo) 
	{
		m_pAmmoCount[nAmmoPoolId] = nMaxAmmo;
	}
	else if (m_pAmmoCount[nAmmoPoolId] < 0)
	{
		m_pAmmoCount[nAmmoPoolId] = 0;
	}

	return taken;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::DecrementAmmo
//
//	PURPOSE:	Decrement the specified ammo count
//
// ----------------------------------------------------------------------- //

void CWeapons::DecrementAmmo(int nAmmoId, int nAmount)
{
	if (!IsValidAmmoId(nAmmoId)) return;

	AMMO* pAmmo =  g_pWeaponMgr->GetAmmo(nAmmoId);

	if(!pAmmo) return;

	AMMO_POOL* pPool =  g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

	if(!pPool) return;

	int nPoolId = pPool->nId;
	
	if( !m_pInfinitePool[nPoolId] )
	{
		m_pAmmoCount[nPoolId] -= nAmount;
	}

	if (m_pAmmoCount[nPoolId] < 0)
	{
		m_pAmmoCount[nPoolId] = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::DecrementAmmo
//
//	PURPOSE:	Decrement the specified ammo count
//
// ----------------------------------------------------------------------- //

void CWeapons::DecrementAmmoPool(int nAmmoPoolId, int nAmount)
{
	if (!IsValidAmmoPoolId(nAmmoPoolId)) return;

	AMMO_POOL* pPool =  g_pWeaponMgr->GetAmmoPool(nAmmoPoolId);

	if(!pPool) return;

	int nPoolId = pPool->nId;
	
	if( !m_pInfinitePool[nPoolId] )
	{
		m_pAmmoCount[nPoolId] -= nAmount;
	}

	if (m_pAmmoCount[nPoolId] < 0)
	{
		m_pAmmoCount[nPoolId] = 0;
	}

	if(m_pAmmoCount[nPoolId] > pPool->GetMaxAmount(m_hCharacter))
	{
		m_pAmmoCount[nPoolId] = pPool->GetMaxAmount(m_hCharacter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::SetAmmo
//
//	PURPOSE:	Set the ammount of ammo for a specific ammo type
//
// ----------------------------------------------------------------------- //

DBOOL CWeapons::SetAmmo(int nAmmoPoolId, int nAmount)
{
	AMMO_POOL* pAmmoPool = g_pWeaponMgr->GetAmmoPool(nAmmoPoolId);
	if (!pAmmoPool) return DFALSE;

	int nMaxAmmo = pAmmoPool->GetMaxAmount(m_hCharacter);

	// Set to max if less than 0...

	if (nAmount < 0)
	{
		nAmount = nMaxAmmo;
	}

	m_pAmmoCount[nAmmoPoolId] = nAmount;

	if (m_pAmmoCount[nAmmoPoolId] > nMaxAmmo) 
	{
		m_pAmmoCount[nAmmoPoolId] = nMaxAmmo;
	}

	return DTRUE; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::SetAmmoPool
//
//	PURPOSE:	Set the ammount of ammo for a specific ammo pool type
//
// ----------------------------------------------------------------------- //

DBOOL CWeapons::SetAmmoPool(const char * szPoolName, int nAmmo)
{
	AMMO_POOL* pAmmoPool = g_pWeaponMgr->GetAmmoPool(szPoolName);

	if (!pAmmoPool) return DFALSE;

	int nMaxAmmo = pAmmoPool->GetMaxAmount(m_hCharacter);

	// Set to max if less than 0...

	if (nAmmo < 0)
	{
		nAmmo = nMaxAmmo;
	}

	m_pAmmoCount[pAmmoPool->nId] = nAmmo;

	if (m_pAmmoCount[pAmmoPool->nId] > nMaxAmmo) 
	{
		m_pAmmoCount[pAmmoPool->nId] = nMaxAmmo;
	}

	return DTRUE; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ObtainWeapon()
//
//	PURPOSE:	Mark a specific weapon as aquired
//
// ----------------------------------------------------------------------- //
void CWeapons::ObtainWeapon( DBYTE nWeaponId, 
							 uint8 nBarrelId /* = BARREL_DEFAULT_ID */, 
							 uint8 nAmmoPoolId /* = AMMO_DEFAULT_ID */, 
						     int nDefaultAmmo /* = -1 */, 
							 DBOOL bNotifyClient /* = DFALSE */,
							 LTBOOL bClientHudNotify /* = LTTRUE */)
{
	if (!m_pWeapons || !IsValidIndex(nWeaponId)) return;

	// If necessary set the ammo type based on the weapon's default
	// ammo...

	if(nBarrelId == BARREL_DEFAULT_ID)
	{
		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (!pWeaponData) return;

		nBarrelId = pWeaponData->nDefaultBarrelType;
	}


	uint8 nAmmoId=AMMO_DEFAULT_ID;

	if (nAmmoId == AMMO_DEFAULT_ID)
	{
		BARREL* pBarrel = g_pWeaponMgr->GetBarrel(nBarrelId);
		if (!pBarrel) return;

		nAmmoId = pBarrel->nAmmoType;
	}
	else
	{
		BARREL* pBarrel = g_pWeaponMgr->GetBarrel(nBarrelId);
		if (!pBarrel) return;

		nAmmoId = pBarrel->nAmmoType;
	}

	AMMO_POOL* pPool=LTNULL;

	if(nAmmoPoolId == AMMO_DEFAULT_ID)
	{
		BARREL* pBarrel = g_pWeaponMgr->GetBarrel(nBarrelId);
		if (!pBarrel) return;

		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);
		if(!pAmmo) return;

		//get ammo pool info
		pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);
	}
	else
	{
		//get ammo pool info
		pPool = g_pWeaponMgr->GetAmmoPool(nAmmoPoolId);
	}
	if(!pPool) return;

	_ASSERT( g_pWeaponMgr->GetWeapon(nWeaponId) );
	_ASSERT( g_pWeaponMgr->GetBarrel(nBarrelId) );
	_ASSERT( g_pWeaponMgr->GetAmmo(nAmmoId) );

	// Give us the weapon!

	if( !m_pWeapons[nWeaponId] )
	{
		m_pWeapons[nWeaponId] = new CWeapon(this, m_hCharacter, nWeaponId, nBarrelId, nAmmoId);

		ASSERT( m_pWeapons[nWeaponId] );
		if( !m_pWeapons[nWeaponId] )
			return;

		// Fill the initial clip
		FillClip(nWeaponId);
	}
	else
	{
		m_pWeapons[nWeaponId]->SetBarrelId(nBarrelId);
		m_pWeapons[nWeaponId]->SetAmmoId(nAmmoId);
	}
	
	// Set the weapon's default ammo if appropriate...

	if (nDefaultAmmo >= 0)
	{
		SetAmmo(pPool->nId, nDefaultAmmo + GetAmmoCount(nAmmoId));
	}

	// Notify the client if this is a player's weapon, and the flag
	// is set...

	if (bNotifyClient)
	{

		// Send the appropriate message to the client...

		if (IsPlayer(m_hCharacter))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hCharacter);
			if (!pPlayer) return;

			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
				g_pServerDE->WriteToMessageByte(hMessage, IC_WEAPON_PICKUP_ID);
				g_pServerDE->WriteToMessageByte(hMessage, nWeaponId);
				g_pServerDE->WriteToMessageByte(hMessage, pPool->nId);
				g_pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)GetAmmoCount(nAmmoId));
				g_pServerDE->WriteToMessageByte(hMessage, bClientHudNotify);
				g_pServerDE->EndMessage(hMessage);
			}
			
			// If there isn't currently a weapon selected, or we're
			// on our melee weapon, Select the new weapon..

			if (m_nCurWeapon < 0)
			{
				pPlayer->ChangeWeapon(g_pWeaponMgr->GetCommandId(nWeaponId));
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ChangeWeapon()
//
//	PURPOSE:	Change to a new weapon
//
// @parm the weapon to go to
// @rdef was the switch allowed?
//
// ----------------------------------------------------------------------- //

DBOOL CWeapons::ChangeWeapon(DBYTE nNewWeapon)
{
	if (nNewWeapon == m_nCurWeapon) return DTRUE;

	if (!m_pWeapons ||!IsValidWeapon(nNewWeapon)) return DFALSE;

	// Set this as our current weapon...

	m_nCurWeapon = nNewWeapon;


	// Change our weapon model to the newly selected weapon

	if (IsKindOf(m_hCharacter, "CCharacter"))
	{
		CCharacter* pChar = (CCharacter*)g_pServerDE->HandleToObject(m_hCharacter);
		if (pChar) 
		{
			WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nCurWeapon);
			if (!pWeaponData) return DFALSE;

			if (!pWeaponData->szHHModel[0]) return DFALSE;

			g_pServerDE->SetModelFilenames(m_hWeaponModel, pWeaponData->szHHModel, pWeaponData->szHHSkins[0]);

			DDWORD dwWeaponModelFlags = g_pServerDE->GetObjectFlags(m_hWeaponModel);
/* 
	this is commented out because CCharacter will not have SetObjectFlags 
	yet, since the aggregate gets the initial update first. 
			DDWORD dwCharacterFlags = g_pServerDE->GetObjectFlags(m_hCharacter);
			if ( !(dwCharacterFlags & FLAG_VISIBLE) )
			{
				dwWeaponModelFlags &= ~FLAG_VISIBLE;
			}
			else
*/			{
				dwWeaponModelFlags |= FLAG_VISIBLE;
			}

			g_pServerDE->SetObjectFlags(m_hWeaponModel, dwWeaponModelFlags);

			// Associated our hand held weapon with our weapon...

			if (m_hWeaponModel)
			{
				CHHWeaponModel* pModel = (CHHWeaponModel*)g_pServerDE->HandleToObject(m_hWeaponModel);
				if (pModel) 
				{
					CWeapon* pWeapon = m_pWeapons[m_nCurWeapon];
					pModel->SetParent(pWeapon);
					pWeapon->SetModelObject(m_hWeaponModel);
//					pWeapon->CacheFiles();
				}
			}

			UpdateCharacterWeaponFX(pChar,pWeaponData);

			// make sure we kill any old fire sound...
			if(g_pGameServerShell->GetGameType() != SP_STORY)
			{
				CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pServerDE->HandleToObject(m_hCharacter));

				if(pPlayer && pPlayer->IsPlayingLoopedSound())
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
					g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
					g_pLTServer->WriteToMessageObject(hMessage, pChar->m_hObject);
					g_pLTServer->WriteToMessageByte(hMessage, CFX_KILL_LOOP_FIRE);
					g_pLTServer->EndMessage(hMessage);

					pPlayer->SetPlayingLoopFireSound(LTFALSE);
				}
			}

		}
	}

	// Give use a fresh clip.

	m_pWeapons[m_nCurWeapon]->ReloadClip();

	return DTRUE;
}


void UpdateCharacterWeaponFX(CCharacter * pChar, const WEAPON * pWeaponData )
{
	if( !pChar )
		return;

	// If we don't have weapon data, be sure the fx are turned off.
	if( !pWeaponData )
	{
		pChar->SetTargetingSprite();
		pChar->SetFlameThrowerFX();
		return;
	}

	//see if the weapon has special targeting
	switch (pWeaponData->nTargetingType)
	{
	case (1):
		{
			pChar->SetTargetingSprite(pChar->HasMask());
		}
		break;

	default:
		{
			pChar->SetTargetingSprite();
		}
		break;
	}

	if(stricmp(pWeaponData->szName, "Flame_Thrower") == 0 || stricmp(pWeaponData->szName, "Flame_Thrower_MP") == 0 || stricmp(pWeaponData->szName, "Exosuit_Flame_Thrower") == 0 )
		pChar->SetFlameThrowerFX(LTTRUE);
	else
		pChar->SetFlameThrowerFX();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::DeselectCurWeapon()
//
//	PURPOSE:	Deselect the current weapon
//
// ----------------------------------------------------------------------- //

void CWeapons::DeselectCurWeapon()
{
	if (IsValidWeapon(m_nCurWeapon))
	{
		if( m_hWeaponModel )
		{
			CCharacter* pChar = dynamic_cast<CCharacter*>(g_pServerDE->HandleToObject(m_hCharacter));
			if (pChar) 
			{
				g_pServerDE->SetObjectFlags(m_hWeaponModel, g_pServerDE->GetObjectFlags(m_hWeaponModel) & ~FLAG_VISIBLE);
			}
		}
	}

	m_nCurWeapon = -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetWeaponAmmoCount()
//
//	PURPOSE:	Return the ammo amount of the specified selected weapon
//
// ----------------------------------------------------------------------- //

int CWeapons::GetWeaponAmmoCount(int nWeapon)
{
	if (!g_pWeaponMgr || !m_pWeapons || !m_pAmmoCount) return 0;

	if (IsValidIndex(nWeapon) && m_pWeapons[nWeapon])
	{
		return GetAmmoCount(m_pWeapons[nWeapon]->GetAmmoId());
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetAmmoCount()
//
//	PURPOSE:	Return the amount of the specified type of ammo
//
// ----------------------------------------------------------------------- //

int CWeapons::GetAmmoPoolCount(int nAmmoPoolId)
{
	if (!IsValidAmmoPoolId(nAmmoPoolId)) return 0;

	return m_pAmmoCount[nAmmoPoolId];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetAmmoCount()
//
//	PURPOSE:	Return the amount of the specified type of ammo
//
// ----------------------------------------------------------------------- //

int CWeapons::GetAmmoCount(int nAmmoId)
{
	if (!IsValidAmmoId(nAmmoId)) return 0;

	AMMO *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);

	if(!pAmmo) return 0;

	AMMO_POOL *pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

	if(!pPool) return 0;

	return m_pAmmoCount[pPool->nId];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CWeapons::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{

	*hWrite << m_hCharacter;
	*hWrite << m_hWeaponModel;
	*hWrite << m_nCurWeapon;

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if( !m_pWeapons )
	{
		nNumWeapons = 0;
	}

	*hWrite << nNumWeapons;

	int nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();
	if (!m_pAmmoCount)
	{
		nNumAmmoPools = 0;
	}

	*hWrite << nNumAmmoPools;

	{for (int i=0; i < nNumWeapons; i++)
	{
		hWrite->WriteByte( m_pWeapons[i] != LTNULL );
		if (m_pWeapons[i]) 
		{
			m_pWeapons[i]->Save(hWrite, nType);
		}
	}}


	{for (int i=0; i < nNumAmmoPools; i++)
	{
		*hWrite << m_pAmmoCount[i];
		*hWrite << m_pInfinitePool[i];
	}}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapons::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	int i = 0;

	DeleteAllocations();

	*hRead >> m_hCharacter;
	*hRead >> m_hWeaponModel;
	*hRead >> m_nCurWeapon;

	int nNumWeapons = 0;
	int nNumAmmoPools = 0;
	*hRead >> nNumWeapons;
	*hRead >> nNumAmmoPools;

	Init(m_hCharacter, m_hWeaponModel, nNumWeapons, nNumAmmoPools);

	for (i=0; i < nNumWeapons; i++)
	{

		if (hRead->ReadByte())
		{
			ASSERT( m_pWeapons );

			if( m_pWeapons )
			{
				m_pWeapons[i] = new CWeapon(this, m_hCharacter, 0,0,0);

				ASSERT( m_pWeapons[i] );
				if (m_pWeapons[i]) 
				{
					m_pWeapons[i]->Load(hRead, nType);
				}
			}
			
		}
	}

	for (i=0; i < nNumAmmoPools; i++)
	{
		ASSERT( m_pAmmoCount );
		if (m_pAmmoCount)
		{
			*hRead >> m_pAmmoCount[i];
		}

		ASSERT( m_pInfinitePool );
		if(m_pInfinitePool)
		{
			*hRead >> m_pInfinitePool[i];
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Reset()
//
//	PURPOSE:	Reset all the weapons (i.e., we don't have any of them)
//
// ----------------------------------------------------------------------- //

void CWeapons::Reset()
{
	int i = 0;
	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

	for (i=0; i < nNumWeapons; i++)
	{
		if (m_pWeapons && m_pWeapons[i])
		{
			delete m_pWeapons[i];
			m_pWeapons[i] = LTNULL;
		}
	}

	int nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();

	for (i=0; i < nNumAmmoPools; i++)
	{
		if (m_pAmmoCount)
		{
			m_pAmmoCount[i] = 0;
			m_pInfinitePool[i] = LTFALSE;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Reset()
//
//	PURPOSE:	Reset all the weapons (i.e., we don't have any of them)
//
// ----------------------------------------------------------------------- //

void CWeapons::DeleteAllocations()
{
	if (m_pWeapons && g_pWeaponMgr)
	{
		for (int i=0; i < g_pWeaponMgr->GetNumWeapons(); i++)
		{
			delete m_pWeapons[i];
			m_pWeapons[i] = LTNULL;
		}

		delete [] m_pWeapons;
		m_pWeapons = LTNULL;
	}

	delete [] m_pAmmoCount;
	m_pAmmoCount = LTNULL;

	delete [] m_pInfinitePool;
	m_pInfinitePool = LTNULL;

}
