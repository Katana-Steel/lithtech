// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponMgr.cpp
//
// PURPOSE : WeaponMgr implementation - Controls attributes of all weapons
//
// CREATED : 12/02/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponMgr.h"
#include "DebrisMgr.h"
#include "PlayerStats.h"
#include "FXButeMgr.h"

#ifdef _CLIENTBUILD
//**************** Client only includes
#include "../ClientShellDLL/GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
#else
//**************** Server only includes
#include "../ObjectDLL/GameServerShell.h"
#include "../ObjectDLL/PlayerObj.h"
#endif

const char* const WMGR_WEAPON_SET_TAG			= "WeaponSet";
const char* const WMGR_WEAPON_SET_ITEM			= "LegalWeapon";
const char* const WMGR_WEAPON_SET_DEFAULT		= "DefaultWeapon";
const char* const WMGR_WEAPON_SET_CANPICKUP		= "CanPickup";

const char* const WMGR_WEAPON_TAG				= "Weapon";
const char* const WMGR_WEAPON_NAMEID			= "NameId";
const char* const WMGR_WEAPON_DESCRIPTIONID		= "DescriptionId";
const char* const WMGR_WEAPON_ANITYPE			= "AniType";
const char* const WMGR_WEAPON_ICON				= "Icon";
const char* const WMGR_WEAPON_PICKUP_ICON		= "PickupIcon";
const char* const WMGR_WEAPON_PICKUP_NAME		= "PickupName";
const char* const WMGR_WEAPON_POS				= "Pos";
const char* const WMGR_WEAPON_PVMODEL			= "PVModel";
const char* const WMGR_WEAPON_PVSKIN			= "PVSkins";
const char* const WMGR_WEAPON_HHMODEL			= "HHModel";
const char* const WMGR_WEAPON_HHSKIN			= "HHSkins";
const char* const WMGR_WEAPON_HHSCALE			= "HHModelScale";
const char* const WMGR_WEAPON_SELECTSND			= "SelectSnd";
const char* const WMGR_WEAPON_DESELECTSND		= "DeselectSnd";
const char* const WMGR_WEAPON_ALPHA_FLAG		= "AlphaFlag";
const char* const WMGR_WEAPON_PVFXNAME			= "PVFXName";
const char* const WMGR_WEAPON_BARREL			= "Barrel";
const char* const WMGR_WEAPON_COUNTER_TYPE		= "CounterType";
const char* const WMGR_WEAPON_TARGETING_TYPE	= "TargetingType";
const char* const WMGR_WEAPON_LOOP_FIRE_ANIM	= "LoopFireAnim";
const char* const WMGR_WEAPON_LOOP_ALTFIRE_ANIM	= "LoopAltFireAnim";
//const char* const WMGR_WEAPON_TARGET_SPRITE		= "TargetingSprite";
//const char* const WMGR_WEAPON_TARGET_SCALE		= "TargetingSpriteScale";
const char* const WMGR_WEAPON_CROSSHAIR_IMAGE	= "CrosshairImage";
const char* const WMGR_WEAPON_CROSSHAIR_SCALE	= "CrosshairScale";
const char* const WMGR_WEAPON_CROSSHAIR_ALPHA	= "CrosshairAlpha";
const char* const WMGR_WEAPON_SPEEDMOD			= "SpeedMod";
const char* const WMGR_WEAPON_AIMING_PITCH_SET	= "AimingPitchSet";
const char* const WMGR_WEAPON_MELEE				= "Melee";
const char* const WMGR_WEAPON_SAVE				= "Save";
const char* const WMGR_WEAPON_CACHE_MP			= "CacheInMP";

const char* const WMGR_BARREL_TAG				= "Barrel";
const char* const WMGR_BARREL_NAMEID			= "NameId";
const char* const WMGR_BARREL_MUZZLEPOS			= "MuzzlePos";
const char* const WMGR_BARREL_MINXPERTURB		= "MinXPerturb";
const char* const WMGR_BARREL_MAXXPERTURB		= "MaxXPerturb";
const char* const WMGR_BARREL_MINYPERTURB		= "MinYPerturb";
const char* const WMGR_BARREL_MAXYPERTURB		= "MaxYPerturb";
const char* const WMGR_BARREL_RANGE				= "Range";
const char* const WMGR_BARREL_MIN_RECOIL_AMOUNT	= "MinRecoilAmount";
const char* const WMGR_BARREL_MAX_RECOIL_AMOUNT	= "MaxRecoilAmount";
const char* const WMGR_BARREL_RECOIL_TIMES		= "RecoilTimes";
const char* const WMGR_BARREL_VECTORSPERROUND	= "VectorsPerRound";
const char* const WMGR_BARREL_PVMUZZLEFXNAME	= "PVMuzzleFXName";
const char* const WMGR_BARREL_HHBREACH			= "HHBreachOffset";
const char* const WMGR_BARREL_PVBREACH			= "PVBreachOffset";
const char* const WMGR_BARREL_HHMUZZLEFXNAME	= "HHMuzzleFXName";
const char* const WMGR_BARREL_FIRESND			= "FireSnd";
const char* const WMGR_BARREL_ALTFIRESND		= "AltFireSnd";
const char* const WMGR_BARREL_DRYFIRESND		= "DryFireSnd";
const char* const WMGR_BARREL_MISCSND			= "MiscSnd";
const char* const WMGR_BARREL_CLIPICON			= "ClipIcon";
const char* const WMGR_BARREL_FIRESNDRADIUS		= "FireSndRadius";
const char* const WMGR_BARREL_INFINITEAMMO		= "InfiniteAmmo";	
const char* const WMGR_BARREL_SHOTSPERCLIP		= "ShotsPerClip";
const char* const WMGR_BARREL_AMMONAME			= "AmmoName";
const char* const WMGR_BARREL_TYPE				= "BarrelType";
const char* const WMGR_BARREL_NEXT_BARREL		= "NextBarrel";
const char* const WMGR_BARREL_ALT_SKIN			= "PVAltSkin";
const char* const WMGR_BARREL_FLASH_SOCKET		= "FlashSocket";
const char* const WMGR_BARREL_DECLOAK			= "DecloakWhenFired";
const char* const WMGR_BARREL_NO_MULTI_IMPACT_SOUND	= "SingleImpactSound";
const char* const WMGR_BARREL_SAVE				= "Save";
const char* const WMGR_BARREL_DRAWAMMO			= "DisplayAmmo";

const char* const WMGR_AMMO_TAG				= "Ammo";
const char* const WMGR_AMMO_NAMEID			= "NameId";
const char* const WMGR_AMMO_TYPE			= "AmmoType";
const char* const WMGR_AMMO_POOLNAME		= "AmmoPoolName";
const char* const WMGR_AMMO_AMOUNT_PER_SHOT	= "AmmoPoolAmount";
const char* const WMGR_AMMO_INSTDAMAGE		= "InstDamage";
const char* const WMGR_AMMO_INSTDAMAGETYPE	= "InstDamageType";
const char* const WMGR_AMMO_AREADAMAGE		= "AreaDamage";
const char* const WMGR_AMMO_AREADAMAGETYPE	= "AreaDamageType";
const char* const WMGR_AMMO_AREADAMAGERADIUS= "AreaDamageRadius";
const char* const WMGR_AMMO_AREADAMAGEDUR	= "AreaDamageDuration";
const char* const WMGR_AMMO_PROGDAMAGE		= "ProgDamage";
const char* const WMGR_AMMO_PROGDAMAGETYPE	= "ProgDamageType";
const char* const WMGR_AMMO_PROGDAMAGEDUR	= "ProgDamageDuration";
const char* const WMGR_AMMO_IMPACTFX		= "ImpactFXName";
const char* const WMGR_AMMO_UWIMPACTFX		= "UWImpactFXName";
const char* const WMGR_AMMO_FIREFX			= "FireFXName";
const char* const WMGR_AMMO_TRACERFX		= "TracerFXName";
const char* const WMGR_AMMO_PROJECTILEFX	= "ProjectileFXName";
const char* const WMGR_AMMO_CASING_DELAY	= "ShellCasingDelay";
const char* const WMGR_AMMO_BUBBLE_TRAIL	= "MakeBubbleTrail";

