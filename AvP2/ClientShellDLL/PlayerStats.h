// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerStats.h
//
// PURPOSE : Definition of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
 
#ifndef __PLAYERSTATS_H
#define __PLAYERSTATS_H

#include "basedefs_de.h"
#include "ObjectivesButeMgr.h"

class CGameClientShell;

class CPlayerStats
{
public:

	CPlayerStats();
	~CPlayerStats();

	LTBOOL		Init();
	void		Term();

	void		OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);
	void		OnExitWorld();

	void		UpdatePlayerWeapon(uint8 nWeaponId, uint8 nAmmoId, LTBOOL bForce=LTFALSE);
	void		Draw(LTBOOL bShowStats);
	void		ResetStats();

	void		Clear();
	void		Update();
	void		UpdateHealth(uint32 nHealth);
	void		UpdateArmor(uint32 nArmor);
	void		UpdateMaxHealth(uint32 nHealth);
	void		UpdateMaxArmor(uint32 nArmor);
	void		UpdateAmmo(uint8 nAmmoPoolId, uint32 nAmt, LTBOOL bPickedup=LTFALSE, LTBOOL bDoHudNotify=LTTRUE);
	void		UpdateClip(uint8 nBarrelId, uint32 nAmt);
	void		UpdateWeapon(uint8 nWeaponId, LTBOOL bPickedup=LTFALSE, LTBOOL bDoHudNotify=LTTRUE);
	void		UpdateAir(LTFLOAT nPercent);
	void		ResetInventory();
	void		DecrementClip(int nBarrelId);
	void		ValidateClips();
	void		UpdateCannonChargeLev(uint32 nLevel);
	void		UpdateBatteryLevel(LTFLOAT fLevel);

	void		Save(HMESSAGEWRITE hWrite);
	void		Load(HMESSAGEREAD hRead);

	uint32		GetAmmoCount(uint8 nAmmoId) const; 
	void		DecrementAmmo(int nAmmoId);
	LTBOOL		HaveWeapon(uint8 nWeaponId) const;

	void		AddCanUseWeapon(uint8 nWeaponId);
	void		AddCanUseAmmo(uint8 nAmmoId);

	void		SetHasMask(LTBOOL bHas);
	LTBOOL		HasMask() const		{ return m_bHasMask; }

	void		SetHasNightVision(LTBOOL bHas);
	LTBOOL		HasNightVision() const		{ return m_bHasNightVision; }

	void		AddToSkullCount()	{++m_nSkullCount;}

	LTBOOL		HaveAirSupply();

	void		NextLayout();
	void		PrevLayout();

	//new for AvP3
	uint32		GetHealthCount();
	uint32		GetArmorCount()		{ return m_nArmor; }
	uint32		GetMaxHealthCount()	{ return m_nMaxHealth; }
	uint32		GetMaxArmorCount()	{ return m_nMaxArmor; }
	LTFLOAT		GetAirPercent()		{ return m_fAirPercent; }
	uint32		GetCurrentLayout()	{ return m_nCurrentLayout; }
	uint32		GetNumWeapons()		{ return m_nWepCount; }
	uint32		GetHUDType();
	uint32		GetWeaponSet()		{ return m_nWepSet; }
	uint32		GetButeSet()		{ return m_nButeSet; }
	uint32		GetCannonChargeLev(){ return m_nCannonChargeLev; }
	uint32		GetExoEnergyCount();
	uint32		GetMaxExoEnergyCount();
	LTFLOAT		GetBatteryLevel()	{ return m_fBatteryChargeLev; }
	uint32		GetSkullCount()	{ return m_nSkullCount; }

	ObjectiveData* GetObjectiveData(uint32 nId) { ASSERT(nId<MAX_OBJECTIVES); return &m_Objective[nId]; }
	void		ClearObjectiveData() { memset(m_Objective, 0, MAX_OBJECTIVES * sizeof(ObjectiveData)); m_nObjectiveOverview= -1; }
	int			GetObjectiveOverview() { return m_nObjectiveOverview; }

	//objective managment
	void	AddObjective(uint32 nId);
	void	UpdateObjective(uint32 nId, ObjectiveState eState);
	void	AddOverview(int nId)	{ m_nObjectiveOverview = nId; } 
	
	void		SetCharacterButes(char *szName);
	void		SetCharacterButes(int nType);
	const char* GetWeaponFromSet(int nIndex);
	int			GetWeaponIndex(int nWepId);
	int			GetAmmoInClip(int nBarrelId);
	int			GetPredatorEnergy();
	int			GetMaxPredatorEnergy();
	int			GetMarineFlares();
	int			GetMaxMarineFlares();

	void		ZoomIn();
	void		ZoomOut();

	//auto-targeting utility functions
	LTBOOL		IsAutoTargetOn	()				{ return m_bAutoTargetOn; }
	void		SetAutoTargetOn	(LTBOOL bOn)	{ m_bAutoTargetOn = bOn; }
	void		SetAutoTarget	(HOBJECT hObj)	{ m_hAutoTarget = hObj; }
	HOBJECT		GetAutoTarget	()				{ return m_hAutoTarget; }
	void		ToggleAutoTargeting	();
	void		SetBarrelID(uint32 nWeaponID, uint32 nBarrelID);
	uint32		GetBarrelID(uint32 nWeaponID);

	LTBOOL		GetResetHealth ()	{ return m_bResetHealth; }
	LTBOOL		GetResetArmor ()	{ return m_bResetArmor; }

	void		SetResetArmor ()	{ m_bResetArmor=LTTRUE; }
	void		SetResetHealth ()	{ m_bResetHealth=LTTRUE; }

