// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponMgr.h
//
// PURPOSE : WeaponMgr definition - Controls attributes of all weapons
//
// CREATED : 12/02/98
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_MGR_H__
#define __WEAPON_MGR_H__

#include "HierarchicalButeMgr.h"
#include "DamageTypes.h"
#include "CommandIDs.h"


struct PROJECTILEFX;
struct IMPACTFX;
struct IMPACTFX;
struct FIREFX;
struct TRACERFX;
struct CMuzzleFX;
class CPlayerStats;

class CWeaponMgr;
extern CWeaponMgr* g_pWeaponMgr;

const LTFLOAT	WEAPON_SOUND_RADIUS		= 2000.0;

const LTFLOAT	MARINE_MAX_BATTERY_LEVEL= 255.0;

const char* const WEAPON_KEY_FIRE				= "FIRE_KEY";
const char* const WEAPON_KEY_KICK_BACK			= "KICK_BACK_KEY";
const char* const WEAPON_KEY_SOUND				= "SOUND_KEY";
const char* const WEAPON_KEY_BUTE_SOUND			= "BUTE_SOUND_KEY";
const char* const WEAPON_KEY_LOOPSOUND			= "LOOPSOUND_KEY";
const char* const WEAPON_KEY_FX					= "FX_KEY";
const char* const WEAPON_KEY_CAMERAFX			= "CAMERAFX_KEY";
const char* const WEAPON_KEY_CHARGE				= "CHARGE_KEY";
const char* const WEAPON_KEY_PRE_CHARGE			= "PRE_CHARGE_KEY";

#ifdef _WIN32
	const char* const WEAPON_DEFAULT_FILE			= "Attributes\\Weapons.txt";
#else
	const char* const WEAPON_DEFAULT_FILE			= "Attributes/Weapons.txt";
#endif

const char* const WEAPON_KEY_SELECT_COMPLETE	= "SELECT_COMPLETE";
const char* const WEAPON_KEY_END_TAIL			= "END_TAIL_KEY";
const char* const WEAPON_KEY_AMMO_CHANGE		= "ammo_change_key";
const char* const WEAPON_KEY_RELOAD				= "reload_key";
const char* const WEAPON_KEY_FORCE_NEXT_WEP		= "force_next_weapon_key";
const char* const WEAPON_KEY_ENERGY_SIFT		= "energy_sift_key";
const char* const WEAPON_KEY_ZOOM_START			= "zoom_start_key";
const char* const WEAPON_KEY_ZOOM_END			= "zoom_end_key";
const char* const WEAPON_KEY_UPDATE_HIDESTATUS	= "update_hidestatus";
const char* const WEAPON_KEY_MEDICOMP			= "medicomp_key";
const char* const WEAPON_KEY_FORCE_LAST_WEP		= "force_last_weapon_key";
const char* const WEAPON_KEY_DETONATE			= "detonate_key";
const char* const WEAPON_KEY_FORCE_DEFAULT_WEP	= "force_default_weapon_key";

const uint32	WMGR_INVALID_ID			= 255;

const uint32	WMGR_MAX_NAME_LENGTH	= 64;
const uint32	WMGR_MAX_FILE_PATH		= 64;
const uint32	WMGR_MAX_AMMO_TYPES		= 10;
const uint32	WMGR_MAX_MOD_TYPES		= 3;
const uint32	WMGR_MAX_MISC_SNDS		= 3;
const uint32	WMGR_MAX_PVFX_TYPES		= 5;
const uint8		WMGR_MAX_FLASH_SOCKETS  = 3;  // This must be less than 256!
const uint32	WMGR_MAX_WEAPONS_IN_SET	= 20;

const uint8		WMGR_NOT_IN_SET			= 0;
const uint8		WMGR_CANT_USE			= 1;
const uint8		WMGR_AVAILABLE			= 2;


enum WeaponState 
{ 
	W_IDLE, 
	W_BEGIN_FIRING, 
	W_FIRING, 
	W_CHARGE_FIRING, 
	W_FIRED,  
	W_END_FIRING, 
	W_RELOADING, 
	W_FIRING_NOAMMO, 
	W_SELECT, 
	W_DESELECT,
	W_BARREL_SWITCH,
	W_SPINUP,
	W_CHANGE_AMMO,
	W_ZOOM_STARTING,
	W_ZOOM_ENDING,
	W_SPINNING,
	W_SPINDOWN,	
	W_START_FIRING,
	W_FIRING_SPECIAL,
	W_FIRING_DETONATE,
	W_CHARGING,
};

enum AmmoType 
{ 
	VECTOR=0, 
	PROJECTILE, 
	CANNON,
	GADGET,
	ALIEN_POUNCE,
	FACE_HUG,
	UNKNOWN_AMMO_TYPE 
};