const char* const WMGR_AMMOPOOL_TAG			= "AmmoPool";
const char* const WMGR_AMMOPOOL_NAMEID		= "NameId";
const char* const WMGR_AMMOPOOL_PUICON		= "PickupIcon";
const char* const WMGR_AMMOPOOL_COUNTICON	= "CounterIcon";
const char* const WMGR_AMMOPOOL_MAXAMOUNT	= "MaxAmount";
const char* const WMGR_AMMOPOOL_SPAWNEDAMOUNT	= "SpawnedAmount";
const char* const WMGR_AMMOPOOL_DEFAULTAMOUNT	= "DefaultAmount";
const char* const WMGR_AMMOPOOL_SAVE				= "Save";

static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];

CWeaponMgr* g_pWeaponMgr = LTNULL;

#ifndef _CLIENTBUILD

#include "FXButeMgr.h"

// Plugin statics

static LTBOOL			sm_bInitted;
static CWeaponMgr		sm_ButeMgr;
static CFXButeMgr		sm_FXButeMgr;
static CDebrisMgr		sm_DebrisMgr;

#endif // _CLIENTBUILD

namespace /* unnamed */
{
	template<typename T>
	class MatchName
	{
			const char * const m_szNameToMatch;

		public :

			MatchName(const char * szNameToMatch)
				: m_szNameToMatch(szNameToMatch) {}

			bool operator()(const T & data) const 
			{
				return 0 == _stricmp(m_szNameToMatch,data.szName);
			}
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CWeaponMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CWeaponMgr::CWeaponMgr()
{
	m_nNumAmmoPools = 0;
	m_pAmmoPools = LTNULL;

	m_nNumWeapons = 0;
	m_pWeapons = LTNULL;

	m_nNumAmmos = 0;
	m_pAmmos = LTNULL;

	m_nNumBarrels = 0;
	m_pBarrels = LTNULL;

	m_nNumWeaponSets = 0;
	m_pWeaponSets = LTNULL;

	m_pPlayerStats		= LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::~CWeaponMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CWeaponMgr::~CWeaponMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CountTags()
//
//	PURPOSE:	Counts the numbers of each type of attribute.
//
// ----------------------------------------------------------------------- //

bool CWeaponMgr::CountTags(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CWeaponMgr * const pButeMgr = (CWeaponMgr*)pData;

	const CString cstrType = pButeMgr->GetString(szTagName, "Type");

	if(  0 == cstrType.CompareNoCase(WMGR_AMMOPOOL_TAG) )
	{
		++pButeMgr->m_nNumAmmoPools;
	}
	else if(  0 == cstrType.CompareNoCase(WMGR_WEAPON_TAG) )
	{
		++pButeMgr->m_nNumWeapons;
	}
	else if(  0 == cstrType.CompareNoCase(WMGR_AMMO_TAG) )
	{
		++pButeMgr->m_nNumAmmos;
	}
	else if(  0 == cstrType.CompareNoCase(WMGR_BARREL_TAG) )
	{
		++pButeMgr->m_nNumBarrels;
	}
	else if(  0 == cstrType.CompareNoCase(WMGR_WEAPON_SET_TAG) )
	{
		++pButeMgr->m_nNumWeaponSets;
	}

	// Keep iterating.
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CountTags()
//
//	PURPOSE:	Loads each type of bute.
//
// ----------------------------------------------------------------------- //

bool CWeaponMgr::LoadAmmoPoolButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CWeaponMgr * const pButeMgr = (CWeaponMgr*)pData;

	const CString cstrType = pButeMgr->GetString(szTagName, "Type");

	if(  0 == cstrType.CompareNoCase(WMGR_AMMOPOOL_TAG) )
	{
		ASSERT( pButeMgr->m_pAmmoPools );

		if( pButeMgr->m_pAmmoPools[ pButeMgr->m_nNumAmmoPools ].Init(*pButeMgr,szTagName) )
		{
			pButeMgr->m_pAmmoPools[ pButeMgr->m_nNumAmmoPools ].nId = pButeMgr->m_nNumAmmoPools;
			++pButeMgr->m_nNumAmmoPools;
		}
	}

	return true;
}

bool CWeaponMgr::LoadAmmoButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CWeaponMgr * const pButeMgr = (CWeaponMgr*)pData;

	const CString cstrType = pButeMgr->GetString(szTagName, "Type");

	if(  0 == cstrType.CompareNoCase(WMGR_AMMO_TAG) )
	{
		ASSERT( pButeMgr->m_pAmmos );

		if( pButeMgr->m_pAmmos[ pButeMgr->m_nNumAmmos ].Init(*pButeMgr,szTagName) )
		{
			pButeMgr->m_pAmmos[ pButeMgr->m_nNumAmmos ].nId = pButeMgr->m_nNumAmmos;
			++pButeMgr->m_nNumAmmos;
		}
	}

	return true;
}

bool CWeaponMgr::LoadBarrelButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CWeaponMgr * const pButeMgr = (CWeaponMgr*)pData;

	const CString cstrType = pButeMgr->GetString(szTagName, "Type");

	if(  0 == cstrType.CompareNoCase(WMGR_BARREL_TAG) )
	{
		ASSERT( pButeMgr->m_pBarrels );

		if( pButeMgr->m_pBarrels[ pButeMgr->m_nNumBarrels ].Init(*pButeMgr,szTagName) )
		{
			pButeMgr->m_pBarrels[ pButeMgr->m_nNumBarrels ].nId = pButeMgr->m_nNumBarrels;
			++pButeMgr->m_nNumBarrels;
		}
	}

	return true;
}

bool CWeaponMgr::LoadWeaponButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CWeaponMgr * const pButeMgr = (CWeaponMgr*)pData;

	const CString cstrType = pButeMgr->GetString(szTagName, "Type");

	if(  0 == cstrType.CompareNoCase(WMGR_WEAPON_TAG) )
	{
		ASSERT( pButeMgr->m_pWeapons );

		if( pButeMgr->m_pWeapons[ pButeMgr->m_nNumWeapons ].Init(*pButeMgr,szTagName) )
		{
			pButeMgr->m_pWeapons[ pButeMgr->m_nNumWeapons ].nId = pButeMgr->m_nNumWeapons;
			++pButeMgr->m_nNumWeapons;
		}
	}

	return true;
}



bool CWeaponMgr::LoadWeaponSetButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CWeaponMgr * const pButeMgr = (CWeaponMgr*)pData;

	const CString cstrType = pButeMgr->GetString(szTagName, "Type");

	if(  0 == cstrType.CompareNoCase(WMGR_WEAPON_SET_TAG) )
	{
		ASSERT( pButeMgr->m_pWeaponSets );

		if( pButeMgr->m_pWeaponSets[ pButeMgr->m_nNumWeaponSets ].Init(*pButeMgr,szTagName) )
		{
			pButeMgr->m_pWeaponSets[ pButeMgr->m_nNumWeaponSets ].nId = pButeMgr->m_nNumWeaponSets;
			++pButeMgr->m_nNumWeaponSets;
		}
	}

	// Keep iterating.
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pWeaponMgr || !szAttributeFile) return LTFALSE;

	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	// Set up global pointer...
	g_pWeaponMgr = this;

#ifdef _CLIENTBUILD
	//get pointer to player stats
	m_pPlayerStats = g_pGameClientShell->GetPlayerStats();