private:

	void		DrawPlayerStats(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bShowStats);
	void		DrawScope(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom);
	void		ClearSurface(HSURFACE hSurf, HDECOLOR hColor);

	uint32		m_nButeSet;						// attributes for this character
	uint32		m_nWepSet;						// index of player's weapon set info
	uint32		m_nHealth;						// current health
	uint32		m_nArmor;						// current armor
	uint32		m_nMaxHealth;					// current maximum health
	uint32		m_nMaxArmor;					// current maximum armor
	uint32		m_nSkullCount;					// How many skulls have we collected so far...
	LTFLOAT		m_fAirPercent;					// The current percent of air we have remaining
	uint32*		m_pnAmmoPool;					// All ammo pool values
	uint32*		m_pnBarrelClip;					// All clip values
	LTBOOL*		m_pbHaveWeapon;					// Weapon status
	uint32*		m_nBarrelID;					// What barrel was last used. (multi-barrel weapons)
	LTBOOL		m_bHasMask;						// Do we have a mask? (Predators)
	LTBOOL		m_bHasNightVision;				// Do we have night vision? (Humans)
	uint32		m_nWepCount;					// Quantity of weapons
	uint32		m_nCannonChargeLev;				// Level that the pred cannon is pre-charged to
	LTFLOAT		m_fBatteryChargeLev;			// Level of battery charge.

	LTBOOL		m_bAutoTargetOn;

	uint32		m_nZoomLevel;					// Current zoom level 0 - (MAX_ZOOM_LEVELS-1)
	LTBOOL		m_bCanZoom;						// Can the character use the zoom
	LTBOOL		m_bZooming;						// Are we currently zooming?

	LTBOOL		m_bResetHealth;					// Did we just reset health?
	LTBOOL		m_bResetArmor;					// Did we just reset armor?

	//layout info
	int			m_nCurrentLayout;

	//used for auto-targeting
	HOBJECT		m_hAutoTarget;

	//used for mission objective tracking
	ObjectiveData m_Objective[MAX_OBJECTIVES];
	int			m_nObjectiveOverview;
};

#endif
