// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponModel.cpp
//
// PURPOSE : Generic client-side WeaponModel wrapper class - Implementation
//
// CREATED : 9/27/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponModel.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "ProjectileFX.h"
#include "ClientWeaponUtils.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "CharacterFuncs.h"
#include "ModelButeMgr.h"

#include "CharacterFX.h"
#include "MuzzleFlashFX.h"

// ----------------------------------------------------------------------- //
// Tracked console variables
const LTFLOAT g_vtAmmoSwitchDelay = 0.75f;
const LTFLOAT g_vtZoomInRate = 20.0f;
const LTFLOAT g_vtMaxZoom = 32.00f;

const LTFLOAT g_vtExoMinEnergy = 25.0f;
const LTFLOAT g_vtExoMoveCost = 0.50f;

static VarTrack g_vtViewModelBaseFOVX;
static VarTrack g_vtViewModelBaseFOVY;

static VarTrack g_vtModelBobVert;
static VarTrack g_vtModelBobHoriz;

// ----------------------------------------------------------------------- //
// External variables

extern CGameClientShell* g_pGameClientShell;
extern LTBOOL g_bInfiniteAmmo;

// ----------------------------------------------------------------------- //
// Constant variables

const uint32 INFINITE_AMMO_AMOUNT = 1000;
const uint32 MAX_ZOOM_LEVEL = 32;

#define INVALID_ANI				((HMODELANIM)-1)

const LTFLOAT	WEAPON_MIN_IDLE_TIME	= 15.0;
const LTFLOAT	WEAPON_MAX_IDLE_TIME	= 30.0;

namespace
{
	VarTrack			g_vtFastTurnRate;
	VarTrack			g_vtPerturbRotationEffect;
	VarTrack			g_vtPerturbIncreaseSpeed;
	VarTrack			g_vtPerturbDecreaseSpeed;
	VarTrack			g_vtPerturbWalkPercent;
				
	LTFLOAT m_fLastPitch	= 0.0f;
	LTFLOAT m_fLastYaw		= 0.0f;
}