#endif
	
	// Count the tags for each type....
	m_nNumAmmoPools = 0;
	m_nNumWeapons = 0;
	m_nNumAmmos = 0;
	m_nNumBarrels = 0;
	m_nNumWeaponSets = 0;
	m_buteMgr.GetTags(CountTags,this);

	
	// Allocate and read in the butes.
	// The order of the butes is important!  Weapons depends on barrels already be read.

	// Read ammo pools...
	if( m_nNumAmmoPools > 0 )
	{
		m_pAmmoPools = new AMMO_POOL[m_nNumAmmoPools];

		m_nNumAmmoPools = 0;
		m_buteMgr.GetTags(LoadAmmoPoolButes,this);
	}
	else
	{
		m_pAmmoPools = LTNULL;
	}

	// Read ammo...
	if( m_nNumAmmos > 0 )
	{
		m_pAmmos = new AMMO[m_nNumAmmos];

		m_nNumAmmos = 0;
		m_buteMgr.GetTags(LoadAmmoButes,this);
	}
	else
	{
		m_pAmmos = LTNULL;
	}
	
	// Read barrel...
	if( m_nNumBarrels > 0 )
	{
		m_pBarrels = new BARREL[m_nNumBarrels];

		m_nNumBarrels = 0;
		m_buteMgr.GetTags(LoadBarrelButes,this);
	}
	else
	{
		m_pBarrels = LTNULL;
	}

	// Read weapons...
	if( m_nNumWeapons > 0 )
	{
		m_pWeapons = new WEAPON[m_nNumWeapons];

		m_nNumWeapons = 0;
		m_buteMgr.GetTags(LoadWeaponButes,this);
	}
	else
	{
		m_pWeapons = LTNULL;
	}

	// Read weapon sets...
	if( m_nNumWeaponSets > 0 )
	{
		m_pWeaponSets = new WEAPON_SET[m_nNumWeaponSets];

		m_nNumWeaponSets = 0;
		m_buteMgr.GetTags(LoadWeaponSetButes,this);
	}
	else
	{
		m_pWeaponSets = LTNULL;
	}
	


	// Free up the bute mgr's memory...

	m_buteMgr.Term();


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::Term()
{
	g_pWeaponMgr = LTNULL;

	delete[] m_pAmmoPools;
	m_pAmmoPools = LTNULL;

	delete[] m_pWeapons;
	m_pWeapons = LTNULL;

	delete[] m_pAmmos;
	m_pAmmos = LTNULL;

	delete[] m_pBarrels;
	m_pBarrels = LTNULL;

	delete[] m_pWeaponSets;
	m_pWeaponSets = LTNULL;

	m_pPlayerStats = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::WriteFile()
//
//	PURPOSE:	Write necessary data back out to bute file
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponMgr::WriteFile() 
{
	// Re-init our bute mgr...

	m_buteMgr.Init(GBM_DisplayError); 
#ifdef _CLIENTBUILD
	Parse(g_pLTClient, WEAPON_DEFAULT_FILE);
#else
	Parse(g_pLTServer, WEAPON_DEFAULT_FILE);
#endif


	// Save the necessary weapon data...
	for( WEAPON* pCurWeapon = m_pWeapons; pCurWeapon < m_pWeapons + m_nNumWeapons; ++pCurWeapon)
	{
		pCurWeapon->Save(m_buteMgr);
	}

	// Save the necessary barrel data...
	for( BARREL* pCurBarrel = m_pBarrels; pCurBarrel < m_pBarrels + m_nNumBarrels; ++pCurBarrel)
	{
		pCurBarrel->Save(m_buteMgr);
	}

	// Save the file...

	DBOOL bRet = m_buteMgr.Save();
	m_buteMgr.Term();


	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::Reload()
//
//	PURPOSE:	Reload data from the bute file
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::Reload(ILTCSBase *pInterface)    
{ 
	Term(); 
	Init(pInterface); 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CalculateWeaponPath()
//
//	PURPOSE:	Utility function used to calculate a path based on the
//				perturb of the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::CalculateWeaponPath(WeaponPath & wp)
{
	BARREL* pBarrel = GetBarrel(wp.nBarrelId);
	if (!pBarrel) return;

	int nMinXPerturb = pBarrel->nMinXPerturb;
	int nMaxXPerturb = pBarrel->nMaxXPerturb;

	int nMinYPerturb = pBarrel->nMinYPerturb;
	int nMaxYPerturb = pBarrel->nMaxYPerturb;

	if ((nMinXPerturb <= nMaxXPerturb) && nMaxXPerturb > 0) 
	{
		float fPerturbRange = float(nMaxXPerturb - nMinXPerturb);

		int nPerturb = nMinXPerturb + (int) (fPerturbRange * wp.fPerturbR);
		DFLOAT fRPerturb = ((DFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

		wp.vPath += (wp.vR * fRPerturb);
	}

	if ((nMinYPerturb <= nMaxYPerturb) && nMaxYPerturb > 0) 
	{
		float fPerturbRange = float(nMaxYPerturb - nMinYPerturb);

		int nPerturb = nMinYPerturb + (int) (fPerturbRange * wp.fPerturbU);
		DFLOAT fUPerturb = ((DFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

		wp.vPath += (wp.vU * fUPerturb);
	}

	wp.vPath.Norm();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetPoolId(uint8 nAmmoId)
//
//	PURPOSE:	Returns the ammo pool id for a given ammo type
//
// ----------------------------------------------------------------------- //

int	CWeaponMgr::GetPoolId(uint8 nAmmoId)
{
	AMMO_POOL* pPool=LTNULL;

	AMMO* pAmmo = GetAmmo(nAmmoId);

	if(pAmmo)
		pPool = GetAmmoPool(pAmmo->szAmmoPool);

	if(pPool)
		return pPool->nId;

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeapon
//
//	PURPOSE:	Get the specified weapon struct
//
// ----------------------------------------------------------------------- //

WEAPON* CWeaponMgr::GetWeapon(int nWeaponId)
{
	if( IsValidWeapon(nWeaponId) )
	{
		return &m_pWeapons[nWeaponId];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeapon
//
//	PURPOSE:	Get the specified weapon struct
//
// ----------------------------------------------------------------------- //


WEAPON* CWeaponMgr::GetWeapon(const char* pWeaponName)
{
	if (!pWeaponName) return LTNULL;

	WEAPON* pFound = std::find_if(m_pWeapons, m_pWeapons + m_nNumWeapons, MatchName<WEAPON>(pWeaponName));

	if( pFound < m_pWeapons + m_nNumWeapons )
	{
		return pFound;
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeaponSet
//
//	PURPOSE:	Get the specified weapon set struct
//
// ----------------------------------------------------------------------- //

WEAPON_SET* CWeaponMgr::GetWeaponSet(int nSetId)
{
	if( nSetId >= 0 && nSetId < m_nNumWeaponSets )
	{
		return &m_pWeaponSets[nSetId];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeaponSet
//
//	PURPOSE:	Get the specified weapon set struct
//
// ----------------------------------------------------------------------- //

WEAPON_SET* CWeaponMgr::GetWeaponSet(const char* pSetName)
{
	if (!pSetName) return LTNULL;

	WEAPON_SET* pFound = std::find_if(m_pWeaponSets, m_pWeaponSets + m_nNumWeaponSets, MatchName<WEAPON_SET>(pSetName));

	if( pFound < m_pWeaponSets + m_nNumWeaponSets )
	{
		return pFound;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetAmmoPool
//
//	PURPOSE:	Get the specified ammo struct
//
// ----------------------------------------------------------------------- //

AMMO_POOL* CWeaponMgr::GetAmmoPool(int nAmmoPoolId)
{
	if( IsValidAmmoPoolType(nAmmoPoolId) )
	{
		return &m_pAmmoPools[nAmmoPoolId];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetAmmoPool
//
//	PURPOSE:	Get the specified ammo struct
//
// ----------------------------------------------------------------------- //

AMMO_POOL* CWeaponMgr::GetAmmoPool(const char* pAmmoPoolName)
{
	if (!pAmmoPoolName) return LTNULL;

	AMMO_POOL* pFound = std::find_if(m_pAmmoPools, m_pAmmoPools + m_nNumAmmoPools, MatchName<AMMO_POOL>(pAmmoPoolName));

	if( pFound < m_pAmmoPools + m_nNumAmmoPools )
	{
		return pFound;
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetAmmo
//
//	PURPOSE:	Get the specified ammo struct
//
// ----------------------------------------------------------------------- //

AMMO* CWeaponMgr::GetAmmo(int nAmmoId)
{
	if( IsValidAmmoType(nAmmoId) )
	{
		return &m_pAmmos[nAmmoId];
	}
	
	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetAmmo
//
//	PURPOSE:	Get the specified ammo struct
//
// ----------------------------------------------------------------------- //

AMMO* CWeaponMgr::GetAmmo(const char* pAmmoName)
{
	if (!pAmmoName) return LTNULL;

	AMMO* pFound = std::find_if(m_pAmmos, m_pAmmos + m_nNumAmmos, MatchName<AMMO>(pAmmoName));

	if( pFound < m_pAmmos + m_nNumAmmos )
	{
		return pFound;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetBarrel
//
//	PURPOSE:	Get the specified barrel struct
//
// ----------------------------------------------------------------------- //

BARREL* CWeaponMgr::GetBarrel(int nBarrelId)
{
	if( IsValidBarrel(nBarrelId) )
	{
		return &m_pBarrels[nBarrelId];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetBarrel
//
//	PURPOSE:	Get the specified barrel struct
//
// ----------------------------------------------------------------------- //

BARREL* CWeaponMgr::GetBarrel(const char* pBarrelName)
{
	if (!pBarrelName) return LTNULL;

	BARREL* pFound = std::find_if(m_pBarrels, m_pBarrels + m_nNumBarrels, MatchName<BARREL>(pBarrelName));

	if( pFound < m_pBarrels + m_nNumBarrels )
	{
		return pFound;
	}

	return LTNULL;
}

/////////////////////////////////////////////////////////////////////////////
//
//	W E A P O N  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::WEAPON
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WEAPON::WEAPON()
{
	int i = 0;

	nId					= WMGR_INVALID_ID;
	hName				= LTNULL;
	hDescription		= LTNULL;
	nAniType			= 0;
	nCounterType		= 0;
	nDefaultBarrelType	= 0;
	nTargetingType		= 0;
	nNumPVFXTypes		= 0;
	nAimingPitchSet		= 0;

	fCrosshairScale		= 1.0f;
	fCrosshairAlpha		= 1.0f;
	fSpeedMod			= 1.0f;

	bLoopFireAnim		= LTFALSE;
	bLoopAltFireAnim	= LTFALSE;
	bSave				= LTFALSE;
	bMPCache			= LTFALSE;

	vPos.Init();
	vHHScale.Init();

	szName[0]			= '\0';
	szIcon[0]			= '\0';
	szPickupIcon[0]		= '\0';
	szPickupName[0]		= '\0';
	szPVModel[0]		= '\0';
	szHHModel[0]		= '\0';
	szSelectSound[0]	= '\0';
	szDeselectSound[0]	= '\0';

	dwAlphaFlag			= 0;

	for(i=0 ; i < MAX_MODEL_TEXTURES ; i++)
	{
		szPVSkins[i][0] = '\0';
		szHHSkins[i][0]	= '\0';
	}

	for (i=0; i < WMGR_MAX_PVFX_TYPES; i++)
	{
		aPVFXTypes[i] = WMGR_INVALID_ID;
	}

	for (i=0; i < NUM_BARRELS; i++)
	{
		aBarrelTypes[i] = WMGR_INVALID_ID;
	}
}

WEAPON::~WEAPON()
{

#ifdef _CLIENTBUILD

	if(hName)
	{
		g_pLTClient->FreeString(hName);
		hName = LTNULL;
	}

	if(hDescription)
	{
		g_pLTClient->FreeString(hDescription);
		hDescription = LTNULL;
	}

#endif

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Init
//
//	PURPOSE:	Build the weapon struct
//
// ----------------------------------------------------------------------- //

DBOOL WEAPON::Init(CHierarchicalButeMgr & buteMgr, const char* aTagName)
{
	int i = 0;

	if (!aTagName) return LTFALSE;

#ifdef _CLIENTBUILD
	hName					= g_pLTClient->FormatString(buteMgr.GetInt(aTagName, WMGR_WEAPON_NAMEID));
	hDescription			= g_pLTClient->FormatString(buteMgr.GetInt(aTagName, WMGR_WEAPON_DESCRIPTIONID));
#endif

	nAniType				= buteMgr.GetInt(aTagName, WMGR_WEAPON_ANITYPE);
	nCounterType			= buteMgr.GetInt(aTagName, WMGR_WEAPON_COUNTER_TYPE);
	nTargetingType			= buteMgr.GetInt(aTagName, WMGR_WEAPON_TARGETING_TYPE);
	nAimingPitchSet			= (uint8)buteMgr.GetInt(aTagName, WMGR_WEAPON_AIMING_PITCH_SET);

	fCrosshairScale			= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_WEAPON_CROSSHAIR_SCALE);
	fCrosshairAlpha			= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_WEAPON_CROSSHAIR_ALPHA);
	fSpeedMod				= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_WEAPON_SPEEDMOD);
	bMelee					= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_MELEE);

	bLoopFireAnim			= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_LOOP_FIRE_ANIM);
	bLoopAltFireAnim		= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_LOOP_ALTFIRE_ANIM);
	bSave					= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_SAVE);
	bMPCache				= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_CACHE_MP);

	vPos					= buteMgr.GetVector(aTagName, WMGR_WEAPON_POS);
	vHHScale				= buteMgr.GetVector(aTagName, WMGR_WEAPON_HHSCALE);

	strncpy(szName, aTagName, ARRAY_LEN(szName));

	CString str = buteMgr.GetString(aTagName, WMGR_WEAPON_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szIcon, str, ARRAY_LEN(szIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_PICKUP_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szPickupIcon, str, ARRAY_LEN(szPickupIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_PICKUP_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szPickupName, str, ARRAY_LEN(szPickupName));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_PVMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szPVModel, str, ARRAY_LEN(szPVModel));
	}

	for(i=0 ; i < MAX_MODEL_TEXTURES ; i++)
	{
		char aTemp[60];
		//get player view skin file
		sprintf(aTemp, "%s%d", WMGR_WEAPON_PVSKIN, i);
		str = buteMgr.GetString(aTagName, aTemp);
		if (!str.IsEmpty())
		{
			strncpy(szPVSkins[i], str, ARRAY_LEN(szPVSkins[i]));
		}
		//get hand held view file
		sprintf(aTemp, "%s%d", WMGR_WEAPON_HHSKIN, i);
		str = buteMgr.GetString(aTagName, aTemp);
		if (!str.IsEmpty())
		{
			strncpy(szHHSkins[i], str, ARRAY_LEN(szHHSkins[i]));
		}
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_HHMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szHHModel, str, ARRAY_LEN(szHHModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_SELECTSND);
	if (!str.IsEmpty())
	{
		strncpy(szSelectSound, str, ARRAY_LEN(szSelectSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_DESELECTSND);
	if (!str.IsEmpty())
	{
		strncpy(szDeselectSound, str, ARRAY_LEN(szDeselectSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_CROSSHAIR_IMAGE);
	if (!str.IsEmpty())
	{
		strncpy(szCrosshair, str, ARRAY_LEN(szCrosshair));
	}

	switch(buteMgr.GetInt(aTagName, WMGR_WEAPON_ALPHA_FLAG))
	{
		case 0:		dwAlphaFlag = 0;						break;
		case 1:		dwAlphaFlag = FLAG_ENVIRONMENTMAP;		break;
		default:	dwAlphaFlag = 0;						break;
	}

	// Build our pv fx id array...

	nNumPVFXTypes = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPON_PVFXNAME, nNumPVFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPVFXTypes < WMGR_MAX_PVFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			PVFX* pPVFX = g_pFXButeMgr->GetPVFX(str.GetBuffer(0));
			if (pPVFX)
			{
				aPVFXTypes[nNumPVFXTypes] = pPVFX->nId;
			}
		}

		nNumPVFXTypes++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_PVFXNAME, nNumPVFXTypes);
	}


	// Build the array of barrels
	nDefaultBarrelType = 0;

	for( i=0 ; i<NUM_BARRELS ; i++)
	{
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_BARREL, i);

		if(buteMgr.Exist(aTagName, s_aAttName))
		{
			str = buteMgr.GetString(aTagName, s_aAttName);
			if (!str.IsEmpty())
			{
				BARREL *pBarrel = g_pWeaponMgr->GetBarrel( str );
				if (pBarrel)
				{
					aBarrelTypes[i] = pBarrel->nId;

					// Set the default to the first barrel type found
					if(i==0)
						nDefaultBarrelType = pBarrel->nId;
				}
			}
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Cache
//
//	PURPOSE:	Cache all the resources associated with the weapon
//
// ----------------------------------------------------------------------- //

void WEAPON::Cache(CWeaponMgr* pWeaponMgr, CString* szAppend)
{
	int i = 0;

#ifndef _CLIENTBUILD

	if (!pWeaponMgr) return;

	if (szPVModel[0])
	{
		g_pServerDE->CacheFile(FT_MODEL, szPVModel);
	}

	for(i=0 ; i < MAX_MODEL_TEXTURES ; i++)
	{
		if (szPVSkins[i][0])
		{
			if(szAppend && !szAppend->IsEmpty())
			{
				char szSkins[WMGR_MAX_FILE_PATH];
				sprintf(szSkins, szPVSkins[i], szAppend->GetBuffer());
				g_pServerDE->CacheFile(FT_TEXTURE, szSkins);
			}
			else
			{
				g_pServerDE->CacheFile(FT_TEXTURE, szPVSkins[i]);
			}
		}
	}

	if (szHHModel[0])
	{
		g_pServerDE->CacheFile(FT_MODEL, szHHModel);
	}

	if (szHHSkins[0][0])
	{
		g_pServerDE->CacheFile(FT_TEXTURE, szHHSkins[0]);
	}

	// Cache sounds...

	if (szSelectSound[0])
	{
		g_pServerDE->CacheFile(FT_SOUND, szSelectSound);
	}

	if (szDeselectSound[0])
	{
		g_pServerDE->CacheFile(FT_SOUND, szDeselectSound);
	}

	for (i=0; i < nNumPVFXTypes; i++)
	{
		PVFX* pPVFX = g_pFXButeMgr->GetPVFX(aPVFXTypes[i]);
		if (pPVFX)
		{
			pPVFX->Cache(g_pFXButeMgr);
		}
	}

	// See if this is a projectile weapon...
	for(i=0; i<NUM_BARRELS; i++)
	{
		BARREL *pBarrel = pWeaponMgr->GetBarrel(aBarrelTypes[i]);

		// Save out the barrel...
		BARREL *pPrimBarrel = pBarrel;

		const int nMaxIterations = 100;
		int nIterations = 0;
		
		do
		{
			++nIterations;

			if(pBarrel)
			{
				pBarrel->Cache(pWeaponMgr);

				// Move on to next barrel
				pBarrel = pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);
			}
		}while(pBarrel && pBarrel != pPrimBarrel && nIterations < nMaxIterations);

		if( nIterations >= nMaxIterations )
		{
			g_pInterface->CPrint("Barrel %s has recursion error, one of the next barrels refers to a barrel which is not the first barrel.",
				pBarrel ? pBarrel->szName : "<unknown>" );
		}

	}

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Save()
//
//	PURPOSE:	Save any necessary data to the bute file...
//
// ----------------------------------------------------------------------- //

void WEAPON::Save(CButeMgr & buteMgr)
{
	CAVector vAVec(vPos.x, vPos.y, vPos.z);
	buteMgr.SetVector(szName, WMGR_WEAPON_POS, vAVec);
}
/////////////////////////////////////////////////////////////////////////////
//
//	W E A P O N _ S E T  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON_SET::WEAPON_SET
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WEAPON_SET::WEAPON_SET()
{
	nId			= 0;
	szName[0]	= '\0';
	nNumWeapons = 0;

	for(int i=0 ; i<WMGR_MAX_WEAPONS_IN_SET ; i++)
	{
		szWeaponName[i][0] = '\0';
		bDefaultWeapon[i] = LTFALSE;
		bCanPickup[i] = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON_SET::Init
//
//	PURPOSE:	Build the weapon set struct
//
// ----------------------------------------------------------------------- //

DBOOL WEAPON_SET::Init(CHierarchicalButeMgr & buteMgr, const char* aTagName)
{
	CString str;

	strncpy(szName, aTagName, ARRAY_LEN(szName));

	for(int i=0 ; i<WMGR_MAX_WEAPONS_IN_SET ; i++)
	{
		char aTemp[60];

		sprintf(aTemp,"%s%d", WMGR_WEAPON_SET_ITEM, i);

		if (buteMgr.Exist(aTagName, aTemp))
		{
			str = buteMgr.GetString(aTagName, aTemp);

			if (!str.IsEmpty())
			{
				strncpy(szWeaponName[i], str, ARRAY_LEN(szWeaponName[i]));
				sprintf(aTemp,"%s%d", WMGR_WEAPON_SET_DEFAULT, i);
				bDefaultWeapon[i] = (LTBOOL) buteMgr.GetInt(aTagName, aTemp);

				if (buteMgr.Exist(aTagName, aTemp))
				{
					sprintf(aTemp,"%s%d", WMGR_WEAPON_SET_CANPICKUP, i);
					bCanPickup[i] = (LTBOOL) buteMgr.GetInt(aTagName, aTemp);
				}
				else
				{
					bCanPickup[i] = LTTRUE;
				}
			}
			else
			{
				szWeaponName[i][0] = '\0';
				bDefaultWeapon[i] = LTFALSE;
				bCanPickup[i] = LTFALSE;
			}

			nNumWeapons++;
		}
	}

	return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	B A R R E L  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BARREL::BARREL
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
BARREL::BARREL()
{
	nId				= WMGR_INVALID_ID;
	nNameId			= 0;
	szName[0]		= '\0';
	vMuzzlePos.Init();
	fHHBreachOffset	= 0.0f;
	fPVBreachOffset	= 0.0f;
	szAltFireSound[0]	= '\0';
	szFireSound[0]		= '\0';
	szDryFireSound[0]	= '\0';
	szNextBarrel[0]		= '\0';
	szPVAltSkin[0]		= '\0';
	nNumFlashSockets	= 0;
	{for (int i=0; i < WMGR_MAX_FLASH_SOCKETS; i++)
	{
		szFlashSocket[i][0] = '\0';
	}}
	{for (int i=0; i < WMGR_MAX_MISC_SNDS; i++)
	{
		szMiscSounds[i][0] = '\0';
	}}
	szClipIcon[0]		= '\0';
	fFireSoundRadius	= 0.0f;
	bInfiniteAmmo		= LTFALSE;
	bDecloak			= LTFALSE;
	bSquelchMultipleImpacts	= LTFALSE;
	bSave			= LTFALSE;
	bDrawChooserBars= LTFALSE;

	nShotsPerClip		= 0;

	nAmmoType			= WMGR_INVALID_ID;

	vMinRecoilAmount.Init();
	vMaxRecoilAmount.Init();
	vRecoilTimes.Init();

	nMinXPerturb		= 0;
	nMaxXPerturb		= 0;
	nMinYPerturb		= 0;
	nMaxYPerturb		= 0;
	nRange				= 0;
	nRangeSqr			= 0;
	nVectorsPerRound	= 0;

	pPVMuzzleFX			= LTNULL;
	pHHMuzzleFX			= LTNULL;
	eBarrelType		= BT_INVALID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BARREL::Init
//
//	PURPOSE:	Build the barrel struct
//
// ----------------------------------------------------------------------- //

DBOOL BARREL::Init(CHierarchicalButeMgr & buteMgr, const char* aTagName)
{
	nNameId					= buteMgr.GetInt(aTagName, WMGR_BARREL_NAMEID);

	vMuzzlePos				= buteMgr.GetVector(aTagName, WMGR_BARREL_MUZZLEPOS);
	vMinRecoilAmount		= buteMgr.GetVector(aTagName, WMGR_BARREL_MIN_RECOIL_AMOUNT);
	vMaxRecoilAmount		= buteMgr.GetVector(aTagName, WMGR_BARREL_MAX_RECOIL_AMOUNT);
	vRecoilTimes			= buteMgr.GetVector(aTagName, WMGR_BARREL_RECOIL_TIMES);
	eBarrelType				= (BarrelType)buteMgr.GetInt(aTagName, WMGR_BARREL_TYPE);

	strncpy(szName, aTagName, ARRAY_LEN(szName));

	CString str = buteMgr.GetString(aTagName, WMGR_BARREL_ALT_SKIN);
	if (!str.IsEmpty())
	{
		strncpy(szPVAltSkin, str, ARRAY_LEN(szPVAltSkin));
	}


	nNumFlashSockets = 1;
	strncpy(szFlashSocket[0], "Flash", ARRAY_LEN(szFlashSocket[0])); // Set default to "Flash"
	{for (int i=0; i < WMGR_MAX_MISC_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", WMGR_BARREL_FLASH_SOCKET, i);

		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			strncpy(szFlashSocket[i], str, ARRAY_LEN(szFlashSocket[i]));
			if( i != 0 ) ++nNumFlashSockets;
		}
	}}

	str = buteMgr.GetString(aTagName, WMGR_BARREL_ALTFIRESND);
	if (!str.IsEmpty())
	{
		strncpy(szAltFireSound, str, ARRAY_LEN(szAltFireSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_BARREL_FIRESND);
	if (!str.IsEmpty())
	{
		strncpy(szFireSound, str, ARRAY_LEN(szFireSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_BARREL_DRYFIRESND);
	if (!str.IsEmpty())
	{
		strncpy(szDryFireSound, str, ARRAY_LEN(szDryFireSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_BARREL_NEXT_BARREL);
	if (!str.IsEmpty())
	{
		strncpy(szNextBarrel, str, ARRAY_LEN(szNextBarrel));
	}

	{for (int i=0; i < WMGR_MAX_MISC_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", WMGR_BARREL_MISCSND, i);

		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			strncpy(szMiscSounds[i], str, WMGR_MAX_FILE_PATH);
		}
	}}

	str = buteMgr.GetString(aTagName, WMGR_BARREL_CLIPICON);
	if (!str.IsEmpty())
	{
		strncpy(szClipIcon, str, ARRAY_LEN(szClipIcon));
	}

	fHHBreachOffset		= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_BARREL_HHBREACH);
	fPVBreachOffset		= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_BARREL_PVBREACH);
	fFireSoundRadius	= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_BARREL_FIRESNDRADIUS);
	bInfiniteAmmo		= (DBOOL) buteMgr.GetInt(aTagName, WMGR_BARREL_INFINITEAMMO);
	nShotsPerClip		= (DBOOL) buteMgr.GetInt(aTagName, WMGR_BARREL_SHOTSPERCLIP);
	bDecloak			= (DBOOL) buteMgr.GetInt(aTagName, WMGR_BARREL_DECLOAK);
	bSquelchMultipleImpacts	= (DBOOL) buteMgr.GetInt(aTagName, WMGR_BARREL_NO_MULTI_IMPACT_SOUND);
	bSave				= (DBOOL) buteMgr.GetInt(aTagName, WMGR_BARREL_SAVE);
	bDrawChooserBars	= (DBOOL) buteMgr.GetInt(aTagName, WMGR_BARREL_DRAWAMMO);

	str = buteMgr.GetString(aTagName, WMGR_BARREL_AMMONAME);
	if (!str.IsEmpty())
	{
		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(str);
		if (pAmmo)
		{
			nAmmoType = pAmmo->nId;
		}
	}

	nMinXPerturb		= buteMgr.GetInt(aTagName, WMGR_BARREL_MINXPERTURB);
	nMaxXPerturb		= buteMgr.GetInt(aTagName, WMGR_BARREL_MAXXPERTURB);
	nMinYPerturb		= buteMgr.GetInt(aTagName, WMGR_BARREL_MINYPERTURB);
	nMaxYPerturb		= buteMgr.GetInt(aTagName, WMGR_BARREL_MAXYPERTURB);
	nRange				= buteMgr.GetInt(aTagName, WMGR_BARREL_RANGE);
	nRangeSqr			= nRange * nRange;
	nVectorsPerRound	= buteMgr.GetInt(aTagName, WMGR_BARREL_VECTORSPERROUND);
	
	str = buteMgr.GetString(aTagName, WMGR_BARREL_HHMUZZLEFXNAME);
	if (!str.IsEmpty())
	{
		pHHMuzzleFX = g_pFXButeMgr->GetMuzzleFX(str.GetBuffer(0));
	}

	str = buteMgr.GetString(aTagName, WMGR_BARREL_PVMUZZLEFXNAME);
	if (!str.IsEmpty())
	{
		pPVMuzzleFX = g_pFXButeMgr->GetMuzzleFX(str.GetBuffer(0));
	}
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BARREL::Cache
//
//	PURPOSE:	Cache all the resources associated with the barrel
//
// ----------------------------------------------------------------------- //

void BARREL::Cache(CWeaponMgr* pWeaponMgr)
{
#ifndef _CLIENTBUILD

	if (!pWeaponMgr) return;

	// Cache sounds...

	if (szPVAltSkin[0])
	{
		g_pServerDE->CacheFile(FT_TEXTURE, szPVAltSkin);
	}


	if (szFireSound[0])
	{
		g_pServerDE->CacheFile(FT_SOUND, szFireSound);
	}

	if (szDryFireSound[0])
	{
		g_pServerDE->CacheFile(FT_SOUND, szDryFireSound);
	}

	for (int i=0; i < WMGR_MAX_MISC_SNDS; i++)
	{
		if (szMiscSounds[i][0])
		{
			g_pServerDE->CacheFile(FT_SOUND, szMiscSounds[i]);
		}
	}

	// Load all the ammo related resources...

	AMMO* pAmmo = pWeaponMgr->GetAmmo(nAmmoType);
	if (pAmmo)
	{
		pAmmo->Cache(pWeaponMgr);
	}

	// Load the player-view muzzle fx...

	if (pPVMuzzleFX)
	{
		pPVMuzzleFX->Cache(g_pFXButeMgr);
	}

	// Load the hand-held muzzle fx...

	if (pHHMuzzleFX)
	{
		pHHMuzzleFX->Cache(g_pFXButeMgr);
	}

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BARREL::Save()
//
//	PURPOSE:	Save any necessary data to the bute file...
//
// ----------------------------------------------------------------------- //

void BARREL::Save(CButeMgr & buteMgr)
{

	CAVector vAVec(vMuzzlePos.x, vMuzzlePos.y, vMuzzlePos.z);
	buteMgr.SetVector(szName, WMGR_BARREL_MUZZLEPOS, vAVec);

	buteMgr.SetDouble(szName, WMGR_BARREL_HHBREACH, (double)fHHBreachOffset);
	buteMgr.SetDouble(szName, WMGR_BARREL_PVBREACH, (double)fPVBreachOffset);
}


/////////////////////////////////////////////////////////////////////////////
//
//	A M M O _ P O O L  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO_POOL::AMMO_POOL
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
AMMO_POOL::AMMO_POOL()
{
	nId			= WMGR_INVALID_ID;

	bSave		= LTFALSE;
	nNameId		= 0;

	szCounterIcon[0]= '\0';
	szPickupIcon[0]	= '\0';
	szName[0]		= '\0';

	nMaxAmount		= 0;
	nSpawnedAmount	= 0;
	nDefaultAmount	= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO_POOL::Init
//
//	PURPOSE:	Build the ammo struct
//
// ----------------------------------------------------------------------- //

DBOOL AMMO_POOL::Init(CHierarchicalButeMgr & buteMgr, const char* aTagName)
{
	if (!aTagName) return LTFALSE;

	bSave				= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_AMMOPOOL_SAVE);
	nNameId				= buteMgr.GetInt(aTagName, WMGR_AMMOPOOL_NAMEID);
	nMaxAmount			= buteMgr.GetInt(aTagName, WMGR_AMMOPOOL_MAXAMOUNT);
	nSpawnedAmount		= buteMgr.GetInt(aTagName, WMGR_AMMOPOOL_SPAWNEDAMOUNT);
	nDefaultAmount		= buteMgr.GetInt(aTagName, WMGR_AMMOPOOL_DEFAULTAMOUNT);

	strncpy(szName, aTagName, ARRAY_LEN(szName));

	CString str = buteMgr.GetString(aTagName, WMGR_AMMOPOOL_PUICON);
	if (!str.IsEmpty())
	{
		strncpy(szPickupIcon, str, ARRAY_LEN(szPickupIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_AMMOPOOL_COUNTICON);
	if (!str.IsEmpty())
	{
		strncpy(szCounterIcon, str, ARRAY_LEN(szCounterIcon));
	}

	return LTTRUE;
}
		
/////////////////////////////////////////////////////////////////////////////
//
//	A M M O  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::AMMO
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AMMO::AMMO()
{
	nId			= WMGR_INVALID_ID;

	nNameId		= 0;
	eType		= UNKNOWN_AMMO_TYPE;

	szName[0]		= '\0';
	szAmmoPool[0]	= '\0';

	nInstDamage		= 0;
	eInstDamageType	= DT_INVALID;

	nAreaDamage			= 0;
	nAreaDamageRadius	= 0;
	eAreaDamageType		= DT_INVALID;
	fAreaDamageDuration	= 0.0f;
	
	fProgDamage			= 0.0f;
	fProgDamageDuration	= 0.0f;
	fShellCasingDelay	= 0.0f;
	eProgDamageType		= DT_INVALID;

	pProjectileFX	= NULL;
	pImpactFX		= NULL;
	pUWImpactFX		= NULL;
	pFireFX			= NULL;
	pTracerFX		= NULL;

	nAmmoPerShot	= 0;
	bBubbleTrail	= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::Init
//
//	PURPOSE:	Build the ammo struct
//
// ----------------------------------------------------------------------- //

DBOOL AMMO::Init(CHierarchicalButeMgr & buteMgr, const char* aTagName)
{
	if (!aTagName) return LTFALSE;

	nNameId				= buteMgr.GetInt(aTagName, WMGR_AMMO_NAMEID);
	eType				= (AmmoType) buteMgr.GetInt(aTagName, WMGR_AMMO_TYPE);

	nInstDamage			= buteMgr.GetInt(aTagName, WMGR_AMMO_INSTDAMAGE);
	eInstDamageType		= (DamageType) buteMgr.GetInt(aTagName, WMGR_AMMO_INSTDAMAGETYPE);

	nAreaDamage			= buteMgr.GetInt(aTagName, WMGR_AMMO_AREADAMAGE);
	eAreaDamageType		= (DamageType) buteMgr.GetInt(aTagName, WMGR_AMMO_AREADAMAGETYPE);
	nAreaDamageRadius	= buteMgr.GetInt(aTagName, WMGR_AMMO_AREADAMAGERADIUS);
	fAreaDamageDuration	= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_AREADAMAGEDUR);
	
	fProgDamage			= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_PROGDAMAGE);
	eProgDamageType		= (DamageType) buteMgr.GetInt(aTagName, WMGR_AMMO_PROGDAMAGETYPE);
	fProgDamageDuration	= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_PROGDAMAGEDUR);
	nAmmoPerShot		= buteMgr.GetInt(aTagName, WMGR_AMMO_AMOUNT_PER_SHOT);
	bBubbleTrail		= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_AMMO_BUBBLE_TRAIL);
	fShellCasingDelay	= (DFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_CASING_DELAY);

	strncpy(szName, aTagName, ARRAY_LEN(szName));

	CString str = buteMgr.GetString(aTagName, WMGR_AMMO_POOLNAME);
	if (!str.IsEmpty())
	{
		strncpy(szAmmoPool, str, ARRAY_LEN(szName));
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_IMPACTFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_IMPACTFX);
		if (!str.IsEmpty())
		{
			pImpactFX = g_pFXButeMgr->GetImpactFX(str.GetBuffer(0));
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_UWIMPACTFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_UWIMPACTFX);
		if (!str.IsEmpty())
		{
			pUWImpactFX = g_pFXButeMgr->GetImpactFX(str.GetBuffer(0));
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_PROJECTILEFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_PROJECTILEFX);
		if (!str.IsEmpty())
		{
			pProjectileFX = g_pFXButeMgr->GetProjectileFX(str.GetBuffer(0));
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_FIREFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_FIREFX);
		if (!str.IsEmpty())
		{
			pFireFX = g_pFXButeMgr->GetFireFX(str.GetBuffer(0));
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_TRACERFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_TRACERFX);
		if (!str.IsEmpty())
		{
			pTracerFX = g_pFXButeMgr->GetTracerFX(str.GetBuffer(0));
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::Cache
//
//	PURPOSE:	Cache all the resources associated with the ammo
//
// ----------------------------------------------------------------------- //

void AMMO::Cache(CWeaponMgr* pWeaponMgr)
{
#ifndef _CLIENTBUILD

	if (!pWeaponMgr) return;

	if (pImpactFX)
	{
		pImpactFX->Cache(g_pFXButeMgr);
	}

	if (pProjectileFX)
	{
		pProjectileFX->Cache(g_pFXButeMgr);
	}

	if (pFireFX)
	{
		pFireFX->Cache(g_pFXButeMgr);
	}

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::GetMaxAmount()
//
//	PURPOSE:	Calculate max amount of ammo of this type allowed for 
//				the character
//
// ----------------------------------------------------------------------- //

int AMMO::GetMaxPoolAmount(HOBJECT hCharacter)
{
	AMMO_POOL* pPool = LTNULL;
	
	if(g_pWeaponMgr)
		pPool = g_pWeaponMgr->GetAmmoPool(szAmmoPool);

	if(pPool)
		return pPool->GetMaxAmount(hCharacter);

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO_POOL::GetMaxAmount()
//
//	PURPOSE:	Calculate max amount of ammo of this type allowed for 
//				the character
//
// ----------------------------------------------------------------------- //

int AMMO_POOL::GetMaxAmount(HOBJECT hCharacter)
{
	int nMaxAmmo = nMaxAmount;
#ifdef _CLIENTBUILD
	if (!g_pGameClientShell->IsMultiplayerGame())
	{
//		CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();
		DFLOAT fMult = 1.0f;//pPSummary->m_PlayerRank.fAmmoMultiplier;
		nMaxAmmo =  (int) ( fMult * (DFLOAT)nMaxAmmo );
	}
#else
	if (IsPlayer(hCharacter) && g_pGameServerShell->GetGameType().IsSinglePlayer())
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pServerDE->HandleToObject(hCharacter);
		DFLOAT fMult = 1.0f;//pPlayer->GetPlayerSummaryMgr()->m_PlayerRank.fAmmoMultiplier;
		nMaxAmmo =  (int) ( fMult * (DFLOAT)nMaxAmmo );
	};
#endif
	return nMaxAmmo;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	C A C H E  Functions...(SERVER-SIDE ONLY)
//
//  These functions are used to cache the resources associated with weapons.
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CacheAll()
//
//	PURPOSE:	Cache all the weapon related resources...
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::CacheAll()
{
#ifndef _CLIENTBUILD // No caching on the client...

	// Cache all the weapons...

	for( WEAPON* pCurWeapon  = m_pWeapons; pCurWeapon < m_pWeapons + m_nNumWeapons; ++pCurWeapon )
	{
		if (pCurWeapon)
		{
			pCurWeapon->Cache(this);
		}
	}

#endif
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeaponId
//
//	PURPOSE:	Gets a weapon ID from a command value
//
// ----------------------------------------------------------------------- //


int CWeaponMgr::GetWeaponId(int nCommandId)
{
#ifdef _CLIENTBUILD
	int nId = nCommandId - COMMAND_ID_WEAPON_BASE;
	if (nId < 0) 
	{
		return WMGR_INVALID_ID;
	}

	WEAPON* pWep = LTNULL;

	pWep = GetWeapon(m_pPlayerStats->GetWeaponFromSet(nId));

	if(pWep)
	{
		return pWep->nId;
	}
	else
#endif
		return WMGR_INVALID_ID;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWepSetIndex
//
//	PURPOSE:	Gets the index number of a weapon from a set
//
// ----------------------------------------------------------------------- //

int	CWeaponMgr::GetWepSetIndex(const char* pWeaponName, int nWepSet)
{
	// Get the weapon set...

	WEAPON_SET* pSet = GetWeaponSet(nWepSet);

	if(pSet)
	{
		for( int i=0 ; i<pSet->nNumWeapons ; i++)
		{
			if(_stricmp(pSet->szWeaponName[i], pWeaponName) == 0)
				return i;
		}
	}

	return WMGR_INVALID_ID;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::IsWeaponInSet
//
//	PURPOSE:	Tells whether a weapon is contained in a particular set
//
// ----------------------------------------------------------------------- //

uint8 CWeaponMgr::IsWeaponInSet(int nWeaponSet, const char* pWeaponName)
{
	WEAPON_SET *pWeaponSet = GetWeaponSet(nWeaponSet);
	if(!pWeaponSet || !pWeaponName) return LTFALSE;

	for(int i = 0; i < pWeaponSet->nNumWeapons; i++)
	{
		if(!_stricmp(pWeaponName, pWeaponSet->szWeaponName[i]))
		{
			if(pWeaponSet->bCanPickup[i])
				return WMGR_AVAILABLE;
			else
				return WMGR_CANT_USE;
		}
	}

	return WMGR_NOT_IN_SET;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::IsAmmoInSet
//
//	PURPOSE:	Tells whether an ammo is part of a particular weapon set
//
// ----------------------------------------------------------------------- //

uint8 CWeaponMgr::IsAmmoInSet(int nWeaponSet, const char* pAmmoPoolName)
{
	WEAPON_SET *pWeaponSet = GetWeaponSet(nWeaponSet);
	if(!pWeaponSet || !pAmmoPoolName) return LTFALSE;

	for(int i = 0; i < pWeaponSet->nNumWeapons; i++)
	{
		WEAPON *pWeapon = GetWeapon(pWeaponSet->szWeaponName[i]);

		if(pWeapon)
		{
			for(int j = 0; j < NUM_BARRELS; j++)
			{
				BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[j]);
				BARREL* pBaseBarrel = pBarrel;

				if(pBarrel)
				{
					const int nMaxIterations = 100;
					int nIterations = 0;

					do
					{
						++nIterations;

						AMMO *pAmmo = g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);

						if(pAmmo)
						{
							if(!_stricmp(pAmmoPoolName, pAmmo->szAmmoPool))
							{
								if(pWeaponSet->bCanPickup[i])
									return WMGR_AVAILABLE;
								else
									return WMGR_CANT_USE;
							}
						}

						pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);
					}
					while(pBarrel && pBarrel != pBaseBarrel && nIterations < nMaxIterations);
				}
			}
		}
	}

	return WMGR_NOT_IN_SET;
}



#ifndef _CLIENTBUILD  // Server-side only


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::ReadWeapon
//
//	PURPOSE:	Read in the weapon properties
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::ReadWeapon(const char* szWeaponName, DBYTE & nWeaponId, DBYTE & nBarrelId, DBYTE & nAmmoId, uint8 &nAmmoPoolId)
{
	// Get the weapon name...

	WEAPON* pWeapon = GetWeapon(szWeaponName);
	if (pWeapon)
	{
		nWeaponId = pWeapon->nId;
	
		BARREL* pBarrel = GetBarrel(pWeapon->aBarrelTypes[0]);
		if(pBarrel)
		{
			nBarrelId = pBarrel->nId;

			AMMO* pAmmo = GetAmmo(pBarrel->nAmmoType);
			if (pAmmo)
			{
				AMMO_POOL* pPool = GetAmmoPool(pAmmo->szAmmoPool);
				nAmmoId		= pAmmo->nId;
				nAmmoPoolId = pPool->nId;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::ReadBarrel
//
//	PURPOSE:	Read in the weapon properties
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::ReadBarrel(const char* szBarrelName, uint8 & nBarrelId, uint8 & nAmmoId, uint8 &nAmmoPoolId)
{
	// Given a barrel name, get its id, ammo, and ammo pool.

	// Find the barrel.
	BARREL* pBarrel = GetBarrel(szBarrelName);
	if (pBarrel)
	{
		nBarrelId = pBarrel->nId;

		// Get the barrel's ammo data.
		AMMO* pAmmo = GetAmmo(pBarrel->nAmmoType);
		if (pAmmo)
		{
			AMMO_POOL* pPool = GetAmmoPool(pAmmo->szAmmoPool);
			nAmmoId		= pAmmo->nId;
			nAmmoPoolId = pPool->nId;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::FindWeaponWithBarrel
//
//	PURPOSE:	Finds the first weapon id which holds the specified barrel id.
//
// ----------------------------------------------------------------------- //

uint8 CWeaponMgr::FindWeaponWithBarrel(uint8 nBarrelId)
{
	if( nBarrelId == WMGR_INVALID_ID )
		return WMGR_INVALID_ID;

	// Iterate through all the weapons.
	for( WEAPON * pWeapon = m_pWeapons; pWeapon < m_pWeapons + m_nNumWeapons; ++pWeapon )
	{
		// Iterate through all the possible barrels of the weapon.
		for(int j = 0; j < NUM_BARRELS; ++j )
		{
			if( pWeapon->aBarrelTypes[j] == nBarrelId )
			{
				// We found it!
				return pWeapon->nId;
			}
			
			// If the barrel is a switching type barrel, we need
			// to walk through its list of barrels.
			BARREL * pStartBarrel = GetBarrel( pWeapon->aBarrelTypes[j] );
			BARREL * pBarrel = pStartBarrel;
			int num_iterations = 0;
			const int max_iterations = 100;
			while( pBarrel && ++num_iterations < max_iterations)
			{ 
				if( pBarrel->nId == nBarrelId )
				{
					return pWeapon->nId;
				}

				if( strlen(pBarrel->szNextBarrel) > 0 )
				{
					pBarrel = GetBarrel( pBarrel->szNextBarrel );
				}
				else
				{
					pBarrel = LTNULL;
				}

				// Don't go forever in a cyclic linked list!!!
				if( pBarrel == pStartBarrel )
					pBarrel = LTNULL;
			}
			
			_ASSERT( num_iterations < max_iterations);
		}
	}

	return WMGR_INVALID_ID;
}

////////////////////////////////////////////////////////////////////////////
//
// CWeaponMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

DRESULT	CWeaponMgrPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if (!sm_bInitted)
	{
		char szFile[256];

		// Create global fx bute mgr if necessary before the weapon
		// bute mgr is created...

		if (!g_pDebrisMgr)
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, DEBRISMGR_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, DEBRISMGR_DEFAULT_FILE);
#endif
			sm_DebrisMgr.SetInRezFile(LTFALSE);
			sm_DebrisMgr.Init(g_pLTServer, szFile);
		}

		if (!g_pFXButeMgr)
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, FXBMGR_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, FXBMGR_DEFAULT_FILE);
#endif
			sm_FXButeMgr.SetInRezFile(LTFALSE);
			sm_FXButeMgr.Init(g_pLTServer, szFile);
		}

#ifdef _WIN32
		sprintf(szFile, "%s\\%s", szRezPath, WEAPON_DEFAULT_FILE);
#else
		sprintf(szFile, "%s/%s", szRezPath, WEAPON_DEFAULT_FILE);
#endif
		sm_ButeMgr.SetInRezFile(LTFALSE);
		sm_ButeMgr.Init(g_pLTServer, szFile);
		sm_bInitted = LTTRUE;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgrPlugin::PopulateStringListWeapons
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

void CWeaponMgrPlugin::PopulateStringList(char* const * aszStrings, uint32* pcStrings, 
	const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return;

	// Add an entry for each weapon...

	uint32 dwNumWeapons = sm_ButeMgr.GetNumWeapons();

	WEAPON* pWeapon = LTNULL;

	for (uint32 i=0; i < dwNumWeapons; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pWeapon = sm_ButeMgr.GetWeapon(i);
		uint32 dwWeaponNameLen = strlen(pWeapon->szName);

		if (pWeapon && pWeapon->szName[0] && 
			dwWeaponNameLen < cMaxStringLength &&
			(*pcStrings) + 1 < cMaxStrings)
		{
			// Just use the weapon name, the default ammo will be used...

			strcpy(*aszStrings, pWeapon->szName);
			++aszStrings;
			++(*pcStrings);
		}
	}
}

#endif // _CLIENTBUILD