enum BarrelType
{
	BT_INVALID = -1,
	BT_NORMAL,
	BT_CHARGE,
	BT_SWITCHING,
	BT_ALIEN_SPECIAL,
	BT_TARGETING_TOGGLE,
	BT_MINIGUN_SPECIAL,
	BT_PRE_CHARGE,
	BT_ZOOMING,
	BT_EXO_LAZER,
	BT_PRED_DETONATOR,
	BT_PRED_STICKYBOMB,
};

enum
{
	PRIMARY_BARREL = 0,
	ALT_BARREL,

	NUM_BARRELS,
};

struct AMMO_POOL
{
	AMMO_POOL();

	LTBOOL	Init(CHierarchicalButeMgr & buteMgr, const char* aTagName);

	int			nId;
	char		szName[WMGR_MAX_NAME_LENGTH];
	int			nNameId;
	char		szPickupIcon[WMGR_MAX_FILE_PATH];
	char		szCounterIcon[WMGR_MAX_FILE_PATH];
	int			nSpawnedAmount;
	int			nDefaultAmount;
	LTBOOL		bSave;

	int			GetMaxAmount(HOBJECT hCharacter);
private:
	int			nMaxAmount;
};

struct AMMO
{
	AMMO();

	LTBOOL	Init(CHierarchicalButeMgr & buteMgr, const char* aTagName);
	void	Cache(CWeaponMgr* pWeaponMgr);

	int			nId;
	int			nNameId;
	AmmoType	eType;
	char		szName[WMGR_MAX_NAME_LENGTH];
	char		szAmmoPool[WMGR_MAX_NAME_LENGTH];
	int			nInstDamage;
	DamageType	eInstDamageType;
	int			nAreaDamage;
	int			nAreaDamageRadius;
	DamageType	eAreaDamageType;
	LTFLOAT		fAreaDamageDuration;
	LTFLOAT		fProgDamage;
	LTFLOAT		fProgDamageDuration;
	LTFLOAT		fShellCasingDelay;
	DamageType	eProgDamageType;
	int			nAmmoPerShot; 
	LTBOOL		bBubbleTrail; 

	int			GetMaxPoolAmount(HOBJECT hCharacter);

	PROJECTILEFX*	pProjectileFX;	// Points at CFXButeMgr::m_ProjectileFXList element
	IMPACTFX*		pImpactFX;		// Points at CFXButeMgr::m_ImpactFXList element
	IMPACTFX*		pUWImpactFX;	// Points at CFXButeMgr::m_ImpactFXList element
	FIREFX*			pFireFX;		// Points at CFXButeMgr::m_FireFXList element
	TRACERFX*		pTracerFX;		// Points at CFXButeMgr::m_TracerFXList element
};

struct BARREL
{
	//constructor of sorts
	BARREL();	

	//initialization routine
	LTBOOL	Init(CHierarchicalButeMgr &buteMgr, const char* aTagName);
	void	Cache(CWeaponMgr* pWeaponMgr);

	//save routine
	void	Save(CButeMgr &buteMgr);

	int		nId;
	char	szName[WMGR_MAX_NAME_LENGTH];
	int		nNameId;

	LTVector	vMuzzlePos;
	int			nMinXPerturb;
	int			nMaxXPerturb;
	int			nMinYPerturb;
	int			nMaxYPerturb;
	int			nRange;
	int			nRangeSqr;
	LTVector	vMinRecoilAmount;
	LTVector	vMaxRecoilAmount;
	LTVector	vRecoilTimes;
	int			nVectorsPerRound;
	CMuzzleFX*	pPVMuzzleFX;	// Player-view muzzle fx
	LTFLOAT		fHHBreachOffset;
	LTFLOAT		fPVBreachOffset;
	CMuzzleFX*	pHHMuzzleFX;	// Hand-held muzzle fx
	char		szFireSound[WMGR_MAX_FILE_PATH];
	char		szAltFireSound[WMGR_MAX_FILE_PATH];
	char		szDryFireSound[WMGR_MAX_FILE_PATH];
	char		szMiscSounds[WMGR_MAX_MISC_SNDS][WMGR_MAX_FILE_PATH];
	char		szClipIcon[WMGR_MAX_FILE_PATH];
	LTFLOAT		fFireSoundRadius;
	LTBOOL		bInfiniteAmmo;
	LTBOOL		bDecloak;
	LTBOOL		bSquelchMultipleImpacts;
	LTBOOL		bDrawChooserBars;
	int			nShotsPerClip;
	int			nAmmoType;
	BarrelType eBarrelType;
	char		szNextBarrel[WMGR_MAX_NAME_LENGTH];
	char		szPVAltSkin[WMGR_MAX_FILE_PATH];
	uint8		nNumFlashSockets;
	char		szFlashSocket[WMGR_MAX_FLASH_SOCKETS][WMGR_MAX_NAME_LENGTH];
	LTBOOL		bSave;
};