static bool FindNodeOrSocket(HOBJECT hModel, const char * szNodeName, LTVector * pvPos)
{
	HMODELNODE hNode = INVALID_MODEL_NODE;
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	LTransform transform;

	ModelLT* pModelLT = g_pLTClient->GetModelLT();

	if( LT_OK == pModelLT->GetNode(hModel, const_cast<char*>(szNodeName), hNode) )
	{
		if( LT_OK != pModelLT->GetNodeTransform(hModel, hNode, transform, LTTRUE) )
		{
			return false;
		}
	}
	else if( LT_OK == pModelLT->GetSocket(hModel, const_cast<char*>(szNodeName), hSocket) )
	{
		if( LT_OK != pModelLT->GetSocketTransform(hModel, hSocket, transform, LTTRUE) )
		{
			return false;
		}
	}
	else
	{
		// Couldn't find a node or socket by this name, so ignore it.
		return false;
	}

	if( pvPos )
	{
		LTRotation rRot;
		TransformLT* pTransLT = g_pLTClient->GetTransformLT();
		pTransLT->Get(transform, *pvPos, rRot);
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CWeaponModel()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //
CWeaponModel::CWeaponModel()
{
	m_hObject			= DNULL;
	m_hGlassModel		= DNULL;
	m_hBreachSocket		= INVALID_MODEL_SOCKET;
	m_hGlassSocket		= INVALID_MODEL_SOCKET;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fFlashStartTime	= 0.0f;

	m_vFlashPos.Init();
	m_vFlashOffset.Init();

	m_pMuzzleFlash = LTNULL;

	m_eState				= W_IDLE;
	m_eLastWeaponState		= W_IDLE;
	m_eLastFireType			= FT_NORMAL_FIRE;

	m_bCanSetLastFire		= LTFALSE;
	m_bUsingAltFireAnis		= LTFALSE;
	m_bFireKeyDownLastUpdate= LTFALSE;
	m_bFire					= LTFALSE;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;
	m_nAltSelectAni			= INVALID_ANI;
	m_nAltDeselectAni		= INVALID_ANI;
	m_nAltDeselect2Ani		= INVALID_ANI;
	m_nAltReloadAni			= INVALID_ANI;
	m_nStillAni				= INVALID_ANI;
	m_nChargingAnim			= INVALID_ANI;
	m_nPreChargingAnim		= INVALID_ANI;
	m_nStartFireAnim		= INVALID_ANI;
	m_nEndFireAnim			= INVALID_ANI;
	m_nSpinupAnim			= INVALID_ANI;
	m_nSpinupStartAnim		= INVALID_ANI;
	m_nSpinupEndAnim		= INVALID_ANI;
	m_nChargeFireAni		= INVALID_ANI;
	m_nStartZoomAnim		= INVALID_ANI;
	m_nEndZoomAnim			= INVALID_ANI;
	m_nDetonateAnim			= INVALID_ANI;

	m_fNextIdleTime			= 0.0f;

	for (int i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		m_nFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		m_nIdleAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		m_nAltFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		m_nAltIdleAnis[i] = INVALID_ANI;
	}

	m_vPath.Init();
	m_vFirePos.Init();
	m_vEndPos.Init();

	m_wIgnoreFX				= 0;
	m_eAmmoType				= PROJECTILE;
	m_nRequestedWeaponId	= WMGR_INVALID_ID;
	m_bWeaponDeselected		= LTFALSE;

	m_nLastWeaponId			= WMGR_INVALID_ID;

	m_nWeaponId			= WMGR_INVALID_ID;
	m_nAmmoId			= WMGR_INVALID_ID;
	m_nBarrelId			= WMGR_INVALID_ID;

	m_pWeapon		= DNULL;
	m_pAmmo			= DNULL;	
	m_pBarrel		= DNULL;
	m_pPrimBarrel	= DNULL;
	m_pAltBarrel	= DNULL;
	m_pOldBarrel	= LTNULL;

	m_fMovementPerturb			= 0.0f;
	m_fBarrelSwitchStartTime	= 0.0f;

	m_fCurZoomLevel = 1.0f;
	m_bZooming		= LTFALSE;
	m_bZoomed		= LTFALSE;
	m_bPreZooming	= LTFALSE;

	m_fLastFOVX		= 0.0f;
	m_fLastFOVY		= 0.0f;

	m_bNewShot		= LTTRUE;
	m_hHotbombTarget= LTNULL;
	m_bAutoSpinOn	= LTFALSE;

	m_bDelayedWeaponChange	= LTFALSE;
	m_nDealyedCommandId		= 0;

	m_bAlienTail	= LTFALSE;
	m_bPlaySelectSound = LTTRUE;

	m_nLoopKeyId		= 0xff;
	m_hLoopFireSound	= LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //
CWeaponModel::~CWeaponModel()
{
	delete m_pMuzzleFlash;
	m_pMuzzleFlash = LTNULL;

	//free our object
	if (m_hObject) 
	{
		g_pInterface->DeleteObject(m_hObject);
	}

	//free the mods
	RemoveAttachments();

	// be sure all sounds are squashed
	KillLoopSound();

	m_PVFXMgr.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Init()
//
//	PURPOSE:	Initialize perturb variables
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::Init()
{
	g_vtFastTurnRate.Init(g_pLTClient, "FastTurnRate", NULL, 2.3f);

	LTFLOAT fTemp = g_pLayoutMgr->GetPerturbRotationEffect();
	g_vtPerturbRotationEffect.Init(g_pLTClient, "PerturbRotationEffect", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetPerturbIncreaseSpeed();
	g_vtPerturbIncreaseSpeed.Init(g_pLTClient, "PerturbIncreaseSpeed", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetPerturbDecreaseSpeed();
	g_vtPerturbDecreaseSpeed.Init(g_pLTClient, "PerturbDecreaseSpeed", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetPerturbWalkPercent();
	g_vtPerturbWalkPercent.Init(g_pLTClient, "PerturbWalkPercent", NULL, fTemp);

	g_vtViewModelBaseFOVX.Init(g_pLTClient, "ViewModelBaseFOVX", NULL, 95.0f);
	g_vtViewModelBaseFOVY.Init(g_pLTClient, "ViewModelBaseFOVY", NULL, 115.0f);

	g_vtModelBobVert.Init(g_pLTClient, "ViewModelBobVert", NULL, 0.02f);
	g_vtModelBobHoriz.Init(g_pLTClient, "ViewModelBobHoriz", NULL, 0.02f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Create()
//
//	PURPOSE:	Create the WeaponModel model
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::Create(CClientDE* pClientDE, uint8 nWeaponId, LTBOOL bPlaySelSound)//, uint8 nAmmoId)
{
	//sanity check
	if (!pClientDE) return LTFALSE;

	//save the sound info...
	m_bPlaySelectSound = bPlaySelSound;

	// kill any old sounds
	KillLoopSound();
	
	//clean up old attachments
	RemoveAttachments();

	//record weapon ID
	m_nWeaponId	= nWeaponId;

	//assign our weapon pointer
	m_pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);

	//test
	if (!m_pWeapon) 
		return LTFALSE;

	//assign our primary barrel pointer
	m_pPrimBarrel = g_pWeaponMgr->GetBarrel(m_pWeapon->aBarrelTypes[PRIMARY_BARREL]);

	//set the current barrel = to primary
	m_pBarrel = m_pPrimBarrel;

	//set barrel id and ammo pointer if we have a primary barrel
	if(m_pPrimBarrel)
	{
		m_nBarrelId = m_pPrimBarrel->nId;

		m_pAmmo	= g_pWeaponMgr->GetAmmo(m_pPrimBarrel->nAmmoType);

		//set ammoId and type if we have ammo for this barrel
		if(m_pAmmo)
		{
			m_nAmmoId = m_pAmmo->nId;

			m_eAmmoType	= m_pAmmo->eType;
		}
	}

	//set altfire barrel
	m_pAltBarrel = g_pWeaponMgr->GetBarrel(m_pWeapon->aBarrelTypes[ALT_BARREL]);

	//run initialization routines
	ResetWeaponData();
	CreateModel();
	CreateFlash();
	CreateAttachments();

	m_PVFXMgr.Init(m_hObject, m_pWeapon);

	// set the vis...
	SetVisible(g_pGameClientShell->IsFirstPerson());

	InitAnimations();

	// Update the movement speed...
	g_pGameClientShell->GetPlayerMovement()->SetEncumbered(m_pWeapon->fSpeedMod == 0.0f);

	// Yet another hack..  Argh!
	if(IsAlien(g_pGameClientShell->GetPlayerStats()->GetButeSet()))
		m_bAlienTail	= LTTRUE;
	else
		m_bAlienTail	= LTFALSE;



	// Select the weapon
	Select();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ResetWeaponData
//
//	PURPOSE:	Reset weapon specific data
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ResetWeaponData()
{
	m_hBreachSocket		= INVALID_MODEL_SOCKET;
	m_hGlassSocket		= INVALID_MODEL_SOCKET;

	m_bFire					= LTFALSE;
	m_bCanSetLastFire		= DTRUE;
	m_bWeaponDeselected		= LTFALSE;
	m_bUsingAltFireAnis		= LTFALSE;
	m_bFireKeyDownLastUpdate= LTFALSE;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fFlashStartTime	= 0.0f;

	m_vFlashPos.Init();
	m_vFlashOffset.Init();

	m_eState				= W_IDLE;
	m_eLastWeaponState		= W_IDLE;
	m_eLastFireType			= FT_NORMAL_FIRE;

	m_nRequestedWeaponId	= WMGR_INVALID_ID;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;
	m_nAltSelectAni			= INVALID_ANI;
	m_nAltDeselectAni		= INVALID_ANI;
	m_nAltDeselect2Ani		= INVALID_ANI;
	m_nAltReloadAni			= INVALID_ANI;
	m_nStillAni				= INVALID_ANI;
	m_nChargingAnim			= INVALID_ANI;
	m_nPreChargingAnim		= INVALID_ANI;
	m_nStartFireAnim		= INVALID_ANI;
	m_nEndFireAnim			= INVALID_ANI;
	m_nSpinupAnim			= INVALID_ANI;
	m_nSpinupStartAnim		= INVALID_ANI;
	m_nSpinupEndAnim		= INVALID_ANI;
	m_nChargeFireAni		= INVALID_ANI;
	m_nStartZoomAnim		= INVALID_ANI;
	m_nEndZoomAnim			= INVALID_ANI;
	m_nDetonateAnim			= INVALID_ANI;

//	m_hLoopSound			= LTNULL;

	for (int i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		m_nFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		m_nIdleAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		m_nAltFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		m_nAltIdleAnis[i] = INVALID_ANI;
	}

	m_vPath.Init();
	m_vFirePos.Init();
	m_vEndPos.Init();

	m_wIgnoreFX = 0;

	m_fNextIdleTime	= GetNextIdleTime();

	m_hHotbombTarget = LTNULL;
	m_bAutoSpinOn	= LTFALSE;

	m_bDelayedWeaponChange = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::InitAnimations
//
//	PURPOSE:	Set the animations
//
// ----------------------------------------------------------------------- //

void CWeaponModel::InitAnimations()
{
	if (!m_hObject) return;

	//get primary model animation indicies
	m_nSelectAni	= g_pInterface->GetAnimIndex(m_hObject, "Select");
	m_nDeselectAni	= g_pInterface->GetAnimIndex(m_hObject, "Deselect");
	m_nReloadAni	= g_pInterface->GetAnimIndex(m_hObject, "Reload");
	m_nAmmoChangeAni= g_pInterface->GetAnimIndex(m_hObject, "AmmoChange");

	//get alternate model animation indicies
	m_nAltSelectAni		= g_pInterface->GetAnimIndex(m_hObject, "AltSelect");
	m_nAltDeselectAni	= g_pInterface->GetAnimIndex(m_hObject, "AltDeselect");
	m_nAltDeselect2Ani	= g_pInterface->GetAnimIndex(m_hObject, "AltDeselect2");
	m_nAltReloadAni		= g_pInterface->GetAnimIndex(m_hObject, "AltReload");

	m_nStillAni			= g_pInterface->GetAnimIndex(m_hObject, "Still");
	m_nChargeFireAni	= g_pInterface->GetAnimIndex(m_hObject, "Fire0_Charge");
	m_nStartZoomAnim	= g_pInterface->GetAnimIndex(m_hObject, "ZoomStart");
	m_nEndZoomAnim		= g_pInterface->GetAnimIndex(m_hObject, "ZoomEnd");
	m_nDetonateAnim		= g_pInterface->GetAnimIndex(m_hObject, "Detonate");

	char buf[30];

	//get idle animation indicies
	for (int i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		sprintf(buf, "Idle%d", i);
		m_nIdleAnis[i] = g_pInterface->GetAnimIndex(m_hObject, buf);
	}

	//get fire animation indicies
	for (i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		sprintf(buf, "Fire%d", i);
		m_nFireAnis[i] = g_pInterface->GetAnimIndex(m_hObject, buf);
	}

	//get altrenate idle animation indicies
	for (i=0; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		sprintf(buf, "AltIdle%d", i);
		m_nAltIdleAnis[i] = g_pInterface->GetAnimIndex(m_hObject, buf);
	}

	//get alternate fire indicies
	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		sprintf(buf, "AltFire%d", i);
		m_nAltFireAnis[i] = g_pInterface->GetAnimIndex(m_hObject, buf);
	}

	//get charging animation index
	m_nChargingAnim = g_pInterface->GetAnimIndex(m_hObject, "Charging");

	//get pre-charging animation index
	m_nPreChargingAnim = g_pInterface->GetAnimIndex(m_hObject, "PreCharging");

	//get begin fire animation index
	m_nStartFireAnim = g_pInterface->GetAnimIndex(m_hObject, "Fire0_start");

	//get end fire animation index
	m_nEndFireAnim = g_pInterface->GetAnimIndex(m_hObject, "Fire0_end");

	//get spinup animation index
	m_nSpinupAnim = g_pInterface->GetAnimIndex(m_hObject, "PreFire");

	//get begin spinup animation index
	m_nSpinupStartAnim = g_pInterface->GetAnimIndex(m_hObject, "PreFire_Start");

	//get end spinup animation index
	m_nSpinupEndAnim = g_pInterface->GetAnimIndex(m_hObject, "PreFire_End");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Reset()
//
//	PURPOSE:	Reset the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Reset()
{
	//be sure to re-set zoom
	if(m_fCurZoomLevel > 1)
	{
		m_fCurZoomLevel = 1;
		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		CameraMgrFX_ZoomToggle(hCamera, m_fCurZoomLevel);
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Weapons\\Marine\\RailGun\\zoom.wav", SOUNDPRIORITY_PLAYER_HIGH);
		m_bZooming = m_bZoomed = m_bPreZooming = LTFALSE;
	}

	//terminate the special effects
	delete m_pMuzzleFlash;
	m_pMuzzleFlash = LTNULL;


	m_PVFXMgr.Term();

	//remove the model boject
	RemoveModel();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::RemoveModel()
//
//	PURPOSE:	Remove our model data member
//
// ----------------------------------------------------------------------- //

void CWeaponModel::RemoveModel()
{
	if (!m_hObject) return;

	if (m_hObject) 
	{
		g_pInterface->DeleteObject(m_hObject);
		m_hObject = LTNULL;
	}

	//free the mod attachments
	RemoveAttachments();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::RemoveMods()
//
//	PURPOSE:	Remove our mod data members
//
// ----------------------------------------------------------------------- //

void CWeaponModel::RemoveAttachments()
{
	// Remove the scope model...

	if (m_hGlassModel)
	{
		g_pInterface->DeleteObject(m_hGlassModel);
		m_hGlassModel  = LTNULL;
		m_hGlassSocket = INVALID_MODEL_SOCKET;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateWeaponModel()
//
//	PURPOSE:	Update the WeaponModel state
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::UpdateWeaponModel(LTRotation rRot, LTVector vPos, LTBOOL bFire, FireType eFireType, LTBOOL bForceIdle)
{
	//sanity check
	if (!m_hObject || !g_pInterface || !g_pWeaponMgr) return W_IDLE;

	// See if we can change weapons..
	if(m_bDelayedWeaponChange)
		ChangeWeapon(m_nDealyedCommandId);

	LTBOOL bNeedReload=LTFALSE;

	//if we are fireing and the last fire animation is cmplete
	if (bFire && m_bCanSetLastFire)
	{
		if(m_eState == W_IDLE ||  m_eState== W_SPINNING ||  m_eState== W_SPINDOWN || m_eState== W_SPINUP || (m_pBarrel && m_pBarrel->eBarrelType==BT_ALIEN_SPECIAL))//m_eState== W_SELECT ||  m_eState== W_SPINNING)
		{
			//record the fire type
			m_eLastFireType	= eFireType;
			
			//set the active barrel
			if(eFireType == FT_NORMAL_FIRE )//|| (m_pBarrel && m_pBarrel->eBarrelType==BT_ALIEN_SPECIAL))
			{
				//PRIMARY FIRE
				if(m_pBarrel != m_pPrimBarrel)
				{
					m_pBarrel = m_pPrimBarrel;
					CreateFlash();
				}
				if(m_pBarrel && m_pBarrel->eBarrelType==BT_PRED_DETONATOR)
				{
					//only allow the detonator to fire when we have a auto target
					CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

					if(!pStats || !pStats->GetAutoTarget())
					{
						bFire = LTFALSE;
						m_hHotbombTarget = LTNULL;
					}
					else
					{
						//save out the target just in case we lose it between now
						//and when the fire message comes from the model.
						m_hHotbombTarget = pStats->GetAutoTarget();
					}
				}

				//don't let the fire happen if we are changing ammo
				if(m_eState == W_CHANGE_AMMO)
					bFire = LTFALSE;
			}
			else
			{
				//ALT FIRE
				if(m_pBarrel)
				{
					switch (m_pBarrel->eBarrelType)
					{
					case (BT_SWITCHING):
					case (BT_TARGETING_TOGGLE):
					case (BT_MINIGUN_SPECIAL):
					case (BT_ZOOMING):
						bFire = LTFALSE;
						break;

					case (BT_ALIEN_SPECIAL):
						if(stricmp(m_pWeapon->szName, "Tail")!=0)
							OnAltFireButtonDown();
						bFire = LTFALSE;
						break;
						
					case (BT_CHARGE):
					case (BT_NORMAL):
					default:
						{
							if(m_pBarrel != m_pAltBarrel && !m_bAlienTail)
							{
								m_pBarrel = m_pAltBarrel;
								CreateFlash();
							}
							
							if(m_pBarrel)
							{
								if(StatusAmmo(m_pBarrel) == AS_CLIP_EMPTY)
								{
									bNeedReload=LTTRUE;
								}
							}
						}
						break;
					}
				}	
				else
				{
					bFire = LTTRUE;
					m_pBarrel = m_pAltBarrel;
					CreateFlash();
				}
			}
			
			//if the barrel exists, set the ammo info
			if(m_pBarrel)
			{
				//set id
				m_nBarrelId = m_pBarrel->nId;
				
				//set ammo pointer
				m_pAmmo	= g_pWeaponMgr->GetAmmo(m_pBarrel->nAmmoType);
				
				//set ammoId and type if we have ammo for this barrel
				if(m_pAmmo)
				{
					m_nAmmoId = m_pAmmo->nId;
					
					m_eAmmoType	= m_pAmmo->eType;
				}
			}
		}
		else
		{
			//check to see if we are spinning up
			if(m_pBarrel && m_nStartFireAnim == INVALID_ANI && m_pBarrel->eBarrelType != BT_CHARGE)
				bFire = LTFALSE;

			if(m_eState == W_CHANGE_AMMO || m_eState == W_SELECT)
				bFire = LTFALSE;
		}
	}
	else
	{
		// Be sure bFire is off...
		bFire = LTFALSE;
	}

	//check to see if we are reloading
	if(bNeedReload)
	{
		bFire = LTFALSE;
		ReloadClip();
	}

	//record some stuff
	m_bFireKeyDownLastUpdate = bFire;
	m_eLastWeaponState		 = m_eState;

	// Update the state of the model...
	WeaponState eState;

	if(bForceIdle)
	{
		eState = W_IDLE;

		if((m_eState != W_IDLE) && (m_eState != W_END_FIRING))
		{
			OnFireButtonUp();
			OnAltFireButtonUp();
		}
	}
	else
	{
		eState = UpdateModelState(bFire);
	}

	// Compute offset for WeaponModel and move the model to the correct position
	LTVector vU(0,1,0), vR(1,0,0), vF(0,0,1);
	LTVector vNewPos(0,0,0);
	
	LTVector vFlashU, vFlashR, vFlashF;
	g_pInterface->GetRotationVectors(&rRot, &vFlashU, &vFlashR, &vFlashF);

	// record the forward vector and position
	m_vPath = vFlashF;
	m_vFirePos = vPos;

	//get weapon model and muzzle offsets
	LTVector vOffset	= GetWeaponOffset();
	LTVector vMuzzleOffset = GetMuzzleOffset();

	//calculate new position for model
	vNewPos += vR * (vOffset.x + m_fBobWidth);
	vNewPos += vU * (vOffset.y + m_fBobHeight);
	vNewPos += vF * vOffset.z;

	m_vFlashOffset.Init();

	m_vFlashOffset.x = (vOffset.x + vMuzzleOffset.x + m_fBobWidth);
	m_vFlashOffset.y = (vOffset.y + vMuzzleOffset.y + m_fBobHeight);
	m_vFlashOffset.z = (vOffset.z + vMuzzleOffset.z);

	//calculate new position for muzzle flash
	m_vFlashPos = vPos;
	m_vFlashPos += vFlashR * (vOffset.x + vMuzzleOffset.x + m_fBobWidth);
	m_vFlashPos += vFlashU * (vOffset.y + vMuzzleOffset.y + m_fBobHeight);
	m_vFlashPos += vFlashF * (vOffset.z + vMuzzleOffset.z);

	if (FiredWeapon(eState))
	{
		//start muzzle flash effects
		m_fFlashStartTime = g_pInterface->GetTime();

		m_bNewShot = LTTRUE;
		
		//send message to server telling player to fire...
		SendFireMsg();
	}
	//set new position
	g_pInterface->SetObjectPos(m_hObject, &vNewPos, DTRUE);

	//update the muzzle flash...
	UpdateFlash(eState);

	//update the mods...
	UpdateAttachments();

	//update any player-view fx...
	m_PVFXMgr.Update();

	//update the perturb factor (accuracy)
	UpdateMovementPerturb();

	//update zooming
	UpdateZooming();

	// Update speed mod...
	UpdateWeaponSpeedMod();

	//----------------------------------------------------------------//
	// Update FOV offsets if needed
	LTFLOAT fFOVX, fFOVY;
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();

	CAMERAMGRFX_WONKY *pData = LTNULL;

	if(CameraMgrFX_Active(hCamera, CAMERAMGRFX_ID_WONKY))
	{
		//See that we are not already ramping down...
		CAMERAEFFECTINSTANCE*pFX = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_WONKY, LTNULL); 

		if(pFX)
		{
			pData = (CAMERAMGRFX_WONKY*)pFX->ceCS.pUserData;
		}
	}

	g_pCameraMgr->GetCameraFOV(hCamera, fFOVX, fFOVY, LTTRUE);

	if((fFOVX != m_fLastFOVX) || (fFOVY != m_fLastFOVY))
	{
		m_fLastFOVX = fFOVX;
		m_fLastFOVY = fFOVY;

		// Update the console variables
		char szBuffer[64];
		if(!pData)
			sprintf(szBuffer, "ExtraFOVXOffset %f", g_vtViewModelBaseFOVX.GetFloat() - fFOVX);
		else
			sprintf(szBuffer, "ExtraFOVXOffset %f", g_vtViewModelBaseFOVX.GetFloat() - fFOVX - pData->fFOVX);
		g_pLTClient->RunConsoleString(szBuffer);

		if(!pData)
			sprintf(szBuffer, "ExtraFOVYOffset %f", g_vtViewModelBaseFOVY.GetFloat() - fFOVY);
		else
			sprintf(szBuffer, "ExtraFOVYOffset %f", g_vtViewModelBaseFOVY.GetFloat() - fFOVY - pData->fFOVY);

		g_pLTClient->RunConsoleString(szBuffer);
	}

	//----------------------------------------------------------------//
	// Update the cloaking state

	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(hClientObj, &r, &g, &b, &a);

	// Cap it off so we can always see our weapon at least a little bit
	if(a < 0.2f) a = 0.2f;
	g_pLTClient->SetObjectColor(m_hObject, r, g, b, a);


	uint32 dwUserFlags, dwObjUserFlags;

	g_pLTClient->GetObjectUserFlags(hClientObj, &dwUserFlags);
	g_pLTClient->GetObjectUserFlags(m_hObject, &dwObjUserFlags);

	if(dwUserFlags & USRFLG_GLOW)
		g_pLTClient->SetObjectUserFlags(m_hObject, dwObjUserFlags | USRFLG_GLOW);
	else
		g_pLTClient->SetObjectUserFlags(m_hObject, dwObjUserFlags & ~USRFLG_GLOW);

	//----------------------------------------------------------------//

	return eState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateBob()
//
//	PURPOSE:	Update WeaponModel bob
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateBob(LTFLOAT fWidth, LTFLOAT fHeight)
{
	m_fBobWidth  = fWidth;
	m_fBobHeight = fHeight;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateMods()
//
//	PURPOSE:	Update mod attachments
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateAttachments()
{
	if (!m_hObject || !g_pLTClient || !g_pModelLT || !g_pInterface) return;

	// Right now just do this for the blowtorch
	if(stricmp(m_pWeapon->szName, "Blowtorch") == 0)
	{
		LTVector		vPos;
		LTRotation	rRot;

		//get the weapon's position vector
		g_pLTClient->GetObjectPos(m_hObject, &vPos);

		//update the scope...
		if (m_hGlassModel)
		{
			if (m_hGlassSocket != INVALID_MODEL_SOCKET)
			{
				LTransform transform;
				if (g_pModelLT->GetSocketTransform(m_hObject, m_hGlassSocket, transform, DTRUE) == LT_OK)
				{
					g_pTransLT->Get(transform, vPos, rRot);
					g_pInterface->SetObjectPos(m_hGlassModel, &vPos, DTRUE);
					g_pInterface->SetObjectRotation(m_hGlassModel, &rRot);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetVisible()
//
//	PURPOSE:	Hide/Show the weapon model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetVisible(LTBOOL bVis)
{
	if (!m_hObject || !g_pInterface) return;
	
	// Hide/Show weapon model...
	uint32 dwFlags = g_pInterface->GetObjectFlags(m_hObject);

	if (bVis) 
	{
		dwFlags |= FLAG_VISIBLE;
	}
	else 
	{
		dwFlags &= ~FLAG_VISIBLE;
	}

	g_pInterface->SetObjectFlags(m_hObject, dwFlags);
	

	// Always hide the flash (it will be shown when needed)...
	if( m_pMuzzleFlash )
		m_pMuzzleFlash->Hide();

	m_PVFXMgr.SetVisible(bVis);

	// Hide/Show scope...
	if (m_hGlassModel) 
	{
		dwFlags = g_pInterface->GetObjectFlags(m_hGlassModel);
		if (bVis) 
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else 
		{
			dwFlags &= ~FLAG_VISIBLE;
		}
		g_pInterface->SetObjectFlags(m_hGlassModel, dwFlags);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsVisible()
//
//	PURPOSE:	Get whether the weapon is visible or not
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsVisible()
{
	if(!m_hObject || !g_pInterface) return LTFALSE;
	
	uint32 dwFlags = g_pInterface->GetObjectFlags(m_hObject);
	return (dwFlags & FLAG_VISIBLE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateFlash
//
//	PURPOSE:	Create the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateFlash()
{
	if (!m_pWeapon || !m_pBarrel) return;

	m_fFlashStartTime = 0.0f;

	// Setup Breach socket (if it exists)...
	m_hBreachSocket = INVALID_MODEL_SOCKET;
	if (m_hObject)
	{
		if (g_pModelLT->GetSocket(m_hObject, "Breach", m_hBreachSocket) != LT_OK)
		{
			m_hBreachSocket = INVALID_MODEL_SOCKET;
		}
	}

	//get position and rotation vectors
	LTVector		vPos;
	LTRotation	rRot;
	g_pLTClient->GetObjectPos(m_hObject, &vPos);
	g_pLTClient->GetObjectRotation(m_hObject, &rRot);

	//initialize create struct
	MUZZLEFLASHCREATESTRUCT mf;

	mf.bPlayerView	= LTTRUE;//g_pGameClientShell->GetCameraMgr()->GetCameraActive(g_pGameClientShell->GetPlayerCamera());
	mf.hParent		= m_hObject;
	mf.pBarrel		= m_pBarrel;
	mf.vPos			= vPos;
	mf.rRot			= rRot;
	mf.nFlashSocket = 0;

	//set up muzzle flash member and hide
	delete m_pMuzzleFlash;
	m_pMuzzleFlash = new CMuzzleFlashFX();

	ASSERT( m_pMuzzleFlash );
	if( m_pMuzzleFlash )
	{
		m_pMuzzleFlash->Setup(mf);
		m_pMuzzleFlash->Hide();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateModel
//
//	PURPOSE:	Create the weapon model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateModel()
{
	if(!m_pWeapon) return;

	ObjectCreateStruct createStruct;

	//init create struct
	INIT_OBJECTCREATESTRUCT(createStruct);

	//load model name
	SAFE_STRCPY(createStruct.m_Filename, m_pWeapon->szPVModel);

	
	//get player stats
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	CString szAppend;

	if(pStats)
	{
		uint32 nSet = pStats->GetButeSet();
		szAppend = g_pCharacterButeMgr->GetWeaponSkinType(nSet);
	}


	//load skin names
	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		if(szAppend.IsEmpty())
		{
			SAFE_STRCPY(createStruct.m_SkinNames[i], m_pWeapon->szPVSkins[i]);
		}
		else
		{
			sprintf(createStruct.m_SkinNames[i], m_pWeapon->szPVSkins[i], szAppend);
		}
	}

	//see if we need to change barrels
	uint32 nBarrelID = pStats->GetBarrelID(m_pWeapon->nId);
	if(nBarrelID != (uint32)-1)
	{
		if(stricmp(m_pWeapon->szName, "Blowtorch")!=0)
		{
			m_pPrimBarrel = g_pWeaponMgr->GetBarrel(nBarrelID);
			CreateFlash();
		}
	}

	if(m_pPrimBarrel && (stricmp(m_pPrimBarrel->szName, "SADAR_Barrel")==0 || stricmp(m_pPrimBarrel->szName, "SADAR_Barrel_MP")==0))
	{
		g_pInterfaceMgr->InitTargetingMgr(TT_MARINE_SADAR_NORMAL);
	}
	else
	{
		g_pInterfaceMgr->InitTargetingMgr(m_pWeapon->nTargetingType);
	}

	if(m_pPrimBarrel && (stricmp(m_pWeapon->szName, "Grenade_Launcher")==0 || stricmp(m_pWeapon->szName, "Grenade_Launcher_MP")==0))
		//overwrite the third one
		SAFE_STRCPY(createStruct.m_SkinNames[2], m_pPrimBarrel->szPVAltSkin);

	//create object
	m_hObject = CreateModelObject(m_hObject, createStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateModelObject
//
//	PURPOSE:	Create a weaponmodel model object
//
// ----------------------------------------------------------------------- //

HOBJECT CWeaponModel::CreateModelObject(HOBJECT hOldObj, ObjectCreateStruct & createStruct)
{
	if (!m_pWeapon || !g_pInterface || !g_pLTClient) return DNULL;

	HOBJECT hObj = hOldObj;

	//get pointer to common interface
	ILTCommon* pCommon = g_pInterface->Common();

	//test
	if(!pCommon) return DNULL;

	if (!hObj)
	{
		//finish filling in the create struct
		createStruct.m_ObjectType	= OT_MODEL;
		createStruct.m_Flags		= FLAG_VISIBLE | FLAG_REALLYCLOSE;
		createStruct.m_Flags2		= FLAG2_PORTALINVISIBLE | FLAG2_DYNAMICDIRLIGHT;

		//create the object itself
		hObj = g_pInterface->CreateObject(&createStruct);

		//test
		if (!hObj) return DNULL;

		g_pInterface->SetObjectColor(hObj, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	//record the filenames
	pCommon->SetObjectFilenames(hObj, &createStruct);

	//set the environment map flag if applicable
	uint32 dwFlags = g_pInterface->GetObjectFlags(hObj);
	dwFlags &= ~FLAG_ENVIRONMENTMAP;
	dwFlags |= m_pWeapon->dwAlphaFlag;
	g_pInterface->SetObjectFlags(hObj, dwFlags);

	//set the notify model keys flag
	uint32 dwCFlags = g_pLTClient->GetObjectClientFlags(hObj);
	g_pLTClient->SetObjectClientFlags(hObj, dwCFlags | CF_NOTIFYMODELKEYS);

	//set animation and looping to off
	g_pInterface->SetModelLooping(hObj, LTFALSE);
	g_pInterface->SetModelAnimation(hObj, 0);

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateScope
//
//	PURPOSE:	Create the scope model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateAttachments()
{
	if (!m_pWeapon) return;

	// Right now just do this for the blowtorch
	if(stricmp(m_pWeapon->szName, "Blowtorch") == 0)
	{
		m_hGlassSocket = INVALID_MODEL_SOCKET;
	
		// Make sure we have a socket for the glass...

		if (m_hObject)
		{
			if (g_pModelLT->GetSocket(m_hObject, "Glass", m_hGlassSocket) != LT_OK)
			{
				if (m_hGlassSocket == INVALID_MODEL_SOCKET)
				{
					return;
				}
			}
		}


		// Okay create/setup the model...

		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);
		
		SAFE_STRCPY(createStruct.m_Filename, "\\models\\weapons\\marine\\mBlowtorch_glass.abc");
		SAFE_STRCPY(createStruct.m_SkinNames[0], "\\skins\\weapons\\marine\\mBlowtorch_glass.dtx");

		m_hGlassModel = CreateModelObject(m_hGlassModel, createStruct);

		if (m_hGlassModel)
		{
			uint32 dwFlags = g_pInterface->GetObjectFlags(m_hGlassModel);
			g_pInterface->SetObjectFlags(m_hGlassModel, dwFlags | FLAG_VISIBLE);

			LTFLOAT r, g, b, a;
			g_pLTClient->GetObjectColor(m_hGlassModel, &r, &g, &b, &a);
			g_pLTClient->SetObjectColor(m_hGlassModel, r, g, b, 0.99f);
		}

		UpdateAttachments();

		//be sure to hide the glass if we are in third person
		SetVisible(g_pGameClientShell->IsFirstPerson());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateFlash()
//
//	PURPOSE:	Update muzzle flash state
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateFlash(WeaponState eState)
{
	//sanity check
	if(!g_pInterface || !m_pBarrel || !g_pGameClientShell) return;
	
	uint32 dwFlags = g_pInterface->GetObjectFlags(m_hObject);

	//if we have a muzzle flash and it's not visible, hide it
	if (!(dwFlags & FLAG_VISIBLE) || !m_pBarrel->pPVMuzzleFX)
	{
		if( m_pMuzzleFlash )
			m_pMuzzleFlash->Hide();
		return;
	}

	LTFLOAT fCurTime			= g_pInterface->GetTime();
	LTFLOAT fFlashDuration	= m_pBarrel->pPVMuzzleFX->fDuration;

	//hide the flash if
	// 1) we are past the flash duration
	// 2) the player is dead
	// 3) we are in liquid

	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();

	//otherwise, update the flash
	if(	fCurTime >= m_fFlashStartTime + fFlashDuration ||
		g_pGameClientShell->GetPlayerState() != PS_ALIVE ||
		CameraMgrFX_VolumeBrushInLiquid(hCamera))
	{
		if( m_pMuzzleFlash )
			m_pMuzzleFlash->Hide();
	}	
	else
	{
		LTVector vDir = m_vPath; // Old way...

		LTVector vU, vR;
		LTRotation rRot;

		HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

		if (!hCamera) return;

		g_pInterface->GetObjectRotation(hCamera, &rRot);
		g_pInterface->GetRotationVectors(&rRot, &vU, &vR, &vDir);
		
		g_pInterface->AlignRotation(&rRot, &vDir, DNULL);

		if( m_pMuzzleFlash )
		{
			m_pMuzzleFlash->Show(m_bNewShot);
			m_pMuzzleFlash->SetPos(m_vFlashPos, m_vFlashOffset);
			m_pMuzzleFlash->SetRot(rRot);
			m_pMuzzleFlash->Update();
		}

		m_bNewShot = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetModelPos()
//
//	PURPOSE:	Get the position of the weapon model
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetModelPos() const
{
	LTVector vPos;
	VEC_INIT(vPos);

	if (g_pInterface && m_hObject)
	{
		g_pInterface->GetObjectPos(m_hObject, &vPos);
	}
			
	return vPos;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::StringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CWeaponModel::OnModelKey(HLOCALOBJ hObj, ArgList* pArgList)
{
	if (	!m_pWeapon || 
			!hObj || 
			(hObj != m_hObject) || 
			!pArgList || 
			!pArgList->argv || 
			pArgList->argc == 0 || 	
			g_pGameClientShell->IsPlayerDead()
		) return;

	//extract the first key
	char* pKey = pArgList->argv[0];

	//test
	if (!pKey) return;

	//handle fire key
	if (stricmp(pKey, WEAPON_KEY_FIRE) == 0)
	{
		m_bFire = DTRUE;
	}
	//handle sound key
	else if (stricmp(pKey, WEAPON_KEY_SOUND) == 0)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			char* pBuf = DNULL;

			WeaponModelKeySoundId nId = (WeaponModelKeySoundId)atoi(pArgList->argv[1]);
			switch (nId)
			{
				case WMKS_FIRE:
					if(m_pBarrel)
						pBuf = m_pBarrel->szFireSound;
				break;

				case WMKS_ALT_FIRE:
					if(m_pBarrel)
						pBuf = m_pBarrel->szAltFireSound;
				break;

				case WMKS_DRY_FIRE:
					if(m_pBarrel)
						pBuf = m_pBarrel->szDryFireSound;
				break;

				case WMKS_MISC0:
				case WMKS_MISC1:
				case WMKS_MISC2:
					if(m_pBarrel)
						pBuf = m_pBarrel->szMiscSounds[nId];
				break;

				case WMKS_SELECT:
					if(m_bPlaySelectSound)
						pBuf = m_pWeapon->szSelectSound;
				break;

				case WMKS_DESELECT:
					pBuf = m_pWeapon->szDeselectSound;
				break;

				case WMKS_INVALID:
				default : break;
			}

			if (pBuf && pBuf[0])
			{
				g_pClientSoundMgr->PlaySoundLocal(pBuf, SOUNDPRIORITY_PLAYER_HIGH);

				// Send message to Server so that other client's can hear this sound...
				HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_SOUND);
				g_pInterface->WriteToMessageByte(hWrite, nId);
				g_pInterface->WriteToMessageByte(hWrite, m_nWeaponId);
				g_pInterface->WriteToMessageByte(hWrite, m_nBarrelId);
				g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
			}
		}
	}
	//handle a buted sound
	else if (stricmp(pKey, WEAPON_KEY_BUTE_SOUND) == 0)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			g_pClientSoundMgr->PlaySoundLocal(pArgList->argv[1]);
		}
	}
	//handle loop sound key
	else if (stricmp(pKey, WEAPON_KEY_LOOPSOUND) == 0)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			// Send message to Server so that other client's can hear this sound...
			WeaponModelKeySoundId nId = (WeaponModelKeySoundId)atoi(pArgList->argv[1]);

			char* pBuf = DNULL;
			switch (nId)
			{
				case WMKS_FIRE:		if(m_pBarrel)	pBuf = m_pBarrel->szFireSound;			break;
				case WMKS_ALT_FIRE: if(m_pBarrel)	pBuf = m_pBarrel->szAltFireSound;		break;
				case WMKS_DRY_FIRE:	if(m_pBarrel)	pBuf = m_pBarrel->szDryFireSound;		break;
				case WMKS_MISC0:
				case WMKS_MISC1:
				case WMKS_MISC2:	if(m_pBarrel)	pBuf = m_pBarrel->szMiscSounds[nId];	break;

				case WMKS_INVALID:
				default :			break;
			}

			if(stricmp(pArgList->argv[1], "stop") == 0)
			{
				KillLoopSound();

				HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_LOOPSOUND);
				g_pInterface->WriteToMessageByte(hWrite, nId);
				g_pInterface->WriteToMessageByte(hWrite, 0xff);
				g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
			}
			else if (pBuf && pBuf[0])
			{
				if(!m_hLoopFireSound || (nId != m_nLoopKeyId))
				{
					// play the sound here, make sure to nuke any old sound
					KillLoopSound();

					// now play it locally
					m_hLoopFireSound = g_pClientSoundMgr->PlaySoundLocal(	pBuf, 
																			SOUNDPRIORITY_PLAYER_HIGH, 
																			PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE );

					// record our barrel that matches our fire sound
					m_nLoopKeyId = nId;

					// now tell the rest of the clients
					HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_LOOPSOUND);
					g_pInterface->WriteToMessageByte(hWrite, nId);
					g_pInterface->WriteToMessageByte(hWrite, m_nBarrelId);
					g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
				}
			}
		}
	}
	//handle charge messages
	else if (stricmp(pKey, WEAPON_KEY_CHARGE) == 0)
	{
		//advance the barrel
		if(m_pBarrel)
		{
			//save our barrel
			BARREL* pTemp = m_pBarrel;

			m_pBarrel = g_pWeaponMgr->GetBarrel(m_pBarrel->szNextBarrel);

			if(m_pBarrel)
			{
				m_nBarrelId = m_pBarrel->nId;

				m_pAmmo	= g_pWeaponMgr->GetAmmo(m_pBarrel->nAmmoType);

				//set ammoId and type if we have ammo for this barrel
				if(m_pAmmo)
				{
					m_nAmmoId = m_pAmmo->nId;

					m_eAmmoType	= m_pAmmo->eType;
				}
			}
			else
			{
				//hmmm should have a next barrel, somthing is wrong
				//reset the barrel back to the old one
				m_pBarrel = pTemp;
			}
		}
	}
	//handle special effects
	else if (stricmp(pKey, WEAPON_KEY_FX) == 0)
	{
		m_PVFXMgr.HandleFXKey(pArgList);
	}
	else if (stricmp(pKey, WEAPON_KEY_CAMERAFX) == 0)
	{
		g_pGameClientShell->HandleCameraEffect(pArgList);
	}
	else if (stricmp(pKey, WEAPON_KEY_SELECT_COMPLETE) == 0)
	{
		if(StatusAmmo(m_pBarrel) == AS_CLIP_EMPTY)
			ReloadClip(DTRUE);
	}
	else if (stricmp(pKey, WEAPON_KEY_KICK_BACK) == 0)
	{

		//get player stats
		CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

		//check
		if(!pStats || !m_pBarrel)
			return;

		//get infinite ammo boolean
		LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pBarrel->bInfiniteAmmo);

		//get ammo or infinite amount
		int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : pStats->GetAmmoCount(m_nAmmoId);

		// If this weapon uses ammo, make sure we have ammo...
		g_pGameClientShell->HandleWeaponKickBack();
	}
	else if (stricmp(pKey, WEAPON_KEY_END_TAIL) == 0)
	{
		ChangeWeapon(COMMAND_ID_WEAPON_BASE);
	}
	else if (stricmp(pKey, WEAPON_KEY_AMMO_CHANGE) == 0)
	{
		//change out the skin here
		if(stricmp(m_pWeapon->szName, "Grenade_Launcher")==0 || stricmp(m_pWeapon->szName, "Grenade_Launcher_MP")==0)
		{
			//swap out the texture #2
			if (!m_pWeapon || !m_pBarrel) return;

			ObjectCreateStruct createStruct;

			//init create struct
			INIT_OBJECTCREATESTRUCT(createStruct);

			//load model name
			SAFE_STRCPY(createStruct.m_Filename, m_pWeapon->szPVModel);


			//get player stats
			CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
			CString szAppend;

			if(pStats)
			{
				uint32 nSet = pStats->GetButeSet();
				szAppend = g_pCharacterButeMgr->GetWeaponSkinType(nSet);
			}


			//load skin names
			for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
			{
				if(szAppend.IsEmpty())
				{
					SAFE_STRCPY(createStruct.m_SkinNames[i], m_pWeapon->szPVSkins[i]);
				}
				else
				{
					sprintf(createStruct.m_SkinNames[i], m_pWeapon->szPVSkins[i], szAppend);
				}
			}


			//overwrite the third one
			SAFE_STRCPY(createStruct.m_SkinNames[2], m_pPrimBarrel->szPVAltSkin);

			//get pointer to common interface
			ILTCommon* pCommon = g_pInterface->Common();

			//record the filenames
			if(pCommon)
				pCommon->SetObjectFilenames(m_hObject, &createStruct);

			if(StatusAmmo(m_pPrimBarrel) == AS_CLIP_EMPTY)
			{
				CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
				int nAmmoCount = pStats->GetAmmoCount(m_pPrimBarrel->nAmmoType);
				int nShotsPerClip = m_pPrimBarrel->nShotsPerClip;

				pStats->UpdateClip(m_pPrimBarrel->nId, nAmmoCount < nShotsPerClip ? nAmmoCount : nShotsPerClip);
			}

			UpdateGLHideStatus();
		}
	}
	else if (stricmp(pKey, WEAPON_KEY_RELOAD) == 0)
	{
		UpdateGLHideStatus(LTTRUE);
		UpdateExoHideStatus(LTTRUE);
	}
	else if (stricmp(pKey, WEAPON_KEY_UPDATE_HIDESTATUS) == 0)
	{
		UpdateExoHideStatus();
	}
	else if (stricmp(pKey, WEAPON_KEY_PRE_CHARGE) == 0)
	{
		// Send message to server
		HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pClientDE->WriteToMessageByte(hMessage, CP_CANNON_CHARGE);
		g_pClientDE->EndMessage(hMessage);
	}
	else if (stricmp(pKey, WEAPON_KEY_FORCE_NEXT_WEP) == 0)
	{
		//grab the next weapon and be sure not to play deselect anim
		uint8 nWepIndex = NextWeapon();

		ChangeWeapon(COMMAND_ID_WEAPON_BASE + nWepIndex, LTTRUE);
	}
	else if (stricmp(pKey, WEAPON_KEY_FORCE_LAST_WEP) == 0)
	{
		// Hack here
		m_eState = W_IDLE;	// need this to get past CanChangeTo code since sift and medi
							// are all select anims...
		LastWeapon(LTTRUE);
	}
	else if (stricmp(pKey, WEAPON_KEY_FORCE_DEFAULT_WEP) == 0)
	{
		ChangeWeapon(COMMAND_ID_WEAPON_BASE, LTTRUE);
	}
	else if (stricmp(pKey, WEAPON_KEY_ENERGY_SIFT) == 0)
	{
		// Send message to server
		HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pClientDE->WriteToMessageByte(hMessage, CP_ENERGY_SIFT);
		g_pClientDE->EndMessage(hMessage);
	}
	else if (stricmp(pKey, WEAPON_KEY_MEDICOMP) == 0)
	{
		// Send message to server
		HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pClientDE->WriteToMessageByte(hMessage, CP_MEDICOMP);
		g_pClientDE->EndMessage(hMessage);
	}
	else if (stricmp(pKey, WEAPON_KEY_ZOOM_START) == 0)
	{
		//set zoom level to 4x
		m_fCurZoomLevel = 2;
		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		CameraMgrFX_ZoomToggle(hCamera, m_fCurZoomLevel);
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Weapons\\Marine\\RailGun\\zoom.wav", SOUNDPRIORITY_PLAYER_HIGH);

		//make the crosshair invisible
		g_pInterfaceMgr->GetCrosshairMgr()->LoadCrosshair(m_pWeapon->szCrosshair, 0.0f, m_pWeapon->fCrosshairAlpha);

		//set overlay
		g_pGameClientShell->GetVisionModeMgr()->SetRailMode(LTTRUE);

		if(m_bPreZooming)
		{
			m_bPreZooming	= LTFALSE;
			m_bZooming		= LTTRUE;
		}
	}
	else if (stricmp(pKey, WEAPON_KEY_ZOOM_END) == 0)
	{
		//set crosshair to normal
		g_pInterfaceMgr->GetCrosshairMgr()->LoadCrosshair(m_pWeapon->szCrosshair, m_pWeapon->fCrosshairScale, m_pWeapon->fCrosshairAlpha);

		//set overlay
		g_pGameClientShell->GetVisionModeMgr()->SetRailMode(LTFALSE);
	}
	else if (stricmp(pKey, WEAPON_KEY_DETONATE) == 0)
	{
		//handle sending the detonate message to the server
		HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pClientDE->WriteToMessageByte(hMessage, CP_HOTBOMB_DETONATE);
		g_pClientDE->EndMessage(hMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateModelState
//
//	PURPOSE:	Update the model's state (fire if bFire == DTRUE)
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::UpdateModelState(LTBOOL bFire)
{
	WeaponState eRet = W_IDLE;

	// Determine what we should be doing...

	// Check ammo
	AmmoStatus as = StatusAmmo(m_pBarrel);
	
	if(bFire && as == AS_CLIP_EMPTY)
	{
		m_eState = W_RELOADING;
		bFire = LTFALSE;
	}
	else if(as == AS_OUT_OF_AMMO)
	{
		if(bFire)
		{
			bFire = LTFALSE;

			switch(AmmoCheck())
			{
			case (AS_OUT_OF_AMMO):
				{
					ChangeToNextBestWeapon();
				}
				break;
			case (AS_NEXT_BARREL):
				{
					NextPrimBarrel(m_pPrimBarrel);
					m_eState = W_CHANGE_AMMO;
					PlayAmmoChangeAnimation();

					if(stricmp(m_pPrimBarrel->szName, "SADAR_Barrel")==0 || stricmp(m_pPrimBarrel->szName, "SADAR_Barrel_MP")==0)
					{
						g_pInterfaceMgr->InitTargetingMgr(TT_MARINE_SADAR_NORMAL);
					}
					else
					{
						g_pInterfaceMgr->InitTargetingMgr(m_pWeapon->nTargetingType);
					}
				}
				break;
			}
		}
	}

	// Check weapon chooser status
	if(bFire && g_pGameClientShell->GetInterfaceMgr()->GetWeaponChooser())
		if(g_pGameClientShell->GetInterfaceMgr()->GetWeaponChooser()->IsOpen())
			bFire = LTFALSE;


	if (bFire) 
	{
		UpdateFiring();
	}
	else 
	{
		UpdateNonFiring();
	}


	if (m_bFire) 
	{
		eRet = Fire();
	}


	// See if we just finished deselecting the weapon...

	if (m_bWeaponDeselected)
	{
		m_bWeaponDeselected = LTFALSE;

		// Change weapons if we're not chaing between normal and
		// alt-fire modes...
		if (m_nRequestedWeaponId != m_nWeaponId && m_nRequestedWeaponId != WMGR_INVALID_ID)
		{
			HandleInternalWeaponChange(m_nRequestedWeaponId);
		}
	}


	// See if we should force a weapons change...

	if (m_eState == W_FIRING_NOAMMO)
	{
		//check to see if there really isnt any ammo
		if(AmmoCheck() == AS_OUT_OF_AMMO)
		{
			if(m_nEndFireAnim != INVALID_ANI)
			{
				PlayEndFireAnimation();
				m_eState = W_END_FIRING;
			}
			else
				ChangeToNextBestWeapon();
		}
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Fire
//
//	PURPOSE:	Handle firing the weapon
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::Fire(LTBOOL bUpdateAmmo)
{
	//sanity check
	if(!g_pGameClientShell || !m_pBarrel || !g_pInterface) return W_IDLE;
	
	WeaponState eRet = W_IDLE;

	//get player stats
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//test
	if (!pStats) return W_IDLE;

	//get infinite ammo boolean
	LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pBarrel->bInfiniteAmmo);

	//get ammo or infinite amount
	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : pStats->GetAmmoCount(m_nAmmoId);

	// If this weapon uses ammo, make sure we have ammo...
	if (nAmmo > 0)
	{
		eRet = W_FIRED;

		if (bUpdateAmmo)
		{
			DecrementAmmo();

			UpdateGLHideStatus();
		}
	} 
	else  // NO AMMO
	{
		m_eState = W_FIRING_NOAMMO;

		// Play dry-fire sound...

		if (m_pBarrel->szDryFireSound[0])
		{
			g_pClientSoundMgr->PlaySoundLocal(m_pBarrel->szDryFireSound, SOUNDPRIORITY_PLAYER_HIGH);
		}

		
		// Send message to Server so that other client's can hear this sound...

		uint32 dwId;
		g_pInterface->GetLocalClientID(&dwId);

		HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_SOUND);
		g_pInterface->WriteToMessageByte(hWrite, WMKS_DRY_FIRE);
		g_pInterface->WriteToMessageByte(hWrite, m_nWeaponId);
		g_pInterface->WriteToMessageByte(hWrite, (uint8)-1);
		g_pInterface->WriteToMessageByte(hWrite, (uint8)dwId);
		g_pInterface->WriteToMessageVector(hWrite, &m_vFlashPos);
		g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
	}

	m_bFire = LTFALSE;
	
	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DecrementAmmo
//
//	PURPOSE:	Decrement the weapon's ammo count
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DecrementAmmo()
{
	//get player stats pointer
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//test
	if (!pStats) 
		return;

	//see if the barrel uses ammo
	LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pBarrel->bInfiniteAmmo);

	//get the current amount of ammo in inventory
	//or use the defined infinite amount if applicable
//	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : pStats->GetAmmoCount(m_nAmmoId);
	if(!bInfiniteAmmo)
		pStats->DecrementAmmo(m_pAmmo->nId);

	int nShotsPerClip = m_pBarrel->nShotsPerClip;

	if (pStats->GetAmmoInClip(m_pBarrel->nId) > 0 || nShotsPerClip == 0)
	{
		if (nShotsPerClip > 0)
		{
			pStats->DecrementClip(m_pBarrel->nId); 
		}
	}

	// Check to see if we need to reload...

	if (nShotsPerClip > 0)
	{
		if (pStats->GetAmmoInClip(m_pBarrel->nId) <= 0) 
		{
			ReloadClip(DTRUE);//, nAmmo-nShotsPerClip-m_nAmmoInClip);
		}
	}

	if(AmmoCheck() == AS_OUT_OF_AMMO)
	{
		if(stricmp(m_pWeapon->szName, "Disc")!=0 && stricmp(m_pWeapon->szName, "Disc_MP")!=0)
		{
			if (stricmp(m_pWeapon->szName, "StickyHotbomb") == 0 || stricmp(m_pWeapon->szName, "StickyHotbomb_MP") == 0)
			{
				//handle sending the detonate message to the server
				if(StatusAmmo(m_pBarrel) == AS_OUT_OF_AMMO)
				{
					//get the model interface
					ModelLT* pModelLT   = g_pLTClient->GetModelLT();

					if(!pModelLT) return;

					HMODELPIECE hPiece;
					pModelLT->GetPiece(m_hObject, "Bomb", hPiece);
					pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);
				}
			}

			m_eState = W_IDLE;
			ChangeToNextBestWeapon();
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ReloadClip
//
//	PURPOSE:	Fill the clip
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ReloadClip(LTBOOL bPlayReload, int nNewAmmo, LTBOOL bForce)
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!g_pInterfaceMgr || !pStats || !m_pPrimBarrel) return;

	// See if we are pointed to the alt barrel
	if(m_pBarrel && (m_pBarrel->eBarrelType != BT_CHARGE) && (m_pBarrel != m_pPrimBarrel))
	{
		// re-set the barrel info to point to the primary barrel
		m_pBarrel = m_pPrimBarrel;
		CreateFlash();

		//set id
		m_nBarrelId = m_pBarrel->nId;
		
		//set ammo pointer
		m_pAmmo	= g_pWeaponMgr->GetAmmo(m_pBarrel->nAmmoType);
		
		//set ammoId and type if we have ammo for this barrel
		if(m_pAmmo)
		{
			m_nAmmoId = m_pAmmo->nId;
			
			m_eAmmoType	= m_pAmmo->eType;
		}

		m_eLastFireType	= FT_NORMAL_FIRE;
	}

	int nAmmoCount = pStats->GetAmmoCount(m_nAmmoId);
	int nAmmo = nNewAmmo >= 0 ? nNewAmmo : nAmmoCount;
	int nShotsPerClip = m_pPrimBarrel->nShotsPerClip;

	// Make sure we can reload the clip...

	if (!bForce)
	{
		// Already reloading...

		if (m_hObject && (m_eState == W_RELOADING))
		{
			return;
		}

		// Clip is full...

		if (pStats->GetAmmoInClip(m_pPrimBarrel->nId) == nShotsPerClip || pStats->GetAmmoInClip(m_pPrimBarrel->nId)  == nAmmoCount)
		{
			return;
		}
	}

	if (nAmmo > 0 && nShotsPerClip > 0)
	{
		if (bPlayReload && GetReloadAni() != INVALID_ANI)
		{
			m_eState = W_RELOADING;
			return;
		}
		else
		{
			pStats->UpdateClip(m_pPrimBarrel->nId, nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateFiring
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateFiring()
{
	m_bCanSetLastFire = DTRUE;
	
	if (m_eState == W_CHARGING)
	{
		if(!PlayChargeAnimation(LTTRUE, LTTRUE))
			m_eState = W_IDLE;
	}

	if(m_eState == W_SPINDOWN)
	{
		if(!PlayEndSpinupAnimation(LTFALSE))
			m_eState = W_IDLE;
	}

	if (m_eState == W_FIRING_DETONATE)
	{
		if(!PlayDetonateAnimation())
		{
			m_eState = W_START_FIRING;
		}
	}

	if (m_eState == W_END_FIRING)
	{
		if(!PlayEndFireAnimation())
		{
			m_eState = W_START_FIRING;
		}
	}

	if (m_eState == W_START_FIRING)
	{
		if(!PlayStartFireAnimation())
		{
			m_eState = W_FIRING_SPECIAL;
		}
	}

	if (m_eState == W_BARREL_SWITCH)
	{
//		if(g_pLTClient->GetTime() - m_fBarrelSwitchStartTime >= g_vtAmmoSwitchDelay.GetFloat() ||
//			(stricmp(m_pWeapon->szName, "Blowtorch") == 0))
		if(g_pLTClient->GetTime() - m_fBarrelSwitchStartTime >= g_vtAmmoSwitchDelay)// ||(stricmp(m_pWeapon->szName, "Blowtorch") == 0))
		{
			// Only switch if the barrel is really a new one...
			if(m_pOldBarrel != m_pPrimBarrel)
			{
				m_eState = W_CHANGE_AMMO;
				PlayAmmoChangeAnimation();
			}
			else
				m_eState = W_IDLE;
		}
	}

	if (m_eState == W_RELOADING)
	{
		if (!PlayReloadAnimation())
		{
			m_eState = W_FIRING;
		}
	}
	if (m_eState == W_CHANGE_AMMO)
	{
		if (!PlayAmmoChangeAnimation())
		{
			if(stricmp(m_pWeapon->szName, "Blowtorch") == 0)
			{
				m_eState = W_START_FIRING;
				m_eLastFireType = FT_NORMAL_FIRE;
			}
			else
				m_eState = W_FIRING;
		}
	}

	if (m_eState == W_IDLE && m_pBarrel)
	{
		m_eState = W_FIRING;
	}
	if (m_eState == W_SELECT)
	{
		if(!PlaySelectAnimation())
			m_eState = W_FIRING;
	}
	if (m_eState == W_DESELECT)
	{
		if (!PlayDeselectAnimation())
		{
			m_bWeaponDeselected = DTRUE;
//			m_eState = W_FIRING;
		}
	}
	if (m_eState == W_CHARGE_FIRING)
	{
		if (PlayChargeFireAnimation())
			m_bCanSetLastFire = LTFALSE;
	}
	if (m_eState == W_ZOOM_STARTING)
	{
		if (PlayStartZoomAnimation(LTFALSE))
			m_bCanSetLastFire = LTFALSE;
		else
			m_eState = W_FIRING;
	}
	if (m_eState == W_ZOOM_ENDING)
	{
		if (PlayEndZoomAnimation(LTFALSE))
			m_bCanSetLastFire = LTFALSE;
		else
			m_eState = W_FIRING;
	}

	if(m_eState == W_SPINUP)
	{
		if(!PlayStartSpinupAnimation())
			m_eState = W_SPINNING;
	}

//	if(m_eState == W_SPINDOWN)
//	{
//		if(!PlayEndSpinupAnimation())
//			m_eState = W_FIRING;
//	}
	if (m_eState == W_FIRING_SPECIAL)
	{
//		if (
			PlayFireAnimation();
//			)
//			m_bCanSetLastFire = LTFALSE;
	}

	if (m_eState == W_FIRING || m_eState == W_FIRING_NOAMMO ||  m_eState == W_SPINNING)
	{
		if(m_pBarrel)
		{
			switch(m_pBarrel->eBarrelType)
			{
			case(BT_NORMAL):
			case(BT_SWITCHING):
			case(BT_ALIEN_SPECIAL):
			case(BT_TARGETING_TOGGLE):
			case(BT_MINIGUN_SPECIAL):
			case(BT_ZOOMING):
			case(BT_PRED_DETONATOR):
			case(BT_PRED_STICKYBOMB):
				{
					if (m_nStartFireAnim != INVALID_ANI && m_pBarrel == m_pPrimBarrel)
					{
						//get the current animation and playing state
						uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
						uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

						if(!IsFireAni(dwAni) && m_eState != W_SPINNING)
						{
							if(!PlayStartFireAnimation())
								PlayFireAnimation();
							else
								m_eState = W_START_FIRING;
						}
						else
						{
							if(m_eState == W_SPINNING)
							{
								m_eLastFireType	= FT_NORMAL_FIRE;
								m_eState = W_FIRING;
							}
							if(PlayFireAnimation())
								m_bCanSetLastFire = LTFALSE;
						}
					}
					else
					{
						if (PlayFireAnimation())
							m_bCanSetLastFire = LTFALSE;
					}
				}
				break;
				
			case (BT_CHARGE):
				{
					if(IsAlien(g_pGameClientShell->GetPlayerStats()->GetButeSet()))
					{
						if(PlayChargeAnimation(LTTRUE, LTTRUE))
						{
							m_eState = W_CHARGING;
							m_bCanSetLastFire = LTFALSE;
						}
					}
					else
					{
						if(PlayChargeAnimation())
							m_bCanSetLastFire = LTFALSE;
					}
				}
				break;
				
			case (BT_PRE_CHARGE):
				{
					if(PlayPreChargeAnimation())
						m_bCanSetLastFire = LTFALSE;
				}
				break;

			case (BT_EXO_LAZER):
				{
					if (PlayFireAnimation())
						m_bCanSetLastFire = LTFALSE;
				}
				break;

			default:
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateNonFiring
//
//	PURPOSE:	Update the non-firing animation state of the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateNonFiring()
{
	m_bCanSetLastFire = DTRUE;

	if (m_eState == W_CHARGING)
	{
		// see if the alt-fire button is not down.
		uint32 dwFlags = 	g_pGameClientShell->GetPlayerMovement()->GetControlFlags();

		if (!(dwFlags & CM_FLAG_ALTFIRING))
		{
			PlayChargeFireAnimation();
			m_eState = W_CHARGE_FIRING;
			return;
		}

		if(!PlayChargeAnimation(LTTRUE, LTTRUE))
			m_eState = W_IDLE;
	}

	if (m_eState == W_FIRING_DETONATE)
	{
		if(!PlayDetonateAnimation(LTFALSE))
		{
			m_eState = W_IDLE;
		}
		else
		{
			m_bCanSetLastFire = LTFALSE;
		}
	}

	if (m_eState == W_END_FIRING)
	{
		if(!PlayEndFireAnimation(LTFALSE))
		{
			m_eState = W_IDLE;
		}
		else
		{
			m_bCanSetLastFire = LTFALSE;
		}
	}

	if (m_eState == W_START_FIRING)
	{
		if(!PlayStartFireAnimation(LTFALSE))
		{
			PlayEndFireAnimation();
			m_eState = W_END_FIRING;
		}
		else
		{
			m_bCanSetLastFire = LTFALSE;
		}
	}
	if (m_eState == W_FIRING_SPECIAL)
	{
		PlayEndFireAnimation();
		m_eState = W_END_FIRING;
	}

	if (m_eState == W_FIRING || m_eState == W_FIRING_SPECIAL)
	{

		if(m_pBarrel)
		{
			if(m_nStartFireAnim != INVALID_ANI && m_pBarrel == m_pPrimBarrel && m_pBarrel->eBarrelType != BT_EXO_LAZER)
			{
				if (!PlayStartFireAnimation(LTFALSE))
				{
					PlayEndFireAnimation();
					m_eState = W_END_FIRING;
				}
				else
				{
					m_bCanSetLastFire = LTFALSE;
				}
			}
			else
			{
				if (!m_pBarrel || !PlayFireAnimation(LTFALSE))
				{
					m_eState = W_IDLE;
				}
				else
				{
					m_bCanSetLastFire = LTFALSE;
				}
			}
		}
	}
	if (m_eState == W_CHARGE_FIRING)
	{
		if (!m_pBarrel || !PlayChargeFireAnimation(LTFALSE))
		{
			m_eState = W_IDLE;
		}
		else
		{
			m_bCanSetLastFire = LTFALSE;
		}
	}
	if (m_eState == W_FIRING_NOAMMO)
	{
		m_eState = W_IDLE;
	}
	if (m_eState == W_RELOADING)
	{
		if (!PlayReloadAnimation())
		{
			m_eState = W_IDLE;
		}
	}
	if (m_eState == W_CHANGE_AMMO)
	{
		if (!PlayAmmoChangeAnimation())
		{
			m_eState = W_IDLE;
		}
	}
	if (m_eState == W_SELECT)
	{
		if (!PlaySelectAnimation())
		{
			m_eState = W_IDLE;
		}
	}

	if (m_eState == W_DESELECT)
	{
		if (!PlayDeselectAnimation())
		{
			m_bWeaponDeselected = DTRUE;
		}
	}

	if (m_eState == W_IDLE)
	{
		// make sure we don't get our tail stuck...
		if(m_bAlienTail && m_pBarrel && m_pBarrel->eBarrelType == BT_CHARGE)
		{
			// see if the alt-fire button is not down.
			uint32 dwFlags = 	g_pGameClientShell->GetPlayerMovement()->GetControlFlags();

			if (!(dwFlags & CM_FLAG_ALTFIRING))
			{
				PlayChargeFireAnimation();
				m_eState = W_CHARGE_FIRING;
				return;
			}
		}

		PlayIdleAnimation();
	}

	if (m_eState == W_BARREL_SWITCH)
	{
		if(g_pLTClient->GetTime() - m_fBarrelSwitchStartTime >= g_vtAmmoSwitchDelay)// || (stricmp(m_pWeapon->szName, "Blowtorch") == 0))
		{
			// Only switch if the barrel is really a new one...
			if(m_pOldBarrel != m_pPrimBarrel)
			{
				m_eState = W_CHANGE_AMMO;
				PlayAmmoChangeAnimation();
			}
			else
				m_eState = W_IDLE;
		}
	}
	if(m_eState == W_SPINUP)
	{
		if(!PlayStartSpinupAnimation(LTFALSE))
			m_eState = W_SPINNING;
	}

	if(m_eState == W_SPINNING)
	{
		PlaySpinupAnimation();
	}

	if(m_eState == W_SPINDOWN)
	{
		if(!PlayEndSpinupAnimation(LTFALSE))
			m_eState = W_IDLE;
	}

	if(m_eState == W_ZOOM_STARTING)
	{
		if(!PlayStartZoomAnimation())
			m_eState = W_IDLE;
	}
	if(m_eState == W_ZOOM_ENDING)
	{
		if(!PlayEndZoomAnimation())
			m_eState = W_IDLE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlaySelectAnimation()
//
//	PURPOSE:	Set model to select animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlaySelectAnimation()
{
	uint32 dwSelectAni = GetSelectAni();

	if (!m_hObject || dwSelectAni == INVALID_ANI) return LTFALSE;

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bIsSelectAni = IsSelectAni(dwAni);
	if (bIsSelectAni && (dwState & MS_PLAYDONE))
	{
		if(m_pBarrel && m_pBarrel->eBarrelType==BT_ALIEN_SPECIAL)// || m_pBarrel->eBarrelType==BT_CHARGE)
		{
			PlayChargeAnimation(LTTRUE, LTTRUE);
//				m_eState = W_CHARGING;
		}
		return LTFALSE;  
	}
	else if (!bIsSelectAni)
	{
		// Send message to server
/*		HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
		g_pLTClient->WriteToMessageByte(hMessage, WS_SELECT);
		g_pLTClient->EndMessage(hMessage);
*/

		g_pInterface->SetModelLooping(m_hObject, LTFALSE);
		g_pInterface->SetModelAnimation(m_hObject, dwSelectAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayDeselectAnimation()
//
//	PURPOSE:	Set model to deselect animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayDeselectAnimation()
{
	uint32 dwDeselectAni = GetDeselectAni();

	if (!m_hObject || dwDeselectAni == INVALID_ANI) return LTFALSE;

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bIsDeselectAni = IsDeselectAni(dwAni);

	if (bIsDeselectAni && (dwState & MS_PLAYDONE))
	{
		return LTFALSE;  
	}
	else if (!bIsDeselectAni)
	{
		// Send message to server
/*		HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
		g_pLTClient->WriteToMessageByte(hMessage, WS_DESELECT);
		g_pLTClient->EndMessage(hMessage);
*/

		g_pInterface->SetModelLooping(m_hObject, LTFALSE);
		g_pInterface->SetModelAnimation(m_hObject, dwDeselectAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayFireAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	// Can only set the last fire type if a fire animation isn't playing 
	// (i.e., we'll assume this function will return false)...

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bIsFireAni = IsFireAni(dwAni);

	if (!bIsFireAni || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			uint32 dwFireAni = GetFireAni(m_eLastFireType);
			if (dwFireAni == INVALID_ANI) return LTFALSE;

			LTBOOL bLooping = m_pBarrel == m_pPrimBarrel ? m_pWeapon->bLoopFireAnim : m_pWeapon->bLoopAltFireAnim;
			g_pInterface->SetModelLooping(m_hObject, bLooping);
			g_pInterface->SetModelAnimation(m_hObject, dwFireAni);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning

			// If this is a melee weapon... send up the melee weapon state on the server
			if(m_pWeapon->bMelee)
			{
				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
				g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
				g_pLTClient->WriteToMessageByte(hMessage, WS_MELEE);
				g_pLTClient->EndMessage(hMessage);
			}
		}

		if (bIsFireAni && (dwState & MS_PLAYDONE))
		{
			m_fNextIdleTime	= GetNextIdleTime();
			return LTFALSE;
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayChargeFireAnimation()
//
//	PURPOSE:	Set model to firing animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayChargeFireAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	// Can only set the last fire type if a fire animation isn't playing 
	// (i.e., we'll assume this function will return false)...

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bIsChargeFireAni = IsChargeFireAni(dwAni);

	if (!bIsChargeFireAni || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			if (m_nChargeFireAni == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, m_pWeapon->bLoopFireAnim);
			g_pInterface->SetModelAnimation(m_hObject, m_nChargeFireAni);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}

		if (bIsChargeFireAni && (dwState & MS_PLAYDONE))
		{
			m_fNextIdleTime	= GetNextIdleTime();
			return LTFALSE;
		}
	}

	return DTRUE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayStartZoomAnimation()
//
//	PURPOSE:	Set model to start zoom animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayStartZoomAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bIsStartZoomAni = IsStartZoomAni(dwAni);

	if (!bIsStartZoomAni || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			if (m_nStartZoomAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nStartZoomAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}

		if (bIsStartZoomAni && (dwState & MS_PLAYDONE))
		{
			m_fNextIdleTime	= GetNextIdleTime();
			return LTFALSE;
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayEndZoomAnimation()
//
//	PURPOSE:	Set model to end zoom animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayEndZoomAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	
	//dont start the animation until the zoom is done
	if(CameraMgrFX_Active(hCamera, CAMERAMGRFX_ID_ZOOM))
		return LTTRUE;

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bIsEndZoomAni = IsEndZoomAni(dwAni);

	if (!bIsEndZoomAni || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			if (m_nEndZoomAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nEndZoomAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}

		if (bIsEndZoomAni && (dwState & MS_PLAYDONE))
		{
			m_fNextIdleTime	= GetNextIdleTime();
			return LTFALSE;
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayChargeAnimation()
//
//	PURPOSE:	Set model to charging animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayChargeAnimation(LTBOOL bResetAni, LTBOOL bLooping)
{
	if (!m_hObject) return LTFALSE;

	if(m_eState == W_DESELECT) 
		return LTFALSE;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	//test to see that we are not already playing a charging
	//animation and that the last animation is completed
	if (!IsChargeAni(dwAni) || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			//if we have a charging animation, play it
			if (m_nChargingAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, bLooping);
			g_pInterface->SetModelAnimation(m_hObject, m_nChargingAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayDetonateAnimation()
//
//	PURPOSE:	Set model to detonate animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayDetonateAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);
	LTBOOL bIsDetAni = IsDetonateAni(dwAni);

	//test to see that we are not already playing a detonate
	//animation and that the last animation is completed
	if (!bIsDetAni || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			//if we have a charging animation, play it
			if (m_nDetonateAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nDetonateAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}
		else if (bIsDetAni && (dwState & MS_PLAYDONE))
		{
			return LTFALSE;
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayPreChargeAnimation()
//
//	PURPOSE:	Set model to charging animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayPreChargeAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	if(m_eState == W_DESELECT) 
		return LTFALSE;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	//test to see that we are not already playing a charging
	//animation and that the last animation is completed
	if (!IsPreChargeAni(dwAni) || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			//if we have a charging animation, play it
			if (m_nPreChargingAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nPreChargingAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlaySpinupAnimation()
//
//	PURPOSE:	Set model to charging animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlaySpinupAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	if(IsEndSpinupAni(dwAni) && (dwState & MS_PLAYDONE))
	{
		//go back to idle
		m_eState = W_IDLE;
		return DTRUE;
	}

	//test to see that we are not already playing a charging
	//animation and that the last animation is completed
	if (!IsSpinupAni(dwAni))
	{
		if (bResetAni)
		{
			//if we have a charging animation, play it
			if (m_nSpinupAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, DTRUE);
			g_pInterface->SetModelAnimation(m_hObject, m_nSpinupAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlaySpinupAnimation()
//
//	PURPOSE:	Set model to charging animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayStartSpinupAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bIsStartSpinup = IsStartSpinupAni(dwAni);

	//test to see that we are not already playing a charging
	//animation and that the last animation is completed
	if (!bIsStartSpinup || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			//if we have a charging animation, play it
			if (m_nSpinupStartAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nSpinupStartAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}

		if (bIsStartSpinup && (dwState & MS_PLAYDONE))
		{
			m_fNextIdleTime	= GetNextIdleTime();
			return LTFALSE;
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayStartFireAnimation()
//
//	PURPOSE:	Set model to start fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayStartFireAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	// Only primary barrels support start fire anims..  Sorry...
	if(m_pBarrel != m_pPrimBarrel) return LTFALSE;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);
	LTBOOL bIsAnyFireAnim = (IsStartFireAni(dwAni) | IsEndFireAni(dwAni) | IsFireAni(dwAni) | IsReloadAni(dwAni));
	LTBOOL bIsStartFireAnim		= IsStartFireAni(dwAni);
	LTBOOL bIsEndFireAnim		= IsEndFireAni(dwAni);
	LTBOOL bIsFireAnim			= IsFireAni(dwAni);

	//test to see that we are not already playing a start fire
	//animation and that the last animation is completed
	if (!bIsAnyFireAnim || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			//if we have a start fire animation, play it
			if (m_nStartFireAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nStartFireAnim);
			g_pInterface->ResetModelAnimation(m_hObject);  // Start from beginning
		}

		if (bIsStartFireAnim && (dwState & MS_PLAYDONE))
		{
			return LTFALSE;
		}

//		if (!bIsFireAnim && (dwState & MS_PLAYDONE))
//		{
//			PlayFireAnimation();
//			return LTFALSE;
//		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayEndSpinupAnimation()
//
//	PURPOSE:	Set model to end fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayEndSpinupAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	uint32		nAnimTime	= 0;
	LTFLOAT		fRatio		= 1.0f;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTAnimTracker* pTracker;
	g_pModelLT->GetMainTracker(m_hObject, pTracker);

	//see if this is a begin fire anim
	if(IsStartSpinupAni(dwAni))
	{
		uint32 curTime;
		uint32 totTime;
		//find out how far into it we are
		g_pModelLT->GetCurAnimTime(pTracker, curTime);
		g_pModelLT->GetCurAnimLength(pTracker, totTime);

		//set ratio
		fRatio = (LTFLOAT)curTime/totTime;
	}

	LTBOOL bIsEndSpinAnim = IsEndSpinupAni(dwAni);

	//test to see that we are not already playing a end fire
	//animation and that the last animation is completed
	if (!bIsEndSpinAnim || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			//if we have a start fire animation, play it
			if (m_nEndFireAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nSpinupEndAnim);

			LTAnimTracker* pTracker;
			g_pModelLT->GetMainTracker(m_hObject, pTracker);

			uint32 totTime;
			g_pModelLT->GetCurAnimLength(pTracker, totTime);

			nAnimTime = (uint32)((1-fRatio) * totTime);

			g_pModelLT->SetCurAnimTime(pTracker, nAnimTime);
		}

		if (bIsEndSpinAnim && (dwState & MS_PLAYDONE))
		{
			m_fNextIdleTime	= GetNextIdleTime();
			return LTFALSE;
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayEndFireAnimation()
//
//	PURPOSE:	Set model to end fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayEndFireAnimation(LTBOOL bResetAni)
{
	if (!m_hObject) return LTFALSE;

	// Only primary barrels support end fire anims..  Sorry...
	if(m_pBarrel != m_pPrimBarrel) return LTFALSE;

	uint32		nAnimTime	= 0;
	LTFLOAT		fRatio		= 1.0f;

	//get the current animation and playing state
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	// Don't endfire if we are reloading...
	if(IsReloadAni(dwAni)) return LTFALSE;

	LTAnimTracker* pTracker;
	g_pModelLT->GetMainTracker(m_hObject, pTracker);

	//see if this is a begin fire anim
	if(IsStartFireAni(dwAni))
	{
		uint32 curTime;
		uint32 totTime;
		//find out how far into it we are
		g_pModelLT->GetCurAnimTime(pTracker, curTime);
		g_pModelLT->GetCurAnimLength(pTracker, totTime);

		//set ratio
		fRatio = (LTFLOAT)curTime/totTime;
	}

	//test to see that we are not already playing a end fire
	//animation and that the last animation is completed
	if (!IsEndFireAni(dwAni) || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
			//if we have a start fire animation, play it
			if (m_nEndFireAnim == INVALID_ANI) return LTFALSE;

			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, m_nEndFireAnim);

			LTAnimTracker* pTracker;
			g_pModelLT->GetMainTracker(m_hObject, pTracker);

			uint32 totTime;
			g_pModelLT->GetCurAnimLength(pTracker, totTime);

			nAnimTime = (uint32)((1-fRatio) * totTime);

			g_pModelLT->SetCurAnimTime(pTracker, nAnimTime);
		}
		if(IsEndFireAni(dwAni) && (dwState & MS_PLAYDONE))
			return LTFALSE;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::OnFireButtonUp()
//
//	PURPOSE:	Handle the release of the fire button
//
// ----------------------------------------------------------------------- //

void CWeaponModel::OnFireButtonUp()
{
	if (!m_hObject || !m_pBarrel || m_pBarrel != m_pPrimBarrel) return;

	CWeaponChooser*	pChooser = g_pGameClientShell->GetInterfaceMgr()->GetWeaponChooser();
	if(pChooser && pChooser->IsOpen())
		return;

	if(g_pGameClientShell->IsCinematic())
		return;

	//set fire animation
	if(m_pBarrel->eBarrelType == BT_CHARGE && m_eState != W_SELECT && m_eState != W_DESELECT )
	{
		m_eLastFireType = FT_NORMAL_FIRE;
		PlayFireAnimation();
	}

	if(m_pWeapon->bLoopFireAnim && m_pBarrel == m_pPrimBarrel && m_eState != W_RELOADING &&  m_eState != W_SPINUP)
	{
		if(m_nEndFireAnim != INVALID_ANI)
		{
			if(m_eState != W_IDLE && (m_eState != W_CHANGE_AMMO) && m_pBarrel->eBarrelType != BT_EXO_LAZER && m_eState != W_DESELECT && m_eState != W_SELECT)
			{
				m_bCanSetLastFire = LTFALSE;
				if(m_bAutoSpinOn)
				{
					PlaySpinupAnimation();
					m_eState = W_SPINNING;
				}
				else
				{
					PlayEndFireAnimation();
					m_eState = W_END_FIRING;
				}
			}
		}
		else
		{
			m_bCanSetLastFire = LTFALSE;
		}
	}

	if (m_eState == W_BARREL_SWITCH)
	{
		// Only switch if the barrel is really a new one...
		if(m_pOldBarrel != m_pPrimBarrel)
		{
			m_eState = W_CHANGE_AMMO;
			PlayAmmoChangeAnimation();
		}
		else
			m_eState = W_IDLE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::OnAltFireButtonDown()
//
//	PURPOSE:	Handle the press of the alt fire button
//
// ----------------------------------------------------------------------- //

void CWeaponModel::OnAltFireButtonDown()
{
	CWeaponChooser*	pChooser = g_pGameClientShell->GetInterfaceMgr()->GetWeaponChooser();
	if(pChooser && pChooser->IsOpen())
		return;

	if(g_pGameClientShell->IsCinematic())
		return;

	if(m_pPrimBarrel && m_pPrimBarrel->eBarrelType == BT_ALIEN_SPECIAL)
	{
		//get the weapon set ID
		int nWepSet = g_pGameClientShell->GetPlayerStats()->GetWeaponSet();

		int nWepIndex = g_pWeaponMgr->GetWepSetIndex("Tail", nWepSet);

		if(nWepIndex == WMGR_INVALID_ID)
			nWepIndex = g_pWeaponMgr->GetWepSetIndex("Queen_Tail", nWepSet);

		if(nWepIndex != WMGR_INVALID_ID)
		{
			ChangeWeapon(COMMAND_ID_WEAPON_BASE + nWepIndex, LTTRUE);
		}

		return;
	}

	if(m_pPrimBarrel && m_pPrimBarrel->eBarrelType == BT_ZOOMING)
	{
		if(m_eState == W_IDLE || m_eState == W_FIRING ||  m_eState == W_ZOOM_ENDING )
		{
			//if we are not zoomed, call the zoom animation
			if(!m_bZoomed)
			{
				PlayStartZoomAnimation();
				g_pGameClientShell->GetVisionModeMgr()->SetRailMode(LTFALSE);
				g_pInterfaceMgr->GetCrosshairMgr()->LoadCrosshair(m_pWeapon->szCrosshair, m_pWeapon->fCrosshairScale);
				m_eState	= W_ZOOM_STARTING;
				m_bZoomed		= LTTRUE;
				m_bPreZooming	= LTTRUE;
			}
			else
			{
				//we are zoomed, call the unzoom animation
				PlayEndZoomAnimation();
				m_eState = W_ZOOM_ENDING;
				m_fCurZoomLevel = 1;
				HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
				CameraMgrFX_ZoomToggle(hCamera, m_fCurZoomLevel);
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Weapons\\Marine\\RailGun\\zoom.wav", SOUNDPRIORITY_PLAYER_HIGH);
				m_bZoomed		= LTFALSE;
				m_bZooming		= LTFALSE;
				m_bPreZooming	= LTFALSE;
			}
		}
		return;
	}

	//see if we are the hotbomb
	if(m_pWeapon && (stricmp(m_pWeapon->szName, "StickyHotbomb")==0 || stricmp(m_pWeapon->szName, "StickyHotbomb_MP")==0) )
	{
		PlayDetonateAnimation();
		m_eState = W_FIRING_DETONATE;
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::OnAltFireButtonUp()
//
//	PURPOSE:	Handle the release of the alt fire button
//
// ----------------------------------------------------------------------- //

void CWeaponModel::OnAltFireButtonUp()
{
	if(!g_pWeaponMgr || !m_hObject || !m_pWeapon || !m_pBarrel) 
		return;

//	CWeaponChooser*	pChooser = g_pGameClientShell->GetInterfaceMgr()->GetWeaponChooser();
//	if(pChooser && pChooser->IsOpen())
//		return;

	if(g_pGameClientShell->IsCinematic())
		return;

	LTBOOL bIsTorch = (stricmp(m_pWeapon->szName, "Blowtorch")==0);
	LTBOOL bIsMP = g_pGameClientShell->IsMultiplayerGame();

	// Cheever test
	if(bIsTorch && !bIsMP)
		return;

	if(m_pBarrel->eBarrelType == BT_SWITCHING && (m_eState == W_IDLE || m_eState == W_BARREL_SWITCH) )//&& m_eState != W_CHANGE_AMMO && m_eState != W_SELECT || m_eState != W_FIRING)
	{
		// if we are not already switching, record the current barrel...
		if(m_eState != W_BARREL_SWITCH)
		{
			m_pOldBarrel = m_pPrimBarrel;
		}

		if((m_eState != W_CHANGE_AMMO) && NextPrimBarrel(m_pPrimBarrel))
		{
			//go ahead and change the ammo if this is the blowtorch
			if(bIsTorch)
			{
				m_eState = W_CHANGE_AMMO;
				PlayAmmoChangeAnimation();
				return;
			}

			//play the interface sound
			g_pClientSoundMgr->PlaySoundLocal("MarineInterfaceAltFireScroll");

			m_fBarrelSwitchStartTime = g_pLTClient->GetTime();
			m_eState = W_BARREL_SWITCH;

			if(stricmp(m_pPrimBarrel->szName, "SADAR_Barrel")==0 || stricmp(m_pPrimBarrel->szName, "SADAR_Barrel_MP")==0)
			{
				g_pInterfaceMgr->InitTargetingMgr(TT_MARINE_SADAR_NORMAL);
			}
			else
			{
				g_pInterfaceMgr->InitTargetingMgr(m_pWeapon->nTargetingType);
			}

			if(StatusAmmo(m_pPrimBarrel) == AS_CLIP_EMPTY)
			{
				CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
				int nAmmoCount = pStats->GetAmmoCount(m_pPrimBarrel->nAmmoType);
				int nShotsPerClip = m_pPrimBarrel->nShotsPerClip;

				pStats->UpdateClip(m_pPrimBarrel->nId, nAmmoCount < nShotsPerClip ? nAmmoCount : nShotsPerClip);
			}
		}
		return;
	}

	//set fire animation
	if(m_pBarrel->eBarrelType == BT_CHARGE && (m_eState != W_DESELECT && !m_bWeaponDeselected))
	{
		m_eLastFireType = FT_NORMAL_FIRE;
		if(PlayChargeFireAnimation())
			m_eState = W_CHARGE_FIRING;
		else if(m_eState != W_FIRING)
			m_eState = W_IDLE;

		return;
	}

	//set fire animation
	if(m_pBarrel->eBarrelType == BT_PRE_CHARGE && (m_eState != W_DESELECT && !m_bWeaponDeselected))
	{
		m_eState = W_IDLE;
		return;
	}

	//toggle off auto-targeting
	if(m_pBarrel->eBarrelType == BT_TARGETING_TOGGLE)
	{
		CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

		if(pStats)
			pStats->ToggleAutoTargeting();

		return;
	}

	// play the spin up animation
	if((m_pBarrel->eBarrelType == BT_MINIGUN_SPECIAL) && ((m_eState == W_IDLE) || (m_eState == W_SPINNING) || (m_eState == W_SPINUP)
		 || (m_eState == W_SPINDOWN)))
	{
		//get the current animation and playing state
		uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);

		//if this is a start-spinup or spin-up animation
		//then play end-spinup
		if(IsStartSpinupAni(dwAni) || IsSpinupAni(dwAni))
		{
			PlayEndSpinupAnimation();
			m_eState = W_SPINDOWN;
		}
		else
		{
			//else play the spin-up animation
			PlayStartSpinupAnimation();
			m_eState = W_SPINUP;
		}

		// Toggle the toggle
		m_bAutoSpinOn = !m_bAutoSpinOn;

		return;
	}
	if(m_pPrimBarrel && m_pPrimBarrel->eBarrelType == BT_ZOOMING)
	{
		m_bZooming		= LTFALSE;
		m_bPreZooming	= LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayReloadAnimation()
//
//	PURPOSE:	Set model to reloading animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayReloadAnimation()
{
	uint32 dwReloadAni = GetReloadAni();

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	if (!m_hObject || !pStats || !m_pBarrel || !g_pInterface || dwReloadAni == INVALID_ANI) return LTFALSE;

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bCanPlay  = (!IsFireAni(dwAni) || g_pInterface->GetModelLooping(m_hObject) || (dwState & MS_PLAYDONE));

	LTBOOL bIsReloadAni = IsReloadAni(dwAni);

	if (bIsReloadAni && (dwState & MS_PLAYDONE))
	{
		// Set ammo in clip amount...
		int nAmmoCount = pStats->GetAmmoCount(m_nAmmoId);
		int nShotsPerClip = m_pBarrel->nShotsPerClip;

		pStats->UpdateClip(m_pBarrel->nId, nAmmoCount < nShotsPerClip ? nAmmoCount : nShotsPerClip);

		return LTFALSE;
	}
	else if (!bIsReloadAni && bCanPlay)
	{
		// Send message to server
		HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
		g_pLTClient->WriteToMessageByte(hMessage, WS_RELOADING);
		g_pLTClient->EndMessage(hMessage);


		g_pInterface->SetModelLooping(m_hObject, LTFALSE);
		g_pInterface->SetModelAnimation(m_hObject, dwReloadAni);
	}

	return DTRUE;  // Animation playing
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayAmmoChangeAnimation()
//
//	PURPOSE:	Set model to ammo change animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayAmmoChangeAnimation()
{
	uint32 dwAmmoChangeAni = GetAmmoChangeAni();

	if (!m_hObject || !m_pBarrel || !g_pInterface || dwAmmoChangeAni == INVALID_ANI) return LTFALSE;

	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);

	LTBOOL bCanPlay  = (!IsFireAni(dwAni) || g_pInterface->GetModelLooping(m_hObject) || (dwState & MS_PLAYDONE));

	LTBOOL bIsAmmoChangeAni = IsAmmoChangeAni(dwAni);

	if (bIsAmmoChangeAni && (dwState & MS_PLAYDONE))
	{
		return LTFALSE;
	}

	if (!bIsAmmoChangeAni && bCanPlay)
	{
		// Send message to server
		HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
		g_pLTClient->WriteToMessageByte(hMessage, WS_RELOADING);
		g_pLTClient->EndMessage(hMessage);


		g_pInterface->SetModelLooping(m_hObject, LTFALSE);
		g_pInterface->SetModelAnimation(m_hObject, dwAmmoChangeAni);
	}

	return DTRUE;  // Animation playing
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayIdleAnimation()
//
//	PURPOSE:	Set model to Idle animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayIdleAnimation()
{
	// Make sure idle animation is done if one is currently playing...

	uint32 dwAni = g_pInterface->GetModelAnimation(m_hObject);
	if (IsIdleAni(dwAni))
	{ 
		if (!(g_pInterface->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))
		{
			return DTRUE;
		}
	}

	LTFLOAT fTime = g_pInterface->GetTime();

	// Play idle if it is time...

	LTBOOL bPlayIdle = LTFALSE;

	if (fTime > m_fNextIdleTime)
	{
		bPlayIdle = DTRUE;
	}

	uint32 nSubleIdleAni = GetSubtleIdleAni();

	if (bPlayIdle)
	{
		m_fNextIdleTime	= GetNextIdleTime();

		uint32 nAni = GetIdleAni();

		if (nAni != INVALID_ANI)
		{
			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, nAni);
			nAni = 0;

			return DTRUE;
		}

	}
	else if (nSubleIdleAni != INVALID_ANI)
	{
		// Play subtle idle...

		if (dwAni != nSubleIdleAni)
		{
			g_pInterface->SetModelLooping(m_hObject, DTRUE);
			g_pInterface->SetModelAnimation(m_hObject, nSubleIdleAni);
		}
		
		return DTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Select()
//
//	PURPOSE:	Select the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Select()
{

	m_eState = W_SELECT;
	uint32 dwSelectAni = GetSelectAni();

	if (m_hObject && dwSelectAni != INVALID_ANI)
	{
		uint32 dwAni = g_pInterface->GetModelAnimation(m_hObject);

		if (!IsSelectAni(dwAni))
		{
			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
			g_pInterface->SetModelAnimation(m_hObject, dwSelectAni);
			g_pInterface->ResetModelAnimation(m_hObject);
		}
	}
	//be sure all pieces are showing
	for(HMODELPIECE hTemp=0 ; hTemp<32 ; hTemp++)
		g_pLTClient->GetModelLT()->SetPieceHideStatus(m_hObject, hTemp, LTFALSE);

	//check for GL piece status
	UpdateGLHideStatus();
	UpdateExoHideStatus();

	m_fNextIdleTime	= GetNextIdleTime();

	//check to see if we need to load a clip
	if(StatusAmmo(m_pBarrel) == AS_CLIP_EMPTY)
	{
		ReloadClip();
	}

	//set the crosshair
	g_pInterfaceMgr->GetCrosshairMgr()->LoadCrosshair(m_pWeapon->szCrosshair, m_pWeapon->fCrosshairScale, m_pWeapon->fCrosshairAlpha);

	//be sure to re-set overlay
	if(g_pGameClientShell->GetVisionModeMgr()->GetRailMode())
		g_pGameClientShell->GetVisionModeMgr()->SetRailMode(LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Deselect()
//
//	PURPOSE:	Deselect the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Deselect(LTBOOL bNoDeselect)
{
	uint32 dwDeselectAni = GetDeselectAni();
	LTBOOL bPlayDeselectAni = (m_hObject && dwDeselectAni != INVALID_ANI);

	if (bPlayDeselectAni && !bNoDeselect)
	{
		uint32 dwAni = g_pInterface->GetModelAnimation(m_hObject);

		if (!IsDeselectAni(dwAni))
		{
			m_eState = W_DESELECT;
//			g_pInterface->SetModelLooping(m_hObject, LTFALSE);
//			g_pInterface->SetModelAnimation(m_hObject, dwDeselectAni);
//			g_pInterface->ResetModelAnimation(m_hObject);
		}
	}
	else
	{
		m_bWeaponDeselected = DTRUE;
	}

	//be sure to re-set zoom
	if(m_fCurZoomLevel > 1)
	{
		m_fCurZoomLevel = 1;
		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		CameraMgrFX_ZoomToggle(hCamera, m_fCurZoomLevel);
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Weapons\\Marine\\RailGun\\zoom.wav", SOUNDPRIORITY_PLAYER_HIGH);
		m_bZooming = m_bZoomed = m_bPreZooming = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleStateChange()
//
//	PURPOSE:	Handle the weapon state changing
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleStateChange(HMESSAGEREAD hMessage)
{
	m_eState = (WeaponState) g_pInterface->ReadFromMessageByte(hMessage);

	if (m_eState == W_DESELECT)
	{
		Deselect();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SendFireMsg
//
//	PURPOSE:	Send fire message to server
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SendFireMsg()
{
	if (!m_hObject) return;


	LTVector vU, vR, vF, vFirePos;

	if (!GetFireInfo(vU, vR, vF, vFirePos)) return;

	// record the fire position
	m_vFirePos = vFirePos;


	// Make sure we always ignore the fire sounds...

	m_wIgnoreFX = WFX_FIRESOUND | WFX_ALTFIRESND;

	// Calculate a random seed...(srand uses this value so it can't be 1, since
	// that has a special meaning for srand)
	
	uint8 nRandomSeed = GetRandom(2, 255);

	WeaponPath wp;
	wp.nBarrelId	= m_nBarrelId;
	wp.vU			= vU;
	wp.vR			= vR;
	wp.fPerturbR	= m_fMovementPerturb;
	wp.fPerturbU	= wp.fPerturbR;
	wp.vPath		= vF;

	// Play Fire sound...

	char* pSnd = m_pBarrel->szFireSound;
	uint8 nFireType = GetLastSndFireType();

	if (nFireType == WMKS_ALT_FIRE)
	{
		pSnd = m_pBarrel->szAltFireSound;
	}
	
	if (pSnd[0])
	{
		g_pClientSoundMgr->PlaySoundLocal(pSnd, SOUNDPRIORITY_PLAYER_HIGH);
	}

	LTBOOL bSendFireMessage = LTTRUE;

	// If we are multiplayer and this is a vector
	// weapon, do client side hit detection.
	if(g_pGameClientShell->IsMultiplayerGame() && m_pAmmo && m_pAmmo->eType == VECTOR)
		bSendFireMessage = ClientHitDetection(wp, vFirePos);

	if(bSendFireMessage)
	{
		// Send Fire message to server...
		CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

		HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_FIRE);
		g_pInterface->WriteToMessageVector(hWrite, &m_vFlashPos);
		g_pInterface->WriteToMessageVector(hWrite, &vFirePos);
		g_pInterface->WriteToMessageVector(hWrite, &vF);
		g_pInterface->WriteToMessageByte(hWrite, nRandomSeed);
		g_pInterface->WriteToMessageByte(hWrite, m_nWeaponId);
		g_pInterface->WriteToMessageByte(hWrite, m_nBarrelId);
		g_pInterface->WriteToMessageByte(hWrite, m_nAmmoId);
		g_pInterface->WriteToMessageByte(hWrite, (LTBOOL) (m_eLastFireType == FT_ALT_FIRE));
		g_pInterface->WriteToMessageByte(hWrite, (uint8) (m_fMovementPerturb * 255.0f));

		if(m_pBarrel && m_pBarrel->eBarrelType==BT_PRED_DETONATOR)
			g_pInterface->WriteToMessageObject(hWrite, m_hHotbombTarget);
		else
			g_pInterface->WriteToMessageObject(hWrite, pStats?pStats->GetAutoTarget():LTNULL);

		g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);

		// TEMP TEMP
//		g_pLTClient->CPrint("Fire Pos: X - %.2f  Y - %.2f  Z - %.2f",vFirePos.x, vFirePos.y, vFirePos.z);
	}

	// special case for MP flamethrower...
	if(g_pGameClientShell->IsMultiplayerGame() && m_pWeapon && stricmp(m_pWeapon->szName, "Flame_Thrower_MP") == 0 )
	{
		// tell our CFX to reset the flame fx...
		CSFXMgr*		psfxMgr	= g_pGameClientShell->GetSFXMgr();
		CCharacterFX*	pFX		= dynamic_cast<CCharacterFX*>(psfxMgr->FindSpecialFX(SFX_CHARACTER_ID , g_pClientDE->GetClientObject()));

		if(pFX)
			pFX->ResetFlameThrowerFX();
	}

	// do a second special case but for the exosuit flame thrower barrel
	// not the weapon since it has a lazer barrel as well...
	if(g_pGameClientShell->IsMultiplayerGame() && m_pBarrel && stricmp(m_pBarrel->szName, "Exosuit_Flame_Thrower_Barrel") == 0 )
	{
		// tell our CFX to reset the flame fx...
		CSFXMgr*		psfxMgr	= g_pGameClientShell->GetSFXMgr();
		CCharacterFX*	pFX		= dynamic_cast<CCharacterFX*>(psfxMgr->FindSpecialFX(SFX_CHARACTER_ID , g_pClientDE->GetClientObject()));

		if(pFX)
			pFX->ResetFlameThrowerFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetFireInfo
//
//	PURPOSE:	Get the fire pos/rot
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::GetFireInfo(LTVector & vU, LTVector & vR, LTVector & vF, 
								LTVector & vFirePos)
{
	LTRotation rRot;

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();

	if (!hCamera) return LTFALSE;

	g_pLTClient->GetObjectPos(hCamera, &vFirePos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
	LTVector vEndPoint;

	IntersectQuery	IQuery;
	IntersectInfo	IInfo;
	IQuery.m_From = vFirePos;
	IQuery.m_To = vFirePos + (vF * (LTFLOAT)m_pBarrel->nRange);
	IQuery.m_Flags	  = INTERSECT_HPOLY | IGNORE_NONSOLID;

	//test for intersect-segment
	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		vEndPoint = IInfo.m_Point;
	}
	else
	{
		vEndPoint = IQuery.m_To;
	}

	//test for min dist
	LTVector vDist = vEndPoint - vFirePos;
	LTVector vMuzzleOffset = GetMuzzleOffset();

	//only adjust our vector if we are shooting more than 150 units
	if(vDist.MagSqr() > 22500.0f)
	{
		//normal target range
		vFirePos += (vR * vMuzzleOffset.x) + (vU * vMuzzleOffset.y) + (vF * vMuzzleOffset.z);
		vEndPoint -= vFirePos;
		vEndPoint.Norm();
		vF = vEndPoint;
	}

	//see if there is an auto target
	HOBJECT hTarget = g_pInterfaceMgr->GetPlayerStats()->GetAutoTarget();

	if(hTarget)
	{
		LTBOOL	bCanSetDir =	(stricmp(m_pPrimBarrel->szName, "SADAR_Tracking_Barrel") != 0) && 
								(stricmp(m_pPrimBarrel->szName, "Disc_Barrel") != 0) &&
								(stricmp(m_pPrimBarrel->szName, "SADAR_Tracking_Barrel_MP") != 0) && 
								(stricmp(m_pPrimBarrel->szName, "Disc_Barrel_MP") != 0);
		if( bCanSetDir )
		{
			LTVector vTarget;
			LTBOOL bTargetTorso=LTTRUE;

			ModelLT* pModelLT   = g_pClientDE->GetModelLT();

			HMODELNODE hNode;

			if (pModelLT->GetNode(hTarget, "Torso_u_node", hNode) == LT_OK)
			{
				LTransform	tTransform;

				if (pModelLT->GetNodeTransform(hTarget, hNode, tTransform, LTTRUE) == LT_OK)
				{
					TransformLT* pTransLT = g_pLTClient->GetTransformLT();
					pTransLT->GetPos(tTransform, vTarget);
				}
				else
					bTargetTorso = LTFALSE;
			}
			else
				bTargetTorso = LTFALSE;

			// Can't find the torso so use the old way...
			if(!bTargetTorso)
			{
				LTVector vObjDims;
				g_pLTClient->GetObjectPos(hTarget, &vTarget);

				PhysicsLT* pPhysics = g_pLTClient->Physics();
				pPhysics->GetObjectDims(hTarget, &vObjDims);

				//adjust to mid chest
				vTarget.y += vObjDims.y/2;
			}

			//get the target's position

			LTVector vToTarget = vTarget-vFirePos;
			vToTarget.Norm();

			vF = vToTarget;
			vR = vU.Cross(vF);
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetLastSndFireType
//
//	PURPOSE:	Get the last fire snd type
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::GetLastSndFireType()
{
	// Determine the fire snd type...

	uint8 nFireType = WMKS_FIRE;

	if (m_eLastFireType == FT_ALT_FIRE)
	{
		nFireType = WMKS_ALT_FIRE;
	}

	return nFireType;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateServerObj
//
//	PURPOSE:	Create a "server" object used by the projectile sfx
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CWeaponModel::CreateServerObj()
{
	if (!m_hObject) return DNULL;

	LTRotation rRot;
	g_pInterface->AlignRotation(&rRot, &m_vPath, DNULL);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	uint32 dwFlags = FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_TOUCH_NOTIFY;
	dwFlags |= m_pAmmo->pProjectileFX ? m_pAmmo->pProjectileFX->dwObjectFlags : 0;

	createStruct.m_ObjectType = OT_NORMAL;
	createStruct.m_Flags = dwFlags;
	VEC_COPY(createStruct.m_Pos, m_vFirePos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	HLOCALOBJ hObj = g_pInterface->CreateObject(&createStruct);

	g_pInterface->Physics()->SetForceIgnoreLimit(hObj, 0.0f);

	return hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::AddImpact
//
//	PURPOSE:	Add the weapon impact
//
// ----------------------------------------------------------------------- //

void CWeaponModel::AddImpact(HLOCALOBJ hObj, LTVector & vImpactPoint, 
							 LTVector & vNormal, SurfaceType eType)
{
	// Handle gadget special case...

	if (m_eAmmoType == GADGET && hObj)
	{
		HandleGadgetImpact(hObj);
		return;  // No impact fx for gadgets...
	}

	::AddLocalImpactFX(	hObj, 
						g_pClientDE->GetClientObject(), 
						m_vFirePos, 
						vImpactPoint, 
						vNormal, 
						eType, 
						m_vPath, 
						m_nWeaponId, 
						m_nAmmoId, 
						m_nBarrelId, 
						m_wIgnoreFX);

	// If we do multiple calls to AddLocalImpact, make sure we only do some
	// effects once :)
	m_wIgnoreFX |= WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER | WFX_IGNORE_IMPACT_SOUND;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo)
{
	// Get the surface type (check the poly first)...

	SurfaceType eType = GetSurfaceType(iInfo);
	
	if (!(eType == ST_LIQUID && m_eAmmoType == CANNON))
	{
		AddImpact(iInfo.m_hObject, iInfo.m_Point, iInfo.m_Plane.m_Normal, eType);
	}

	
	// If we hit liquid, cast another ray that will go through the water...
	
	if (eType == ST_LIQUID)
	{
		qInfo.m_FilterFn = AttackerLiquidFilterFn;

		if (g_pInterface->IntersectSegment(&qInfo, &iInfo))
		{
			// Get the surface type (check the poly first)...

			SurfaceType eType = GetSurfaceType(iInfo);
			
			AddImpact(iInfo.m_hObject, iInfo.m_Point, iInfo.m_Plane.m_Normal, eType);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleGadgetImpact
//
//	PURPOSE:	Handle a gadget vector hitting an object
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleGadgetImpact(HOBJECT hObj)
{
	// If the gadget can activate this type of object, Tell the server 
	// that the gadget was activated on this object...

	uint32 dwUserFlags = 0;
	if (hObj)
	{
		g_pInterface->GetObjectUserFlags(hObj, &dwUserFlags);
	}

	DamageType eType = m_pAmmo->eInstDamageType;
//	if (eType == DT_GADGET_CAMERA_DISABLER && 
//		!(dwUserFlags & USRFLG_GADGET_CAMERA_DISABLER))
//	{
//		return;
//	}
//	else if (eType == DT_GADGET_CODE_DECIPHERER &&
//		!(dwUserFlags & USRFLG_GADGET_CODE_DECIPHERER))
//	{
//		return;
//	}


	LTVector vU, vR, vF, vFirePos;
	if (!GetFireInfo(vU, vR, vF, vFirePos)) return;


	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_FIRE);
	g_pInterface->WriteToMessageVector(hWrite, &m_vFlashPos);
	g_pInterface->WriteToMessageVector(hWrite, &vFirePos);
	g_pInterface->WriteToMessageVector(hWrite, &vF);
	g_pInterface->WriteToMessageByte(hWrite, 0);
	g_pInterface->WriteToMessageByte(hWrite, m_nWeaponId);
	g_pInterface->WriteToMessageByte(hWrite, m_nBarrelId);
	g_pInterface->WriteToMessageByte(hWrite, m_nAmmoId);
	g_pInterface->WriteToMessageByte(hWrite, (LTBOOL) (m_eLastFireType == FT_ALT_FIRE));
	g_pInterface->WriteToMessageByte(hWrite, (uint8) (m_fMovementPerturb * 255.0f));
	g_pInterface->WriteToMessageObject(hWrite, hObj);
	g_pInterface->WriteToMessageObject(hWrite, pStats?pStats->GetAutoTarget():LTNULL);
	g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);

	// Do any special processing...

//	DoSpecialFire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CanChangeToWeapon()
//
//	PURPOSE:	See if we can change to this weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::CanChangeToWeapon(uint8 nCommandId, LTBOOL bForceNextWep)
{
	if (g_pGameClientShell->IsPlayerDead()) return LTFALSE;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return LTFALSE;

	uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);


	// Make sure this is a valid weapon for us to switch to...

	if (!pStats->HaveWeapon(nWeaponId)) return LTFALSE;



	// If this weapon has no ammo, let user know...

	if (AmmoCheck(g_pWeaponMgr->GetWeapon(nWeaponId)) == AS_OUT_OF_AMMO)
	{
		g_pInterfaceMgr->UpdatePlayerStats(IC_OUTOFAMMO_ID, nWeaponId, 0, 0.0f, LTFALSE);		
		return LTFALSE;
	}

	// check to see if we are curently firing
	if(m_eState == W_FIRING && !bForceNextWep || m_eState == W_RELOADING || m_eState == W_SELECT)
	{
		m_bDelayedWeaponChange	= LTTRUE;
		m_nDealyedCommandId		= nCommandId;
		return LTFALSE;
	}
	
	// Reset the dalay bool
	m_bDelayedWeaponChange = LTFALSE;

	return DTRUE;
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetFirstAvailableAmmoType()
//
//	PURPOSE:	Get the fire available ammo type for this weapon.
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::GetFirstAvailableAmmoType(int & nAmmoType)//uint8 nWeaponId, int & nAmmoType)
{
	//sanity check
	if(!m_pBarrel)
		return LTFALSE;
	
	nAmmoType = WMGR_INVALID_ID;

//	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
//	if (!pWeapon) return LTFALSE;

	// If we don't always have ammo, return an ammo type that we have (if
	// possible)...
	
	//loop thru barrels
//	for(int j=0 ; j<pWeapon->nNumBarrelTypes ; j++)
//	{
//		BARREL *pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[j]);

//		if(!pBarrel)
//			return LTFALSE;

		if (m_pBarrel->bInfiniteAmmo)
		{
			nAmmoType = m_pBarrel->nAmmoType;
			return DTRUE;
		}
		else
		{
			CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
			if (!pStats) return LTFALSE;

//			for (int i=0; i < m_pBarrel->nNumAmmoTypes; i++)
//			{
				if (pStats->GetAmmoCount(m_pBarrel->nAmmoType) > 0)
				{
					nAmmoType = m_pBarrel->nAmmoType;
					return DTRUE;
				}
//			}
		}
//	}

	return LTFALSE;
}

*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeWeapon()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeWeapon(uint8 nCommandId, LTBOOL bNoDeselect)
{
	if (!CanChangeToWeapon(nCommandId, bNoDeselect)) return;

	//save old weapon
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;

	//get weapon set info
	WEAPON_SET *pSet = g_pWeaponMgr->GetWeaponSet(pStats->GetWeaponSet());
	if(!pSet) return;

	//get the index of the current weapon in the set i.e. 0 = first weapon in the set.
	uint8 nSetId = pStats->GetWeaponIndex(m_nWeaponId);

	if(nSetId != WMGR_INVALID_ID && m_nLastWeaponId != m_nWeaponId)
		m_nLastWeaponId = m_nWeaponId;

	// Don't do anything if we are trying to change to the same weapon...

	LTBOOL bDeselectWeapon = (m_nWeaponId != WMGR_INVALID_ID);
	uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);

	if (nWeaponId == m_nWeaponId)
	{
		return;
	}


	// Handle deselection of current weapon...

	if (bDeselectWeapon)
	{
		// Need to wait for deselection animation to finish...Save the
		// new weapon id...

		m_nRequestedWeaponId = nWeaponId;

		Deselect(bNoDeselect);
	}
	else
	{
		HandleInternalWeaponChange(nWeaponId);
	}

	pStats->ValidateClips();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ForceChangeWeapon()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ForceChangeWeapon(uint8 nWeaponId, LTBOOL bDeselect)
{
	//save old weapon
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;

	//get weapon set info
	WEAPON_SET *pSet = g_pWeaponMgr->GetWeaponSet(pStats->GetWeaponSet());
	if(!pSet) return;

	//get the index of the current weapon in the set i.e. 0 = first weapon in the set.
	uint8 nSetId = pStats->GetWeaponIndex(m_nWeaponId);

	if(nSetId != WMGR_INVALID_ID && m_nLastWeaponId != m_nWeaponId)
		m_nLastWeaponId = m_nWeaponId;

	// Don't do anything if we are trying to change to the same weapon...

	LTBOOL bDeselectWeapon = bDeselect;
	
	if(m_nWeaponId == WMGR_INVALID_ID)
		bDeselectWeapon = LTFALSE;

	if (nWeaponId == m_nWeaponId)
	{
		return;
	}


	// Handle deselection of current weapon...

	if (bDeselectWeapon)
	{
		// Need to wait for deselection animation to finish...Save the
		// new weapon id...

		m_nRequestedWeaponId = nWeaponId;

		Deselect();
	}
	else
	{
		HandleInternalWeaponChange(nWeaponId);
	}

	//be sure clips are in order
	pStats->ValidateClips();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleInternalWeaponChange()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleInternalWeaponChange(uint8 nWeaponId)
{
	if (g_pGameClientShell->IsPlayerDead()) return;


	// Change to the weapon...

	DoWeaponChange(nWeaponId);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoWeaponChange()
//
//	PURPOSE:	Do the actual weapon change.  This isn't part of 
//				HandleInternalWeaponChange() so that it can be called when
//				loading the player.  
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoWeaponChange(uint8 nWeaponId)
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if(!pWeapon) return;

	LTBOOL bIsTorch = (stricmp(pWeapon->szName, "Blowtorch") == 0);
	LTBOOL bIsHacker = (stricmp(pWeapon->szName, "MarineHackingDevice") == 0);
	LTBOOL bIsPounceJump = (stricmp(pWeapon->szName, "PounceJump") == 0);
	LTBOOL bIsMediComp = (stricmp(pWeapon->szName, "Medicomp") == 0);
	LTBOOL bIsEnergySift = (stricmp(pWeapon->szName, "Energy_Sift") == 0);

	if (pStats->HaveWeapon(nWeaponId) || bIsTorch || bIsHacker || bIsPounceJump || bIsMediComp || bIsEnergySift)
	{
		g_pGameClientShell->ChangeWeapon(nWeaponId);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeToPrevWeapon()
//
//	PURPOSE:	Change to the previous weapon is
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeToPrevWeapon()	
{ 
	uint8 nWepIndex = PrevWeapon();

	ChangeWeapon(COMMAND_ID_WEAPON_BASE + nWepIndex);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeToNextBestWeapon()
//
//	PURPOSE:	Change to the next weapon is
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeToNextBestWeapon(LTBOOL bNoDeselect)
{ 
	uint8 nWepIndex = NextWeapon(LTTRUE);

	uint8 nCurWep = 0;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if(pStats)
		nCurWep = pStats->GetWeaponIndex(m_pWeapon->nId);

	if(nWepIndex < nCurWep)
	{
		//hmm... let's use prev wep
		nWepIndex = PrevWeapon(LTTRUE);
	}

	ChangeWeapon(COMMAND_ID_WEAPON_BASE + nWepIndex, bNoDeselect);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PrevWeapon()
//
//	PURPOSE:	Determine what the previous weapon is
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::PrevWeapon(LTBOOL bNoGarbage)
{
	//get player stats
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return -1;

	//get weapon set info
	WEAPON_SET *pSet = g_pWeaponMgr->GetWeaponSet(pStats->GetWeaponSet());
	if(!pSet) return -1;

	//get the index of the current weapon in the set i.e. 0 = first weapon in the set.
	uint8 rVal = pStats->GetWeaponIndex(m_pWeapon->nId);

	if(rVal != 0)
	{
		//decrement rval once then start testing
		for ( rVal-- ; (int)rVal>-1 ; rVal--)
		{
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pStats->GetWeaponFromSet(rVal));

			// Check for garbage
			if(pWeapon && bNoGarbage)
			{
				if(stricmp(pWeapon->szName, "Medicomp") == 0 || stricmp(pWeapon->szName, "Energy_Sift") == 0)
					pWeapon = LTNULL;
			}

			if(pWeapon)
			{
				//see if we got the weapon
				if(pStats->HaveWeapon(pWeapon->nId))
					//now see if we got the ammo
					if(AmmoCheck(pWeapon) != AS_OUT_OF_AMMO)
						//WE HAVE A WINNER!!!
						return rVal;
			}
		}
	}

	//set our limit
	uint8 nLimit = pStats->GetWeaponIndex(m_pWeapon->nId);

	//didn't find any going down the list, lets try from the top
	for(rVal=(pSet->nNumWeapons-1) ; rVal != nLimit ; rVal--)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pStats->GetWeaponFromSet(rVal));

		// Check for garbage
		if(pWeapon && bNoGarbage)
		{
			if(stricmp(pWeapon->szName, "Medicomp") == 0 || stricmp(pWeapon->szName, "Energy_Sift") == 0)
				pWeapon = LTNULL;
		}

		if(pWeapon)
		{
			//see if we got the weapon
			if(pStats->HaveWeapon(pWeapon->nId))
				//now see if we got the ammo
				if(AmmoCheck(pWeapon) != AS_OUT_OF_AMMO)
					//WE HAVE A WINNER!!!
					return rVal;
		}
	}

	//hmmmm  couldn't find a next weapon, very wierd
	return -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::NextWeapon()
//
//	PURPOSE:	Determine what the next weapon is
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::NextWeapon(LTBOOL bNoGarbage)
{
	//get player stats
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return -1;

	//get weapon set info
	WEAPON_SET *pSet = g_pWeaponMgr->GetWeaponSet(pStats->GetWeaponSet());
	if(!pSet) return -1;

	//get the index of the current weapon in the set i.e. 0 = first weapon in the set.
	uint8 rVal = pStats->GetWeaponIndex(m_pWeapon->nId);

	//incriment rval once then start testing
	for ( rVal++ ; rVal<pSet->nNumWeapons ; rVal++)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pStats->GetWeaponFromSet(rVal));

		// Check for garbage
		if(pWeapon && bNoGarbage)
		{
			if(stricmp(pWeapon->szName, "Medicomp") == 0 || stricmp(pWeapon->szName, "Energy_Sift") == 0)
				pWeapon = LTNULL;
		}

		if(pWeapon)
		{
			//see if we got the weapon
			if(pStats->HaveWeapon(pWeapon->nId))
				//now see if we got the ammo
				if(AmmoCheck(pWeapon) != AS_OUT_OF_AMMO)
					//WE HAVE A WINNER!!!
					return rVal;
		}
	}

	//set our limit
	uint8 nLimit = pStats->GetWeaponIndex(m_pWeapon->nId);

	//didn't find any going up the list, lets try from the bottom
	for(rVal=0 ; rVal != nLimit ; rVal++)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pStats->GetWeaponFromSet(rVal));

		// Check for garbage
		if(pWeapon && bNoGarbage)
		{
			if(stricmp(pWeapon->szName, "Medicomp") == 0 || stricmp(pWeapon->szName, "Energy_Sift") == 0)
				pWeapon = LTNULL;
		}

		if(pWeapon)
		{
			//see if we got the weapon
			if(pStats->HaveWeapon(pWeapon->nId))
				//now see if we got the ammo
				if(AmmoCheck(pWeapon) != AS_OUT_OF_AMMO)
					//WE HAVE A WINNER!!!
					return rVal;
		}
	}

	//hmmmm  couldn't find a next weapon, very wierd
	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::LastWeapon()
//
//	PURPOSE:	Load the last weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::LastWeapon(LTBOOL bNoDeselect)
{
	//check for no last weapon
	if(m_nLastWeaponId == WMGR_INVALID_ID) return;
	
	//get player stats
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;

	//get weapon set info
	WEAPON_SET *pSet = g_pWeaponMgr->GetWeaponSet(pStats->GetWeaponSet());
	if(!pSet) return;

	//get the index of the current weapon in the set i.e. 0 = first weapon in the set.
	uint8 nSetId = pStats->GetWeaponIndex(m_nLastWeaponId);

	if(nSetId == WMGR_INVALID_ID) return;

	if(CanChangeToWeapon(COMMAND_ID_WEAPON_BASE + nSetId, bNoDeselect))
		//go ahead and change the weapon
		ChangeWeapon(COMMAND_ID_WEAPON_BASE + nSetId, bNoDeselect);
	else
	{
		if(!m_bDelayedWeaponChange)
			ChangeToNextBestWeapon(bNoDeselect);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::NextAmmo()
//
//	PURPOSE:	Determine the next ammo type
//
// ----------------------------------------------------------------------- //
/*
uint8 CWeaponModel::NextAmmo(uint8 nCurrAmmoId)	
{ 
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats || !m_pWeapon) return -1;

	if (!g_pWeaponMgr->IsValidAmmoType(nCurrAmmoId))
	{
		nCurrAmmoId = m_nAmmoId;
	}

	int nNewAmmoId = nCurrAmmoId;
	int nOriginalAmmoIndex = 0;
	int nCurAmmoIndex = 0;
	int nAmmoCount = 0;
	
//	for (int i=0; i < m_pBarrel->nNumAmmoTypes; i++)
//	{
		if (nCurrAmmoId == m_pBarrel->nAmmoType)
		{
			nOriginalAmmoIndex = 0;
			nCurAmmoIndex = 0;
//			break;
		}
//	}

	while (1)
	{
		nCurAmmoIndex++;

		if (nCurAmmoIndex >= 1) nCurAmmoIndex = 0;
		if (nCurAmmoIndex == nOriginalAmmoIndex) break;

		nAmmoCount = pStats->GetAmmoCount(m_pBarrel->nAmmoType);
		if (nAmmoCount > 0)
		{
			nNewAmmoId = m_pBarrel->nAmmoType;
			break;
		}
	}

	return nNewAmmoId;
}
*/
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeAmmo()
//
//	PURPOSE:	Change to the specified ammo type
//
// ----------------------------------------------------------------------- //
/*
void CWeaponModel::ChangeAmmo(uint8 nNewAmmoId)
{
	// Update the player's stats...
	if (m_eState == W_RELOADING) return;

	if (g_pWeaponMgr->IsValidAmmoType(nNewAmmoId) && nNewAmmoId != m_nAmmoId)
	{
		m_nAmmoId = nNewAmmoId;
		m_pAmmo	  = g_pWeaponMgr->GetAmmo(m_nAmmoId);

		ReloadClip(DTRUE, -1, DTRUE);
	}
}
*/
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetWeaponOffset()
//
//	PURPOSE:	Set the weapon offset
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetWeaponOffset()
{
	LTVector vRet;
	vRet.Init();

	if (!m_pWeapon) return vRet;
	return m_pWeapon->vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetWeaponOffset()
//
//	PURPOSE:	Set the weapon offset
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetWeaponOffset(LTVector vPos)
{
//	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);

	if (!m_pWeapon) return;
	m_pWeapon->vPos = vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetMuzzleOffset()
//
//	PURPOSE:	Set the weapon muzzle offset
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetMuzzleOffset()
{
	LTVector vRet;
	vRet.Init();

	if (!m_pBarrel) return vRet;
	return m_pBarrel->vMuzzlePos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetMuzzleOffset()
//
//	PURPOSE:	Set the muzzle offset
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetMuzzleOffset(LTVector vPos)
{
	if (!m_pBarrel) return;
	m_pBarrel->vMuzzlePos = vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetShellEjectPos()
//
//	PURPOSE:	Get the shell eject pos
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetShellEjectPos(LTVector & vOriginalPos)
{
	LTVector vPos = vOriginalPos;

	if (m_hObject && m_hBreachSocket != INVALID_MODEL_SOCKET)
	{
		LTransform transform;
		if (g_pModelLT->GetSocketTransform(m_hObject, m_hBreachSocket, transform, DTRUE) == LT_OK)
		{
			HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
			LTVector vCamPos;
			g_pLTClient->GetObjectPos(hCamera,&vCamPos);
			g_pTransLT->GetPos(transform, vPos);
			vPos+=vCamPos;
		}
	}

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetNextIdleTime()
//
//	PURPOSE:	Determine the next time we should play an idle animation
//
// ----------------------------------------------------------------------- //

LTFLOAT CWeaponModel::GetNextIdleTime()
{
	return g_pInterface->GetTime() + GetRandom(WEAPON_MIN_IDLE_TIME, WEAPON_MAX_IDLE_TIME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetFireAni()
//
//	PURPOSE:	Get the fire animation based on the fire type
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetFireAni(FireType eFireType)
{
	int nNumValid = 0;

//	if ((eFireType == FT_ALT_FIRE && !CanUseAltFireAnis()) || 
//		(m_bUsingAltFireAnis && eFireType == FT_NORMAL_FIRE))

	if (eFireType == FT_ALT_FIRE && !CanUseAltFireAnis())
	{
		uint32 dwValidAltFireAnis[WM_MAX_ALTFIRE_ANIS];

		for (int i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
		{
			if (m_nAltFireAnis[i] != INVALID_ANI)
			{
				dwValidAltFireAnis[nNumValid] = m_nAltFireAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidAltFireAnis[GetRandom(0, nNumValid-1)];
		}
	}
	else if (eFireType == FT_NORMAL_FIRE)
	{
		uint32 dwValidFireAnis[WM_MAX_FIRE_ANIS];

		for (int i=0; i < WM_MAX_FIRE_ANIS; i++)
		{
			if (m_nFireAnis[i] != INVALID_ANI)
			{
				dwValidFireAnis[nNumValid] = m_nFireAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidFireAnis[GetRandom(0, nNumValid-1)];
		}
	}

	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetIdleAni()
//
//	PURPOSE:	Get an idle animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetIdleAni()
{
	int nNumValid = 0;

	if (m_bUsingAltFireAnis)
	{
		uint32 dwValidAltIdleAnis[WM_MAX_ALTIDLE_ANIS];

		// Note that we skip the first ani, this is reserved for 
		// the subtle idle ani...

		for (int i=1; i < WM_MAX_ALTIDLE_ANIS; i++)
		{
			if (m_nAltIdleAnis[i] != INVALID_ANI)
			{
				dwValidAltIdleAnis[nNumValid] = m_nAltIdleAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidAltIdleAnis[GetRandom(0, nNumValid-1)];
		}
	}
	else  // Normal idle anis
	{
		uint32 dwValidIdleAnis[WM_MAX_IDLE_ANIS];

		// Note that we skip the first ani, this is reserved for 
		// the subtle idle ani...

		for (int i=1; i < WM_MAX_IDLE_ANIS; i++)
		{
			if (m_nIdleAnis[i] != INVALID_ANI)
			{
				dwValidIdleAnis[nNumValid] = m_nIdleAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidIdleAnis[GetRandom(0, nNumValid-1)];
		}
	}


	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetSubtleIdleAni()
//
//	PURPOSE:	Get a sutble idle animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetSubtleIdleAni()
{
	return m_bUsingAltFireAnis ? m_nAltIdleAnis[0] : m_nIdleAnis[0];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetSelectAni()
//
//	PURPOSE:	Get a select animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetSelectAni()
{
	return m_bUsingAltFireAnis ? m_nAltSelectAni : m_nSelectAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetReloadAni()
//
//	PURPOSE:	Get a reload animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetReloadAni()
{
	return m_bUsingAltFireAnis ? m_nAltReloadAni : m_nReloadAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetDeselectAni()
//
//	PURPOSE:	Get a deselect animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetDeselectAni()
{
	uint32 dwAni = INVALID_ANI;


	if (m_bUsingAltFireAnis)
	{
		// If we're actually changing weapons make sure we use the 
		// currect AltDeselect animation...

		if (m_nRequestedWeaponId != WMGR_INVALID_ID &&
			m_nRequestedWeaponId != m_nWeaponId)
		{
			dwAni = m_nAltDeselect2Ani;
		}
		else
		{
			dwAni = m_nAltDeselectAni;
		}
	}
	else
	{
		dwAni = m_nDeselectAni;
	}

	return dwAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsFireAni()
//
//	PURPOSE:	Is the passed in animation a fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsFireAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	for (int i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		if (m_nFireAnis[i] == dwAni)
		{
			return DTRUE;
		}
	}

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		if (m_nAltFireAnis[i] == dwAni)
		{
			return DTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsChargeFireAni()
//
//	PURPOSE:	Is the passed in animation a charge fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsChargeFireAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nChargeFireAni == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsChargeAni()
//
//	PURPOSE:	Is the passed in animation a charge animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsChargeAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nChargingAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsPreChargeAni()
//
//	PURPOSE:	Is the passed in animation a pre-charge animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsPreChargeAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nPreChargingAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsSpinupAni()
//
//	PURPOSE:	Is the passed in animation a spinup animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsSpinupAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nSpinupAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsStartSpinupAni()
//
//	PURPOSE:	Is the passed in animation a start spinup animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsStartSpinupAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nSpinupStartAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsEndSpinupAni()
//
//	PURPOSE:	Is the passed in animation a end spinup animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsEndSpinupAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nSpinupEndAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsStartFireAni()
//
//	PURPOSE:	Is the passed in animation a start fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsStartFireAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nStartFireAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsEndFireAni()
//
//	PURPOSE:	Is the passed in animation a end fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsEndFireAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nEndFireAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsStartZoomAni()
//
//	PURPOSE:	Is the passed in animation a start zoom animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsStartZoomAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nStartZoomAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsEndZoomAni()
//
//	PURPOSE:	Is the passed in animation an end zoom animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsEndZoomAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nEndZoomAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsDetonateAni()
//
//	PURPOSE:	Is the passed in animation a detonate animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsDetonateAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (m_nDetonateAnim == dwAni)
		return DTRUE;

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsIdleAni()
//
//	PURPOSE:	Is the passed in animation an idle animation (NOTE this
//				will return LTFALSE if the passed in animation is a subtle
//				idle animation).
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsIdleAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	for (int i=1; i < WM_MAX_IDLE_ANIS; i++)
	{
		if (m_nIdleAnis[i] == dwAni)
		{
			return DTRUE;
		}
	}

	for (i=1; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		if (m_nAltIdleAnis[i] == dwAni)
		{
			return DTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsDeselectAni()
//
//	PURPOSE:	Is this a valid deselect ani
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsDeselectAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (dwAni == m_nDeselectAni || 
		dwAni == m_nAltDeselectAni ||
		dwAni == m_nAltDeselect2Ani)
	{
		return DTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsSelectAni()
//
//	PURPOSE:	Is this a valid Select ani
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsSelectAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (dwAni == m_nSelectAni || dwAni == m_nAltSelectAni)
	{
		return DTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsReloadAni()
//
//	PURPOSE:	Is this a valid Reload ani
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsReloadAni(uint32 dwAni)
{
	if (dwAni == INVALID_ANI) return LTFALSE;

	if (dwAni == m_nReloadAni || dwAni == m_nAltReloadAni)
	{
		return DTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsAmmoChangeAni()
//
//	PURPOSE:	Is this a valid Reload ani
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsAmmoChangeAni(uint32 dwAni)
{
	//test this first because all anims default to this.
	if (dwAni == INVALID_ANI) return LTFALSE;

	return(dwAni == m_nAmmoChangeAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CanUseAltFireAnis()
//
//	PURPOSE:	Can we use alt-fire anis?
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::CanUseAltFireAnis()
{
	return (m_nAltSelectAni != INVALID_ANI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateMovementPerturb()
//
//	PURPOSE:	Update m_fMovementPerturb
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateMovementPerturb()
{
	//sanity check
	if(!g_pGameClientShell)
		return;
	
	LTFLOAT		fDelta		= g_pLTClient->GetFrameTime();
//	CMoveMgr*	pMoveMgr	= g_pGameClientShell->GetMoveMgr();
	LTFLOAT		fMove		= 0.0f; 
	LTVector		vPlayerRot; 
	
//	if(pMoveMgr)
//		fMove = g_pGameClientShell->GetMoveMgr()->GetMovementPercent();

	//test for limits
	if (fMove > 1.0f)
		fMove = 1.0f;

	//if walking
//	if (pMoveMgr || !(pMoveMgr->GetControlFlags() & BC_CFLG_RUN) || g_pGameClientShell->IsZoomed())
//	{
//		fMove *= g_vtPerturbWalkPercent.GetFloat();
//	}

	//get player rotation vector
//	g_pGameClientShell->GetPlayerPitchYawRoll(vPlayerRot);

	//calculate deltas
//	DFLOAT fPitchDiff	= (DFLOAT)fabs(vPlayerRot.x - m_fLastPitch);
//	DFLOAT fYawDiff		= (DFLOAT)fabs(vPlayerRot.y - m_fLastYaw);

	//record current data
//	m_fLastPitch	= vPlayerRot.x;
//	m_fLastYaw		= vPlayerRot.y;

	//calculate rotation
//	DFLOAT fRot =	g_vtPerturbRotationEffect.GetFloat() * 
//					(fPitchDiff + fYawDiff) / 
//					(2.0f + g_vtFastTurnRate.GetFloat() * fDelta);
	
	//test for limits
//	if (fRot > 1.0f)
//		fRot = 1.0f;

	//take the greater of the rotation or movement factor
	//and claculate the changer from the last factor
//	DFLOAT fAdj = Max(fRot,fMove);
//	DFLOAT fDiff = (DFLOAT)fabs(fAdj - m_fMovementPerturb);

	//if the adjustment factor is greater than the last factor
	//increment the movemnet factor (adjusted for frame time)
	//otherwise decrement the factor.

	// TEMP!! (ALM) 6/12/00
	m_fMovementPerturb = 1.0f;

/*	if (fAdj >  m_fMovementPerturb)
	{
		fDelta *= g_vtPerturbIncreaseSpeed.GetFloat();
		m_fMovementPerturb += Min(fDelta,fDiff);
	}
	else if (fAdj <  m_fMovementPerturb)
	{
		fDelta *= g_vtPerturbDecreaseSpeed.GetFloat();
		m_fMovementPerturb -= Min(fDelta,fDiff);
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetAmmoInClip()
//
//	PURPOSE:	Gets the amount of ammo in the current
//				barrel's clip
//
// ----------------------------------------------------------------------- //

int CWeaponModel::GetAmmoInClip(BARREL* pBarrel)
{
	//sanity check
	if(!g_pGameClientShell || !pBarrel)
		return 0;

	//get player stats pointer
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	if(!pStats)
		return 0;

	//get the amount
	return pStats->GetAmmoInClip(pBarrel->nId);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HaveAmmo()
//
//	PURPOSE:	Returns true if the weapon uses a clip and has ammo
//				in the clip or if it does not use a clip it will return
//				true if there is ammo.	
//
// ----------------------------------------------------------------------- //

AmmoStatus CWeaponModel::StatusAmmo(BARREL* pBarrel)
{
	//get player stats pointer
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	//sanity check
	if(!pStats || !pBarrel || !g_pWeaponMgr)
		return AS_INVALID;

	//check for infinite ammo
	if(pBarrel->bInfiniteAmmo)
		return AS_INFINITE;

	//get ammo data
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);

	//test
	if(!pAmmo)
		return AS_OUT_OF_AMMO;

	//see if we have ammo enuf for a shot
	if (pStats->GetAmmoCount(pAmmo->nId) < (uint32)pAmmo->nAmmoPerShot)
		return AS_OUT_OF_AMMO;


	//see if we have any clip issues
	if(pBarrel->nShotsPerClip > 0)
	{
		if(GetAmmoInClip(pBarrel) <= 0)
			return AS_CLIP_EMPTY;
	}


	//all is good in the land of ammo!
	return AS_HAS_AMMO;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::NextBarrel()
//
//	PURPOSE:	Changes to a new barrel if there is one.
//				
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::NextPrimBarrel(BARREL* pCurBarrel)
{
	if(!pCurBarrel) return LTFALSE;

	//loop through barrels until one with ammo is found
	BARREL* pBarrel = pCurBarrel;

	while(LTTRUE)
	{
		pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);

		if(pBarrel && pBarrel != pCurBarrel)
		{
			//check to see if there is ammo
			AmmoStatus eAmmo = StatusAmmo(pBarrel);

			if(eAmmo == AS_HAS_AMMO || eAmmo == AS_INFINITE || eAmmo == AS_CLIP_EMPTY)
			{
				m_pPrimBarrel = pBarrel;
				CreateFlash();
				//record the barrel...
				CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
				if(pStats)
				{
					pStats->SetBarrelID(m_pWeapon->nId, pBarrel->nId);
				}
				return LTTRUE;
			}
		}
		else
			return LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::AmmoCheck()
//
//	PURPOSE:	Checks to see if there is any ammo in the gun at all
//				
// ----------------------------------------------------------------------- //

AmmoStatus CWeaponModel::AmmoCheck(WEAPON *pWeapon)
{
	//loop through barrels until one with ammo is found
	BARREL *pStartBarrel, *pBarrel;
	
	if(pWeapon)
	{
		pStartBarrel = pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[PRIMARY_BARREL]);
	}
	else
	{
		pStartBarrel = pBarrel  = m_pPrimBarrel;
	}

	//test variable
	AmmoStatus eAmmo;

	do
	{
		eAmmo = StatusAmmo(pBarrel);

		if(eAmmo != AS_OUT_OF_AMMO)
			return AS_NEXT_BARREL;

		pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);
	}while(pBarrel && pBarrel != pStartBarrel);

	if(pWeapon)
	{
		eAmmo = StatusAmmo(g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[ALT_BARREL]));
	}
	else
	{
		eAmmo = StatusAmmo(m_pAltBarrel);
	}

	if(eAmmo != AS_OUT_OF_AMMO && eAmmo != AS_INVALID)
		return AS_HAS_AMMO;

	return AS_OUT_OF_AMMO;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetVisionModeSkins
//						(HOBJECT hObj, ObjectCreateStruct & createStruct)
//
//	PURPOSE:	If there is a vision mode active then set the 
//				appropriate skin
//				
// ----------------------------------------------------------------------- //

void CWeaponModel::ResetModelSkins()
{
	if(!m_pWeapon) return;

	ObjectCreateStruct createStruct;

	//init create struct
	INIT_OBJECTCREATESTRUCT(createStruct);

	//load model name
	SAFE_STRCPY(createStruct.m_Filename, m_pWeapon->szPVModel);

	
	//get player stats
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	CString szAppend;

	if(pStats)
	{
		uint32 nSet = pStats->GetButeSet();
		szAppend = g_pCharacterButeMgr->GetWeaponSkinType(nSet);
	}


	//load skin names
	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		if(szAppend.IsEmpty())
		{
			SAFE_STRCPY(createStruct.m_SkinNames[i], m_pWeapon->szPVSkins[i]);
		}
		else
		{
			sprintf(createStruct.m_SkinNames[i], m_pWeapon->szPVSkins[i], szAppend);
		}
	}

	if(g_pGameClientShell->IsMultiplayerGame())
	{
		if(m_pPrimBarrel && stricmp(m_pWeapon->szName, "Grenade_Launcher_MP")==0)
			//overwrite the third one
			SAFE_STRCPY(createStruct.m_SkinNames[2], m_pPrimBarrel->szPVAltSkin);
	}
	else
	{
		if(m_pPrimBarrel && stricmp(m_pWeapon->szName, "Grenade_Launcher")==0)
			//overwrite the third one
			SAFE_STRCPY(createStruct.m_SkinNames[2], m_pPrimBarrel->szPVAltSkin);
	}


	//don't re-set the animations
	createStruct.m_bResetAnimations = LTFALSE;

	//record the filenames
	g_pInterface->Common()->SetObjectFilenames(m_hObject, &createStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::void	UpdateGLHideStatus();
//
//	PURPOSE:	Hide the shot grenades	
//				
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateGLHideStatus(LTBOOL bShowAll)
{
	if(m_pWeapon)
	{
		//be sure we are on the grenade launcher
		if(stricmp(m_pWeapon->szName, "Grenade_Launcher")==0 || stricmp(m_pWeapon->szName, "Grenade_Launcher_MP")==0)
		{
			//get player stats pointer
			CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

			//get ammo status
			int nAmmoUsed = m_pPrimBarrel->nShotsPerClip - pStats->GetAmmoInClip(m_pPrimBarrel->nId);

			//get the model interface
			ModelLT* pModelLT   = g_pClientDE->GetModelLT();

			if(!pModelLT) return;

			HMODELPIECE hPiece;
			char pTemp[15];

			//show ammo that is there hide the others
			for(int i=0; i<m_pPrimBarrel->nShotsPerClip ; i++)
			{
				//set piece name and get piece handle
				sprintf(pTemp,"projectile%d",i);
				pModelLT->GetPiece(m_hObject, pTemp, hPiece);

				LTBOOL bStatus;

				if(bShowAll)
					bStatus = LTFALSE;
				else
					bStatus = i<nAmmoUsed?LTTRUE:LTFALSE;

				pModelLT->SetPieceHideStatus(m_hObject, hPiece, bStatus);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::void	UpdateExoHideStatus();
//
//	PURPOSE:	Hide the shot missiles	
//				
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateExoHideStatus(LTBOOL bShowAll)
{
	if(m_pWeapon)
	{
		//be sure we are on the grenade launcher
		if(stricmp(m_pWeapon->szName, "Exosuit_Minigun")==0)
		{
			//get player stats pointer
			CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

			//get ammo info
			AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_pAltBarrel->nAmmoType);

			if(!pAmmo) return;

			//get ammo status
			int nAmmo = pStats->GetAmmoCount(pAmmo->nId);

			//get the model interface
			ModelLT* pModelLT   = g_pClientDE->GetModelLT();

			if(!pModelLT) return;

			HMODELPIECE hPiece;
			char pTemp[15];

			if(bShowAll)
			{
				for(int i=0 ; i<3 ; i++)
				{
					sprintf(pTemp,"rocket%d",i);
					pModelLT->GetPiece(m_hObject, pTemp, hPiece);
					pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTFALSE);
				}
				return;
			}

			if(nAmmo < 3)
			{
				//show ammo that is there hide the others
				for(int i=2; i>=nAmmo ; i--)
				{
					//set piece name and get piece handle
					sprintf(pTemp,"rocket%d",i);
					pModelLT->GetPiece(m_hObject, pTemp, hPiece);

					pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);
				}
			}
		}
	}
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::BuildTargetingSpriteUpdate
//
//	PURPOSE:	Build the update message
//				
// ----------------------------------------------------------------------- //

void CWeaponModel::BuildTargetingSpriteUpdate(HMESSAGEWRITE hMessage)
{
	//do an intersect at max weapon range
	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();

	if (!hCamera) return;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	if(pStats && pStats->GetAutoTarget())
	{
		g_pLTClient->WriteToMessageByte(hMessage, LTFALSE);
		return;
	}


	LTVector vPos;
	LTRotation rRot;
	LTVector vF, vR, vU;
	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_pLTClient->GetRotationVectors(&rRot, &vR, &vU, &vF);

	IntersectQuery	IQuery;
	IntersectInfo	IInfo;
	IQuery.m_From = vPos;
	IQuery.m_To = vPos + (vF * (LTFLOAT)m_pBarrel->nRange);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;

	//test for intersect-segment
	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		uint32 nType = g_pLTClient->GetObjectType(IInfo.m_hObject);

		if(nType != OT_MODEL)
		{
			//good enuf to show
			IInfo.m_Point += IInfo.m_Plane.m_Normal;			
			g_pLTClient->WriteToMessageByte(hMessage, LTTRUE);
			g_pLTClient->WriteToMessageVector(hMessage, &IInfo.m_Point);
			g_pLTClient->WriteToMessageVector(hMessage, &IInfo.m_Plane.m_Normal);
			g_pLTClient->WriteToMessageVector(hMessage, &vF);
			return;
		}
	}

	//we are not hitting a world poly
	g_pLTClient->WriteToMessageByte(hMessage, LTFALSE);
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateZooming
//
//	PURPOSE:	Update Zooming
//				
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateZooming()
{
	if(m_bZooming)
	{
		//set zoom level to 4x
		m_fCurZoomLevel += g_pLTClient->GetFrameTime() * g_vtZoomInRate;

		if(m_fCurZoomLevel > g_vtMaxZoom)
		{
			//set to max
			m_fCurZoomLevel =  g_vtMaxZoom;
			m_bZooming = LTFALSE;
		}

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		CameraMgrFX_ZoomToggle(hCamera, m_fCurZoomLevel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsOutOfAmmoOrReloading
//
//	PURPOSE:	Is the weapon reloading or out of ammo?
//				
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsOutOfAmmoOrReloading()
{
	// Get the curent anim
	uint32 dwAni	= g_pInterface->GetModelAnimation(m_hObject);

	// If we are reloading then return
	if(dwAni == m_nReloadAni) return LTTRUE;

	// Now see if we have ammo
	if(StatusAmmo(m_pPrimBarrel) == AS_OUT_OF_AMMO) return LTTRUE;

	// If we get to here then we are not out of ammo
	// nor are we reloading.
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ClientHitDetection
//
//	PURPOSE:	Do the client side hit detection
//				Return LTTRUE if we did not hit anything so that
//				the server can handle the normal shot.
//				
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::ClientHitDetection(WeaponPath& wp, LTVector& vFirePos)
{
	// Set up our return value 
	LTBOOL bRval = LTTRUE;

	// Sanity check
	if(!m_pBarrel) return bRval;


	// Be sure the up and right vectors are accurate
	LTVector vU, vR, vF;
	LTRotation rRot;

	g_pCameraMgr->GetCameraRotation(g_pGameClientShell->GetPlayerCamera(), rRot);
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	wp.vU = vU;
	wp.vR = vR;

	// Just in case we hit more then once.
	LTBOOL bHidePVFX = LTFALSE;

	// Loop thru the number of vectors
	for(int i=m_pBarrel->nVectorsPerRound ; i ; --i)
	{
		// Do the perturb thing...
		g_pWeaponMgr->CalculateWeaponPath(wp);

		// Ok, time to do our intersection test...
		IntersectInfo	iInfo;
		IntersectQuery	qInfo;

		qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		qInfo.m_From		= vFirePos;
		qInfo.m_To			= vFirePos + (wp.vPath * (LTFLOAT)m_pBarrel->nRange);

		if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
		{
			// Well we hit somthing, let's see if it is a character...
			CSFXMgr*		psfxMgr	= g_pGameClientShell->GetSFXMgr();
			CCharacterFX*	pFX		= dynamic_cast<CCharacterFX*>(psfxMgr->FindSpecialFX(SFX_CHARACTER_ID , iInfo.m_hObject));

			if(pFX)
			{
				ModelNode eModelNode;

				// Looks like we hit a character so lets do some node detection...
				if(HandleVectorImpact(iInfo, wp.vPath, pFX, &eModelNode))
				{
					// Since we have a valid hit, lets not let the rest
					// of the code send the normal fire message.
					bRval = LTFALSE;

					// Ok, now send our new message.
					LTVector vTargetPos;
					g_pLTClient->GetObjectPos(iInfo.m_hObject, &vTargetPos);

					// Send Fire message to server...
					CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

					HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_HIT);
					g_pInterface->WriteToMessageVector(hWrite, &m_vFlashPos);
					g_pInterface->WriteToMessageVector(hWrite, &vFirePos);
					g_pInterface->WriteToMessageVector(hWrite, &wp.vPath);
					g_pInterface->WriteToMessageByte(hWrite, m_nWeaponId);
					g_pInterface->WriteToMessageByte(hWrite, m_nBarrelId);
					g_pInterface->WriteToMessageByte(hWrite, m_nAmmoId);
					g_pInterface->WriteToMessageByte(hWrite, (LTBOOL) (m_eLastFireType == FT_ALT_FIRE));
					g_pInterface->WriteToMessageObject(hWrite, iInfo.m_hObject);
					g_pInterface->WriteToMessageByte(hWrite, (BYTE)eModelNode);

					// Make sure we only see the PVFX once.
					g_pInterface->WriteToMessageByte(hWrite, bHidePVFX);
					bHidePVFX = LTTRUE;

					// Last two parts are for validation
					g_pInterface->WriteToMessageFloat(hWrite, g_pLTClient->GetGameTime());
					g_pInterface->WriteToMessageVector(hWrite, &vTargetPos);

					g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
				} // if(HandleVectorImpact(iInfo, wp.vPath, pFX, &eModelNode))
			} // if(pFX)

			// we hit somthing so let's to some local impact FX...
			HandleVectorImpact(qInfo, iInfo);
		} // (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	} // for(int i=m_pBarrel->nVectorsPerRound ; i ; --i)


	// Bye bye...
	return bRval;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleVectorImpact()
//
//	PURPOSE:	Handle being hit by a vector
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir, CCharacterFX* pCharFX, ModelNode* peModelNode)
{
	// Some needed variables...
	ModelNode		eModelNodeHit	= eModelNodeInvalid;
	ModelSkeleton	eModelSkeleton	= pCharFX->GetModelSkeleton();
	int				cNodes			= g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	const char*		szNodeName		= LTNULL;

	// Initialize to an "obviously" too large value.
	LTFLOAT		fNearestRayDist = 1000.0f;
	LTVector	vClosePoint(0.0f, 0.0f, 0.0f);

	// Iterate through each node and/or socket.  If the ray passes 
	// with that node's radius, consider the character hit at that location.
	// The algorithm goes through all the nodes, returning the node 
	// which was "first" hit (the hit node closest to the rays origin).

	// Start the iteration.
	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		// Gather the node's information from the Skeleton bute file.
		ModelNode eCurrentNode = (ModelNode)iNode;


		// Get the model piece related to this node
		const char *pPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkeleton, eCurrentNode);
		HMODELPIECE hPiece;
		LTBOOL bHidden = LTFALSE;

		g_pModelLT->GetPiece(iInfo.m_hObject, const_cast<char*>(pPiece), hPiece);
		if(hPiece != INVALID_MODEL_PIECE)
		{
			// If the piece is hidden... just skip this node
			g_pModelLT->GetPieceHideStatus(iInfo.m_hObject, hPiece, bHidden);
			if(bHidden) continue;
		}


		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

		if( szNodeName )
		{
			LTVector vPos;
			if( FindNodeOrSocket( iInfo.m_hObject, szNodeName, &vPos ) )
			{

				// Find the node's hit radius.
				LTFLOAT fNodeRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eCurrentNode);
			
				const LTVector vRelativeNodePos = vPos - iInfo.m_Point;

				// Distance along ray to point of closest approach to node point
				const LTFLOAT fRayDist = vDir.Dot(vRelativeNodePos); 

				// If this value is negitive then the vector gets no closer
				// to the node then the start position
				if(fRayDist >= 0.0f)
				{
					const LTFLOAT fDistSqr = (vDir*fRayDist - vRelativeNodePos).MagSqr();

					if (fDistSqr < fNodeRadius*fNodeRadius)
					{
						// Aha!  The node has been hit!  
						// Check to see if the node is closer to
						// the ray's origin.  Record it as the hit
						// if it is.
						if (fRayDist < fNearestRayDist)
						{
							eModelNodeHit = ModelNode(iNode);
							fNearestRayDist = fRayDist;
							vClosePoint = vPos;
						}
					}
				}
			} //if( FindNodeOrSocket( m_hModel, szNodeName, &vPos ) )
		} //if (szNodeName)
	}

	// Did we hit something?

	if (eModelNodeHit != eModelNodeInvalid)
	{
		// finally record the node info
		if(peModelNode)
			*peModelNode = eModelNodeHit;

		return LTTRUE;
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateWeaponSpeedMod()
//
//	PURPOSE:	Update the weapon speed info...
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateWeaponSpeedMod()
{
	// Update the movement speed...
//	LTFLOAT fMod = m_pWeapon->fSpeedMod;	// Right now the weapon speed is being used elsewhere.
	LTFLOAT fMod = 1.0f;

	// Rignt now, only modify if we are an exosuit.
	if(IsExosuit(g_pGameClientShell->GetPlayerStats()->GetButeSet()))
	{
		// See if we are low on energy...
		if( g_pGameClientShell->GetPlayerStats()->GetExoEnergyCount() < (uint32)(g_vtExoMinEnergy))
		{
			fMod -= g_vtExoMoveCost;
		}
	}

	g_pGameClientShell->GetPlayerMovement()->SetWeaponSpeedMod(fMod);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::KillLoopSound()
//
//	PURPOSE:	Kill the looping sound
//
// ----------------------------------------------------------------------- //

void CWeaponModel::KillLoopSound()
{
	m_nLoopKeyId = 0xff;
	if(m_hLoopFireSound)
	{
		g_pLTClient->KillSound(m_hLoopFireSound);
		m_hLoopFireSound = LTNULL;
	}
}