struct WEAPON
{
	WEAPON();
	~WEAPON();

	LTBOOL	Init(CHierarchicalButeMgr & buteMgr, const char* aTagName);

	void	Cache(CWeaponMgr* pWeaponMgr, CString* szAppend=LTNULL);
	void	Save(CButeMgr & buteMgr);

	int		nId;
	HSTRING	hName;
	HSTRING	hDescription;
	int		nAniType;
	int		nCounterType;
	int		nNumPVFXTypes;
	int		aPVFXTypes[WMGR_MAX_PVFX_TYPES];
	int		aBarrelTypes[NUM_BARRELS];
	int		nDefaultBarrelType;
	int		nTargetingType;
	LTFLOAT fCrosshairScale;
	LTFLOAT fCrosshairAlpha;
	LTBOOL	bMelee;
	LTBOOL	bMPCache;

	char	szName[WMGR_MAX_NAME_LENGTH];
	char	szIcon[WMGR_MAX_FILE_PATH];
	char	szPickupIcon[WMGR_MAX_FILE_PATH];
	char	szPickupName[WMGR_MAX_FILE_PATH];
	char	szPVModel[WMGR_MAX_FILE_PATH];
	char	szPVSkins[MAX_MODEL_TEXTURES][WMGR_MAX_FILE_PATH];
	char	szHHModel[WMGR_MAX_FILE_PATH];
	char	szHHSkins[MAX_MODEL_TEXTURES][WMGR_MAX_FILE_PATH];
	char	szSelectSound[WMGR_MAX_FILE_PATH];
	char	szDeselectSound[WMGR_MAX_FILE_PATH];
//	char	szTargetingSprite[WMGR_MAX_FILE_PATH];
	char	szCrosshair[WMGR_MAX_FILE_PATH];

	LTVector	vPos;
	LTVector	vHHScale;
//	LTVector	vTargetingSpriteScale;

	uint32	dwAlphaFlag;

	LTBOOL	bLoopFireAnim;
	LTBOOL	bLoopAltFireAnim;

	uint8	nAimingPitchSet;
	LTFLOAT fSpeedMod;
	LTBOOL		bSave;
};


struct WEAPON_SET
{
	WEAPON_SET();

	LTBOOL	Init(CHierarchicalButeMgr & buteMgr, const char* aTagName);

	int		nId;
	char	szName[WMGR_MAX_NAME_LENGTH];
	int		nNumWeapons;
	char	szWeaponName[WMGR_MAX_WEAPONS_IN_SET][WMGR_MAX_NAME_LENGTH];
	char	bDefaultWeapon[WMGR_MAX_WEAPONS_IN_SET];
	char	bCanPickup[WMGR_MAX_WEAPONS_IN_SET];
};


struct WeaponPath
{
	WeaponPath()
	{
		vPath.Init();
		vR.Init();
		vU.Init();
		nBarrelId = WMGR_INVALID_ID;
		fPerturbU = 1.0f;	// 0.0 - 1.0
		fPerturbR = 1.0f;	// 0.0 - 1.0
	}

	LTVector	vPath;
	LTVector vU;
	LTVector vR;
	uint8	nBarrelId;
	LTFLOAT	fPerturbU;
	LTFLOAT	fPerturbR;
};

class CWeaponMgr : public CHierarchicalButeMgr
{
	public :
	
		CWeaponMgr();
		~CWeaponMgr();

#ifndef _CLIENTBUILD
		void ReadWeapon(const char* szWeaponName, uint8 & nWeaponId, uint8 & nBarrelId, uint8 & nAmmoId, uint8 &nAmmoPoolId);
		void ReadBarrel(const char* szBarrelName, uint8 & nBarrelId, uint8 & nAmmoId, uint8 &nAmmoPoolId);
		uint8	FindWeaponWithBarrel(uint8 nBarrelId);
#endif // _CLIENTBUILD

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile=WEAPON_DEFAULT_FILE);
		void		Term();

		void		CacheAll();
		void		CacheWeapon(int nWeaponId);

		void		CalculateWeaponPath(WeaponPath & wp);
	
		LTBOOL		WriteFile();
		void		Reload(ILTCSBase *pInterface);

		int			GetNumWeapons()		const { return m_nNumWeapons; }
		int			GetNumAmmoTypes()	const { return m_nNumAmmos; }
		int			GetNumBarrels()		const { return m_nNumBarrels; }
		int			GetNumAmmoPools()	const { return m_nNumAmmoPools; }

		int			GetPoolId(uint8 nAmmoId);

		LTBOOL		IsValidWeapon(int nWeaponId) const { return nWeaponId >= 0 && nWeaponId < m_nNumWeapons; }
		LTBOOL		IsValidBarrel(int nBarrelId) const { return nBarrelId >= 0 && nBarrelId < m_nNumBarrels; }
		LTBOOL		IsValidAmmoType(int nAmmoId) const { return nAmmoId >= 0 && nAmmoId < m_nNumAmmos; }
		LTBOOL		IsValidAmmoPoolType(int nAmmoPoolId) const { return nAmmoPoolId >= 0 && nAmmoPoolId < m_nNumAmmoPools; }

		int			GetWeaponId(int nCommandId);
		int			GetCommandId(int nWeaponId);
		int			GetFirstWeaponCommandId();
		int			GetLastWeaponCommandId();
		int			GetWepSetIndex(const char* pWeaponName, int nWepSet);

		uint8		IsWeaponInSet(int nWeaponSet, const char *pWeaponName);
		uint8		IsAmmoInSet(int nWeaponSet, const char *pAmmoPoolName);

		WEAPON*		GetWeapon(int nWeaponId);
		WEAPON*		GetWeapon(const char* pWeaponName);

		AMMO*		GetAmmo(int nAmmoId);
		AMMO*		GetAmmo(const char* pAmmoName);

		AMMO_POOL*	GetAmmoPool(int nAmmoPoolId);
		AMMO_POOL*	GetAmmoPool(const char* pAmmoPoolName);

		BARREL*		GetBarrel(int nBarrelId);
		BARREL*		GetBarrel(const char* pBarrelName);

		WEAPON_SET* GetWeaponSet(int nSetId);
		WEAPON_SET* GetWeaponSet(const char* pSetName);
		

	protected :

		static bool CountTags(const char *szTagName, void *pData);
		static bool LoadAmmoPoolButes(const char *szTagName, void *pData);
		static bool LoadWeaponButes(const char *szTagName, void *pData);
		static bool LoadAmmoButes(const char *szTagName, void *pData);
		static bool LoadBarrelButes(const char *szTagName, void *pData);
		static bool LoadWeaponSetButes(const char *szTagName, void *pData);

		int					m_nNumAmmoPools;
		AMMO_POOL*			m_pAmmoPools;		// All ammo pool types

		int					m_nNumWeapons;
		WEAPON*				m_pWeapons;			// All weapon types

		int					m_nNumAmmos;
		AMMO*				m_pAmmos;			// All ammo types

		int					m_nNumBarrels;
		BARREL*				m_pBarrels;			// All barrel types

		int					m_nNumWeaponSets;
		WEAPON_SET*			m_pWeaponSets;	// All weapon sets

		CPlayerStats*		m_pPlayerStats;		// pointer to player stats class
};


// Map a commandid to a weapon id...

inline int CWeaponMgr::GetFirstWeaponCommandId()
{
	return COMMAND_ID_WEAPON_BASE;
}

inline int CWeaponMgr::GetLastWeaponCommandId()
{
	return COMMAND_ID_WEAPON_MAX;
}

inline int CWeaponMgr::GetCommandId(int nWeaponId)
{
#ifdef _CLIENTBUILD
	if (nWeaponId < 0)
	{
		return WMGR_INVALID_ID;
	}
	
//	WEAPON* pWep = GetWeapon(m_pPlayerStats->GetWeaponFromSet(nWeaponId));
//
//	if (pWep)
//	{
		return COMMAND_ID_WEAPON_BASE + nWeaponId;
//	}
#else
	int nListLength = m_nNumWeapons;

	if (nWeaponId < 0 || nWeaponId >= nListLength) 
	{
		return WMGR_INVALID_ID;
	}
	
	for (int i=0; i < nListLength; i++)
	{
		WEAPON* pWep = GetWeapon(i);

		if (pWep && pWep->nId == nWeaponId)
		{
			return COMMAND_ID_WEAPON_BASE + i;
		}
	}
#endif
	return WMGR_INVALID_ID;
}


inline LTBOOL FiredWeapon(WeaponState eState)
{
	return (eState == W_FIRED);
}



////////////////////////////////////////////////////////////////////////////
//
// CWeaponMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CWeaponMgrPlugin : public IObjectPlugin
{
	public:

		virtual DRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

		void PopulateStringList(char* const * aszStrings, uint32* pcStrings, 
			const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

};

#endif // _CLIENTBUILD


#endif // __WEAPON_MGR_H__

