//----------------------------------------------------------
//
// MODULE  : CAutoTargetMgr.cpp
//
// PURPOSE : Auto-Targeting Utility
//
// CREATED : 6/15/2000
//
//----------------------------------------------------------

#include "stdafx.h"
#include "AutoTargetMgr.h"
#include "GameClientShell.h"
#include "VisionModeButeMGR.h"
#include "CharacterFX.h"
#include "CharacterFuncs.h"
#include "VarTrack.h"
#include "PlayerStats.h"
#include "SFXMgr.h"
#include "BodyPropFX.h"

const LTFLOAT g_cvarSADARLockTime = 2.0f;
const LTFLOAT g_cvarPounceDeadZone = 1.0f;
const LTFLOAT g_cvarPounceTargetLock = 0.8f;
const LTFLOAT g_cvarMaxSGRange = 1800.0f;
const LTFLOAT g_cvarHeadBiteActivationMod = 0.70f;

const LTFLOAT MAX_AUTO_TARGET_RANGE_SQ = 5000000;

extern CGameClientShell* g_pGameClientShell;
LTBOOL SFXObjectFilterFn(HOBJECT hObj, void *pUserData);

using namespace VisionMode;

LTBOOL BodyPropListFilterFn(HOBJECT hTest, void *pUserData)
{
	//Filter out the body props
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return LTTRUE;

	if(psfxMgr->FindSpecialFX(SFX_BODYPROP_ID, hTest))
		return LTFALSE;

	return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CAutoTargetMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAutoTargetMgr::CAutoTargetMgr()
{
	m_psfxMgr				= LTNULL;
	m_pStats				= LTNULL;
	m_eType					= TT_NORMAL;
	m_hPotentialTarget		= LTNULL;
	m_fNewTargetStartTime	= 0.0f;
	m_bLockSoundPlayed		= LTFALSE;
	m_hBiteOrPounceTarget	= LTNULL;
	m_hLockedLoopSound		= LTNULL;
	m_fLastBiteSound		= 0;
	m_vPounceTargetLoc.Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::~CAutoTargetMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAutoTargetMgr::~CAutoTargetMgr()
{
	m_psfxMgr				= LTNULL;
	m_pStats				= LTNULL;

	if(m_hLockedLoopSound)
	{
		g_pLTClient->KillSound(m_hLockedLoopSound);
		m_hLockedLoopSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::~CAutoTargetMgr()
//
//	PURPOSE:	Initialization Routine
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::Init( TargetingType eType )
{	
	m_psfxMgr	= g_pGameClientShell->GetSFXMgr();
	m_pStats	= g_pInterfaceMgr->GetPlayerStats();
	m_eType		= eType;

	if(m_eType == TT_MARINE_SADAR_NORMAL)
	{
		// white crosshair
		g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 1.0, 1.0, 1.0, 1.0);
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);
	}

	m_pStats->SetAutoTargetOn(LTTRUE);
	m_pStats->SetAutoTarget(LTNULL);

	m_bLockSoundPlayed = LTFALSE;
	m_fNewTargetStartTime = 0.0f;
	m_hPotentialTarget = LTNULL;

	ResetSounds();
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::ResetSounds()
//
//	PURPOSE:	Calls the apropriate update function
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::ResetSounds()
{
	//reset the sounds
	if(m_hLockedLoopSound)
	{
		g_pLTClient->KillSound(m_hLockedLoopSound);
		m_hLockedLoopSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdateTargets()
//
//	PURPOSE:	Calls the apropriate update function
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdateTargets()
{
	if(m_pStats->IsAutoTargetOn())
	{
		switch(m_eType)
		{
		case(TT_PRED_SHOULDER_CANNON):	{ UpdatePredShoulderCannon(); break; }
		case(TT_PRED_DISK):				{ UpdatePredShoulderCannon(); break; }
		case(TT_ALIEN):					{ UpdateAlienTargeting(); break; }
		case(TT_MARINE_SMARTGUN):		{ UpdateMarineSmartgun(); break; }
		case(TT_MARINE_SADAR):			{ UpdateMarineSADAR(); break; }
		case(TT_FACEHUGGER):			{ UpdateFacehuggerTargeting(); break; }
		case(TT_CHESTBURSTER):			{ UpdateChestbursterTargeting(); break; }
		case(TT_PREDALIEN):				{ UpdatePredalienTargeting(); break; }
		case(TT_RUNNER):				{ UpdateRunnerTargeting(); break; }
		case(TT_NORMAL):
		default:						{ break;}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdatePredShoulderCannon()
//
//	PURPOSE:	Updates auto-targeting for the Predator
//				shoulder cannon.
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdatePredShoulderCannon()
{
	//sanity check
	if(!m_psfxMgr) return;

	// See if we are dead
	if(g_pGameClientShell->IsPlayerDead())
	{
		m_pStats->SetAutoTarget(LTNULL);
		return;
	}

	//see about head bite
	ModelLT* pModelLT   = g_pClientDE->GetModelLT();

	LTFLOAT fMinRange	= MAX_AUTO_TARGET_RANGE_SQ;
	HOBJECT hTarget		= LTNULL;
	HOBJECT hPlayer		= g_pLTClient->GetClientObject();
	HOBJECT hCamera		= g_pGameClientShell->GetPlayerCameraObject();

	//get our positions
	LTVector	vPlayerPos, vR, vU, vF;
	LTRotation	rRot;
	g_pLTClient->GetObjectPos(hCamera, &vPlayerPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_pLTClient->GetRotationVectors(&rRot,&vU,&vR,&vF);
	vPlayerPos += vF * 20.0;

	//targeting bools
	LTBOOL	bPredTargeting	= VisionMode::g_pButeMgr->IsPredatorTargeting(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName());
	LTBOOL	bAlienTargeting = VisionMode::g_pButeMgr->IsAlienTargeting(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName());
	LTBOOL	bHumanTargeting = VisionMode::g_pButeMgr->IsHumanTargeting(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName());

		//set targeting type based on vision mode

	CSpecialFXList* pList = m_psfxMgr->GetFXList(SFX_CHARACTER_ID);
	if (!pList) return;

	//loop through players
	for( CSpecialFXList::Iterator iter = pList->Begin();
		 iter != pList->End(); ++iter)
	{
		//assign our character pointer
		const HOBJECT hCharacter = iter->pSFX ? iter->pSFX->GetServerObj() : LTNULL;

		LTBOOL bValidType = LTFALSE;

		if(hCharacter)
		{
			const CCharacterFX * pCharacterFX = (const CCharacterFX *)iter->pSFX;
			const int nCharacterSet = pCharacterFX->m_cs.nCharacterSet;

			bValidType = bHumanTargeting & (IsHuman(nCharacterSet) || IsSynthetic(nCharacterSet));

			bValidType =	(	
							(bPredTargeting		&& IsPredator(nCharacterSet))	||
							(bAlienTargeting	&& IsAlien(nCharacterSet))	||
							(bHumanTargeting	&& IsHuman(nCharacterSet)) 	
							);
		}

		//test that we are not comparing with ourself
		if(bValidType && hPlayer != hCharacter)
		{
			uint32 dwFlags = g_pLTClient->GetObjectFlags(hCharacter);

			if(dwFlags & FLAG_VISIBLE)
			{
				//get our positions
				LTVector vCharPos;
				
				g_pLTClient->GetObjectPos(hCharacter, &vCharPos);
				
				// Do an interesect check to be sure nothing is blocking view.
				IntersectQuery	IQuery;
				IntersectInfo	IInfo;
				
				VEC_COPY(IQuery.m_From, vPlayerPos);
				VEC_COPY(IQuery.m_To, vCharPos);
				IQuery.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
				IQuery.m_FilterFn = SFXObjectFilterFn;
				
				SfxFilterStruct MyData;
				uint32 SfxTypes[2]={SFX_PROJECTILE_ID,SFX_BODYPROP_ID};
				MyData.nNumTypes	= 2;
				MyData.pTypes		= SfxTypes;

				IQuery.m_pUserData = &MyData;
				
				//test for intersect-segment
				if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
				{
					if(IInfo.m_hObject == hCharacter)
					{
						//test for infront
						LTVector vToTarget = vCharPos-vPlayerPos;
						LTFLOAT fMagSq = vToTarget.MagSqr();
						vToTarget.Norm();
						
						if(vF.Dot(vToTarget) > 0)
						{
							//we have a winner!
							if(fMagSq < fMinRange)
							{
								//final check
								LTIntPt ptTemp;

								//update the screen position of the target
								GetScreenPos(	g_pGameClientShell->GetPlayerCamera(), 
												hCharacter,
												ptTemp, LTTRUE);

								uint32 nScreenWidth, nScreenHeight;

								HSURFACE hScreen = g_pClientDE->GetScreenSurface();

								g_pClientDE->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

								if( ptTemp.x > 0 && ptTemp.x < (int)nScreenWidth
									&& ptTemp.y > 0 && ptTemp.y < (int)nScreenHeight)
								{
									fMinRange = fMagSq;
									hTarget = hCharacter;
								}
							}// if(fMagSq < fMinRange)
						}// if(vF.Dot(vToTarget) > 0)
					}// if(IInfo.m_hObject == hCharacter)
				}// if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
			}
		}
	}

	//test to see it's not the same one we already have
	if(hTarget != m_pStats->GetAutoTarget())
	{
		m_pStats->SetAutoTarget(hTarget);

		if(hTarget != LTNULL)
		{
		
			//send SFX message
			SFXCREATESTRUCT ps;
			
			m_psfxMgr->CreateSFX(SFX_PRED_TARGET_ID, &ps);

		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdateAlienTargeting()
//
//	PURPOSE:	Updates auto-targeting for the Alien
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdateAlienTargeting()
{
	//sanity check
	if(!g_pLTClient || !g_pGameClientShell || !g_pWeaponMgr || !m_pStats) return;

	WEAPON* pBiteWep	= g_pWeaponMgr->GetWeapon(m_pStats->GetWeaponFromSet(AWM_HEAD_BITE));
	WEAPON* pNormWep	= g_pWeaponMgr->GetWeapon(m_pStats->GetWeaponFromSet(AWM_NORMAL));
	WEAPON* pTearWep	= g_pWeaponMgr->GetWeapon(m_pStats->GetWeaponFromSet(AWM_TEAR));

	//test
	if(!pBiteWep || !pNormWep || !pTearWep) return;

	BARREL* pBiteBarrel		= g_pWeaponMgr->GetBarrel(pBiteWep->aBarrelTypes[PRIMARY_BARREL]);
	BARREL* pNormBarrel		= g_pWeaponMgr->GetBarrel(pNormWep->aBarrelTypes[PRIMARY_BARREL]);
	BARREL* pTearBarrel		= g_pWeaponMgr->GetBarrel(pTearWep->aBarrelTypes[PRIMARY_BARREL]);

	//test
	if(!pNormBarrel || !pBiteBarrel || !pTearBarrel) return;

	CWeaponModel* pModel = g_pGameClientShell->GetWeaponModel();

	//test
	if(!pModel) return;

	// See if we are dead
	if(g_pGameClientShell->IsPlayerDead())
	{
		//set to normal weapon
		if(pModel->GetWeapon() != pNormWep)
		{
			pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + AWM_NORMAL);
			m_pStats->SetAutoTarget(LTNULL);
			m_hBiteOrPounceTarget = LTNULL;
		}

		//kill any looped sounds
		if(m_hLockedLoopSound)
		{
			g_pLTClient->KillSound(m_hLockedLoopSound);
			m_hLockedLoopSound = LTNULL;
		}
		return;
	}

	// Dont do this if we are pounce jumping...
	WEAPON* pWep	= g_pWeaponMgr->GetWeapon(g_pGameClientShell->GetPlayerMovement()->GetCharacterButes()->m_szPounceWeapon);

	if(g_pGameClientShell->GetWeaponModel()->GetWeapon() == pWep) return;

	//see what im aiming at	
	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();	

	//get our position and rotation
	LTVector vPlayerPos;
	LTRotation rRot;
	LTVector vF, vR, vU;
	
	g_pLTClient->GetObjectPos(hCamera, &vPlayerPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_pLTClient->GetRotationVectors(&rRot,&vR,&vU,&vF);

	LTVector vEnd = vPlayerPos + (vF * (LTFLOAT)pBiteBarrel->nRange * 2);

	// Do an interesect
	IntersectQuery	IQuery;
	IntersectInfo	IInfo;
	
	VEC_COPY(IQuery.m_From, vPlayerPos);
	VEC_COPY(IQuery.m_To, vEnd);
	IQuery.m_FilterFn = BodyPropListFilterFn;
	IQuery.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	
	//test for intersect-segment
	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		if(CheckForTearTarget(IInfo, vPlayerPos, pTearBarrel->nRangeSqr))
		{
			//set to tear bite
			if(pModel->GetWeapon() != pTearWep)
			{
				pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + AWM_TEAR);
				m_hBiteOrPounceTarget = LTNULL;
			}
			return;
		}

		//find out what we hit
		uint32 nType = g_pLTClient->GetObjectType(IInfo.m_hObject);
		uint32 dwFlags = g_pLTClient->GetObjectFlags(IInfo.m_hObject);

		if(nType == OT_MODEL && dwFlags & FLAG_VISIBLE)
		{
			if(CheckForLiveAlienTarget(	IInfo.m_hObject, 
										IInfo.m_Point, 
										vPlayerPos, 
										(int)(pNormBarrel->nRangeSqr * g_cvarPounceDeadZone), 
										(int)(pBiteBarrel->nRangeSqr*g_cvarHeadBiteActivationMod),
										pBiteWep,
										vF	)	)
			{
				return;
			}
		}
	}

	//if we already have a target and have the pounce weapon selected
	//see if we should just keep this weapon
//	if( m_pStats->GetAutoTarget() && ( pLungeWep == pModel->GetWeapon()))
//		if(CheckForLungeBounds(	vPlayerPos, 
//								(int)(pNormBarrel->nRangeSqr * g_cvarPounceDeadZone.GetFloat()),
//								pLungeBarrel,
//								vF ) )
//			return;

	if(CheckForDeadAlienTarget(	vPlayerPos,
								pBiteBarrel->nRangeSqr,
								vF,
								pBiteWep))
	{
		return;
	}

	//no intersection at max range so be sure we are in normal mode
	//or we may not be pointing at a valid target
	
	m_hBiteOrPounceTarget = LTNULL;
	m_pStats->SetAutoTarget(LTNULL);

	if(pModel->GetWeapon() != pNormWep)
	{
		pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + AWM_NORMAL);
	}

	//kill any looped sounds
	if(m_hLockedLoopSound)
	{
		g_pLTClient->KillSound(m_hLockedLoopSound);
		m_hLockedLoopSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdateFacehuggerTargeting()
//
//	PURPOSE:	Updates auto-targeting for the Facehugger
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdateFacehuggerTargeting()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdateChestbursterTargeting()
//
//	PURPOSE:	Updates auto-targeting for the Chestburster
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdateChestbursterTargeting()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdatePredalienTargeting()
//
//	PURPOSE:	Updates auto-targeting for the Predalien
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdatePredalienTargeting()
{
	//sanity check
	if(!g_pLTClient || !g_pGameClientShell || !g_pWeaponMgr || !m_pStats) return;

	WEAPON* pNormWep	= g_pWeaponMgr->GetWeapon(m_pStats->GetWeaponFromSet(PAWM_NORMAL));
	WEAPON* pTearWep	= g_pWeaponMgr->GetWeapon(m_pStats->GetWeaponFromSet(PAWM_TEAR));

	//test
	if(!pNormWep || !pTearWep) return;

	BARREL* pNormBarrel		= g_pWeaponMgr->GetBarrel(pNormWep->aBarrelTypes[PRIMARY_BARREL]);
	BARREL* pTearBarrel		= g_pWeaponMgr->GetBarrel(pTearWep->aBarrelTypes[PRIMARY_BARREL]);

	//test
	if(!pNormBarrel || !pTearBarrel) return;

	CWeaponModel* pModel = g_pGameClientShell->GetWeaponModel();
	//test
	if(!pModel) return;

	// See if we are dead
	if(g_pGameClientShell->IsPlayerDead())
	{
		//set to normal weapon
		if(pModel->GetWeapon() != pNormWep)
		{
			pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + PAWM_NORMAL);
			m_pStats->SetAutoTarget(LTNULL);
			m_hBiteOrPounceTarget = LTNULL;
		}

		//kill any looped sounds
		if(m_hLockedLoopSound)
		{
			g_pLTClient->KillSound(m_hLockedLoopSound);
			m_hLockedLoopSound = LTNULL;
		}
		return;
	}

	//see what im aiming at	
	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();	

	//get our position and rotation
	LTVector vPlayerPos;
	LTRotation rRot;
	LTVector vF, vR, vU;
	
	g_pLTClient->GetObjectPos(hCamera, &vPlayerPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_pLTClient->GetRotationVectors(&rRot,&vR,&vU,&vF);

	LTVector vEnd = vPlayerPos + (vF * (LTFLOAT)pNormBarrel->nRange * 2);


	// Do an interesect
	IntersectQuery	IQuery;
	IntersectInfo	IInfo;
	
	VEC_COPY(IQuery.m_From, vPlayerPos);
	VEC_COPY(IQuery.m_To, vEnd);
	IQuery.m_FilterFn = BodyPropListFilterFn;
	IQuery.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	
	//test for intersect-segment
	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		if(CheckForTearTarget(IInfo, vPlayerPos, pTearBarrel->nRangeSqr))
		{
			//set to tear bite
			if(pModel->GetWeapon() != pTearWep)
			{
				pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + PAWM_TEAR);
				m_hBiteOrPounceTarget = LTNULL;
			}
			return;
		}

		//find out what we hit
		uint32 nType = g_pLTClient->GetObjectType(IInfo.m_hObject);
		uint32 dwFlags = g_pLTClient->GetObjectFlags(IInfo.m_hObject);

		if(nType == OT_MODEL && dwFlags & FLAG_VISIBLE)
		{
			if(CheckForLivePredalienTarget(	IInfo.m_hObject, 
											IInfo.m_Point, 
											vPlayerPos, 
											(int)(pNormBarrel->nRangeSqr * g_cvarPounceDeadZone),
											vF	)	)
			{
				//test to see it's not the same one we already have
//				if(IInfo.m_hObject != m_pStats->GetAutoTarget())
//				{
//					m_pStats->SetAutoTarget(IInfo.m_hObject);
//				}
				return;
			}
		}
	}

	//if we already have a target and have the pounce weapon selected
	//see if we should just keep this weapon
//	if( m_pStats->GetAutoTarget() && ( pLungeWep == pModel->GetWeapon()))
//		if(CheckForLungeBounds(	vPlayerPos, 
//								(int)(pNormBarrel->nRangeSqr * g_cvarPounceDeadZone.GetFloat()),
//								pLungeBarrel,
//								vF ) )
//			return;

	//no intersection at max range so be sure we are in normal mode
	//or we may not be pointing at a valid target
	
	m_hBiteOrPounceTarget = LTNULL;
	m_pStats->SetAutoTarget(LTNULL);

	if(pModel->GetWeapon() != pNormWep)
	{
		pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + PAWM_NORMAL);
//		m_pStats->SetAutoTarget(LTNULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdateRunnerTargeting()
//
//	PURPOSE:	Updates auto-targeting for the Runner
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdateRunnerTargeting()
{
	UpdateAlienTargeting();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::IsValidAlienTarget()
//
//	PURPOSE:	Checks to see if the target is a vaild
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::IsValidAlienTarget(HOBJECT hTarget)
{
	HOBJECT hPlayer		= g_pLTClient->GetClientObject();

	//sanity check
	if(!hPlayer || !hTarget) return LTFALSE;

	CSpecialFXList* pList = m_psfxMgr->GetFXList(SFX_CHARACTER_ID);
    if (!pList) return LTFALSE;


	for( CSpecialFXList::Iterator iter = pList->Begin();
		 iter != pList->End(); ++iter)
	{
		if(iter->pSFX)
		{
			HOBJECT hCharacter = iter->pSFX->GetServerObj();

			if(hCharacter)
			{
				if((hTarget == hCharacter) && (hPlayer != hCharacter))
				{
					// Make sure we don't target other aliens...
					CCharacterFX* pCharFX = dynamic_cast<CCharacterFX*>(iter->pSFX);

					if(pCharFX && (IsAlien(pCharFX->m_cs.nCharacterSet) || IsSynthetic(pCharFX->m_cs.nCharacterSet)))
							return LTFALSE;

					return LTTRUE;
				}
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::IsValidFacehuggerTarget()
//
//	PURPOSE:	Checks to see if the target is a vaild
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::IsValidFacehuggerTarget(HOBJECT hTarget)
{
	HOBJECT hPlayer		= g_pLTClient->GetClientObject();

	//sanity check
	if(!hPlayer || !hTarget) return LTFALSE;

	CSpecialFXList* pList = m_psfxMgr->GetFXList(SFX_CHARACTER_ID);
    if (!pList) return LTFALSE;

	for( CSpecialFXList::Iterator iter = pList->Begin();
		 iter != pList->End(); ++iter)
	{
		if(iter->pSFX)
		{
			HOBJECT hCharacter = iter->pSFX->GetServerObj();

			if(hCharacter)
			{
				if((hTarget == hCharacter) && (hPlayer != hCharacter))
				{
					CCharacterFX* pSfx = (CCharacterFX*)iter->pSFX;
					if( IsHuman( pSfx->m_cs.nCharacterSet) )
						return LTTRUE;
					else
						return LTFALSE;
				}
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CheckForLungeBounds()
//
//	PURPOSE:	Checks to see if the target is a vaild and sets weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::CheckForLungeBounds(		LTVector	vPlayerPos, 
												int			nNormalRngSqr,
												BARREL*		pLungeBarrel,
												LTVector	vF)
{
	//check range
	LTVector vToTarget = m_vPounceTargetLoc - vPlayerPos;
	int nRange = (int)vToTarget.MagSqr();

	if( (nRange  > nNormalRngSqr) && (nRange < pLungeBarrel->nRangeSqr) )
	{
		vToTarget.Norm();

		LTFLOAT fDot = vToTarget.Dot(vF);

		if( fDot > g_cvarPounceTargetLock)
		{
			//exit here
			return LTTRUE;
		}
	}

	//no good target
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CheckForLiveAlienTarget()
//
//	PURPOSE:	Checks to see if the target is a vaild and sets weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::CheckForLiveAlienTarget(	HOBJECT		hTarget, 
												LTVector	vTargetLoc, 
												LTVector	vPlayerPos, 
												int			nNormalRngSqr,
												int			nBiteRngSqr,
												WEAPON*		pBiteWep,
												LTVector	vF)
{
	//we hit some kind of model
	if(IsValidAlienTarget(hTarget))
	{
		//we are aimed at a valid target
		//check range and pick weapon
		int nRange = (int)(vTargetLoc-vPlayerPos).MagSqr();
		CharacterMovementState cms = g_pGameClientShell->GetPlayerMovement()->GetMovementState();

		if(nRange <= nBiteRngSqr && cms != CMS_POUNCING)
		{
			//see about head bite
			ModelLT* pModelLT   = g_pClientDE->GetModelLT();

			//see if this character has a head
			HMODELPIECE hHeadPiece;
			pModelLT->GetPiece(hTarget, "Head", hHeadPiece);
			LTBOOL bHidden;
			pModelLT->GetPieceHideStatus(hTarget, hHeadPiece, bHidden);

			//no head, no head_bite for you!
			if(bHidden) return LTFALSE;

			char* pSocketName	= "Head";

			HMODELSOCKET hSocket;

			if (pModelLT->GetSocket(hTarget, pSocketName, hSocket) == LT_OK)
			{
				LTransform	tTransform;
				LTVector	vHeadPos;

				if (pModelLT->GetSocketTransform(hTarget, hSocket, tTransform, DTRUE) == LT_OK)
				{
					TransformLT* pTransLT = g_pClientDE->GetTransformLT();
					pTransLT->GetPos(tTransform, vHeadPos);
				}

				LTVector vToHead = vHeadPos - vPlayerPos;
				vToHead.Norm();

				if(vToHead.Dot(vF) > 0.995)
				{
					//set to head bite
					CWeaponModel* pModel = g_pGameClientShell->GetWeaponModel();

					WEAPON* pRequestedWep = pModel->GetRequestedWeapon();
					WEAPON* pCurWep = pModel->GetWeapon();
					
					if(m_hBiteOrPounceTarget != hTarget && (pCurWep != pBiteWep && pRequestedWep != pBiteWep) )
					{
						pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + AWM_HEAD_BITE);
						m_hBiteOrPounceTarget = hTarget;
						if(g_pLTClient->GetTime() - m_fLastBiteSound > 3.0f)
						{
							g_pClientSoundMgr->PlaySoundLocal("bitetarget");
							m_fLastBiteSound = g_pLTClient->GetTime();
						}
					}
					//exit here
					return LTTRUE;
				}
			}
		}
	}

	//no good target
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CheckForDeadAlienTarget()
//
//	PURPOSE:	Checks to see if the target is a vaild and sets weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::CheckForDeadAlienTarget( LTVector	vPlayerPos,
											   int			nBiteRngSqr,
											   LTVector		vF,
											   WEAPON*		pBiteWep)
{
	//assign our pointers
	CSpecialFXList* pList		= m_psfxMgr->GetFXList(SFX_BODYPROP_ID);
	ModelLT*		pModelLT	= g_pClientDE->GetModelLT();
	TransformLT*	pTransLT	= g_pClientDE->GetTransformLT();
	CWeaponModel*	pModel		= g_pGameClientShell->GetWeaponModel();
	
    if (!pList || !pModelLT || !pTransLT || !pModel) return LTFALSE;
	
	char* pSocketName	= "Head";
	
	HMODELSOCKET	hSocket;
	LTVector		vHeadPos;
	LTransform		tTransform;
	HMODELPIECE		hHeadPiece;
	LTBOOL			bHidden;
	
	//check bodies for head bite
	
	//loop through characters
	for( CSpecialFXList::Iterator iter = pList->Begin();
	iter != pList->End(); ++iter)
	{
		//get the server object
		const HOBJECT hBody	= iter->pSFX ? iter->pSFX->GetServerObj() : LTNULL;
		
		if(hBody)
		{
			BodyPropFX* pBodyFX = dynamic_cast<BodyPropFX*>(iter->pSFX);
			
			if(pBodyFX && (!IsAlien(pBodyFX->m_cs.nCharacterSet) && !IsSynthetic(pBodyFX->m_cs.nCharacterSet)) )
			{
				//get head position
				if (pModelLT->GetSocket(hBody, pSocketName, hSocket) == LT_OK)
				{
					if (pModelLT->GetSocketTransform(hBody, hSocket, tTransform, DTRUE) == LT_OK)
					{
						pTransLT->GetPos(tTransform, vHeadPos);
						
						//see if this corpse has a head
						pModelLT->GetPiece(hBody, "Head", hHeadPiece);
						pModelLT->GetPieceHideStatus(hBody, hHeadPiece, bHidden);
						
						//no head, no head_bite for you!
						if(!bHidden)
						{
							int nRange = (int)(vHeadPos-vPlayerPos).MagSqr();
							
							//check range and pick weapon
							if(nRange <= nBiteRngSqr)
							{
								//see about head bite
								LTVector vToHead = vHeadPos - vPlayerPos;
								vToHead.Norm();
								
								if(vToHead.Dot(vF) > 0.995)
								{
									WEAPON* pRequestedWep = pModel->GetRequestedWeapon();
									WEAPON* pCurWep = pModel->GetWeapon();
									
									//set to head bite
									if(pCurWep != pBiteWep && pRequestedWep != pBiteWep)
									{
										pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + AWM_HEAD_BITE);
										m_hBiteOrPounceTarget = hBody;
										if(g_pLTClient->GetTime() - m_fLastBiteSound > 3.0f)
										{
											g_pClientSoundMgr->PlaySoundLocal("bitetarget");
											m_fLastBiteSound = g_pLTClient->GetTime();
										}
									}
									//exit here
									return LTTRUE;
								}
							}
						}
					}
				}
			}
		}
	}
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CheckForTearTarget()
//
//	PURPOSE:	Checks to see if the target is a vaild and sets weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::CheckForTearTarget(IntersectInfo IInfo, LTVector vPos, uint32 nRngSq)
{
	//assign our pointers
	CWeaponModel*	pModel = g_pGameClientShell->GetWeaponModel();

    if (!pModel) return LTFALSE;

	if(GetActivateType(IInfo.m_hObject) == AT_TEAR)
	{
		//now check range
		LTVector vToTarget = IInfo.m_Point - vPos;

		if(vToTarget.MagSqr() < nRngSq)
			//exit here
			return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CheckForLiveFacehuggerTarget()
//
//	PURPOSE:	Checks to see if the target is a vaild and sets weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::CheckForLiveFacehuggerTarget	(	HOBJECT		hTarget,
														LTVector	vTargetLoc,
														LTVector	vPlayerPos,
														int			nNormalRngSqr,
														WEAPON*		pLungeWep,
														LTVector	vF			)
{
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CheckForLivePredalienTarget()
//
//	PURPOSE:	Checks to see if the target is a vaild and sets weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CAutoTargetMgr::CheckForLivePredalienTarget	(	HOBJECT		hTarget, 
														LTVector	vTargetLoc, 
														LTVector	vPlayerPos, 
														int			nNormalRngSqr,
														LTVector	vF			)
{
	//we hit some kind of model
//	if(IsValidAlienTarget(hTarget))
//	{
//		//we are aimed at a valid target
//		//check range and pick weapon
//		int nRange = (int)(vTargetLoc-vPlayerPos).MagSqr();
//
//		if(nRange > nNormalRngSqr)
//		{
//			//set to lunge weapon
//			CWeaponModel* pModel = g_pGameClientShell->GetWeaponModel();
//
//			if(pModel->GetWeapon() != pLungeWep)
//			{
//				pModel->ChangeWeapon(COMMAND_ID_WEAPON_BASE + PAWM_POUNCE);
//				m_hBiteOrPounceTarget = hTarget;
//			}
//
//			if(hTarget != m_pStats->GetAutoTarget())
//			{
//				m_pStats->SetAutoTarget(hTarget);
//			}
//
//			//exit here
//			return LTTRUE;
//		}
//	}

	//no good target
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdateMarineSmartgun()
//
//	PURPOSE:	Updates auto-targeting for the Marine Smartgun
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdateMarineSmartgun()
{
	//sanity check
	if(!m_psfxMgr) return;
	
	// See if we are dead
	if(g_pGameClientShell->IsPlayerDead())
	{
		m_pStats->SetAutoTarget(LTNULL);
		
		//green crosshair
		g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 0.0, 1.0, 0.0, 1.0);
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);
		
		//kill any looped sounds
		if(m_hLockedLoopSound)
		{
			g_pLTClient->KillSound(m_hLockedLoopSound);
			m_hLockedLoopSound = LTNULL;
		}
		return;
	}
	
	//see about head bite
	ModelLT* pModelLT   = g_pClientDE->GetModelLT();
	
	LTFLOAT fMinRange	= MAX_AUTO_TARGET_RANGE_SQ;
	HOBJECT hTarget		= LTNULL;
	HOBJECT hPlayer		= g_pLTClient->GetClientObject();
	HOBJECT hCamera		= g_pGameClientShell->GetPlayerCameraObject();
	LTIntPt ptTarget(0,0);
	
	//targeting bools
	if(VisionMode::g_pButeMgr->IsMarineNightVision(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName()))
	{
		//doesnt work in nightvision
		
		// white crosshair
		g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 1.0, 1.0, 1.0, 1.0);
		
		// re-set the target
		m_pStats->SetAutoTarget(LTNULL);
		
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);
		
		//kill any sounds
		if(m_hLockedLoopSound)
		{
			g_pLTClient->KillSound(m_hLockedLoopSound);
			m_hLockedLoopSound = LTNULL;
		}
		return;
	}
	
	//set targeting type based on vision mode
	
	CSpecialFXList* pList = m_psfxMgr->GetFXList(SFX_CHARACTER_ID);
    if (!pList) return;
	
	//loop through players
	for( CSpecialFXList::Iterator iter = pList->Begin();
		 iter != pList->End(); ++iter)
	{
		//assign our character pointer
		const HOBJECT hCharacter = iter->pSFX ? iter->pSFX->GetServerObj() : LTNULL;
		
		//test that we are not comparing with ourself
		if(hCharacter && hPlayer != hCharacter)
		{
			uint32 dwFlags = g_pLTClient->GetObjectFlags(hCharacter);
			
			if(dwFlags & FLAG_VISIBLE)
			{
				//get our positions
				LTVector vCharPos, vPlayerPos;
				
				g_pLTClient->GetObjectPos(hCamera, &vPlayerPos);
				g_pLTClient->GetObjectPos(hCharacter, &vCharPos);
				
				// Do an interesect check to be sure nothing is blocking view.
				IntersectQuery	IQuery;
				IntersectInfo	IInfo;

				// Find our direction...
				LTVector vDir = vCharPos - vPlayerPos;
				vDir.Norm();

				// Set our max range...
				vDir *= g_cvarMaxSGRange;
				
				VEC_COPY(IQuery.m_From, vPlayerPos);
				VEC_COPY(IQuery.m_To, vPlayerPos + vDir);
				IQuery.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
				IQuery.m_FilterFn = SFXObjectFilterFn;
				
				SfxFilterStruct MyData;
				uint32 SfxTypes[2]={SFX_PROJECTILE_ID,SFX_BODYPROP_ID};
				MyData.nNumTypes		 = 2;
				MyData.pTypes			= SfxTypes;
				MyData.bFilterFriends	= LTTRUE;
				
				IQuery.m_pUserData = &MyData;
				
				//test for intersect-segment
				if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
				{
					if(IInfo.m_hObject == hCharacter)
					{
						CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_CHARACTER_ID, hCharacter);
						if(pFX)
						{
							CCharacterFX* pCharFX = dynamic_cast<CCharacterFX*>(pFX);
							
							if(pCharFX)
							{
								if(hCharacter == m_pStats->GetAutoTarget() || pCharFX->IsMoving())
								{
									//test for infront
									LTVector vToTarget = vCharPos-vPlayerPos;
									LTFLOAT fMagSq = vToTarget.MagSqr();
									vToTarget.Norm();
									
									LTRotation rRot;
									g_pLTClient->GetObjectRotation(hCamera, &rRot);
									
									LTVector vF, vR, vU;
									g_pLTClient->GetRotationVectors(&rRot,&vR,&vU,&vF);
									
									if(vF.Dot(vToTarget) > 0)
									{
										//we have a winner!
										if(fMagSq < fMinRange)
										{
											//final check
											LTIntPt ptTemp;
											
											//update the screen position of the target
											GetScreenPos(	g_pGameClientShell->GetPlayerCamera(), 
												hCharacter,
												ptTemp, LTTRUE);
											
											uint32 nScreenWidth, nScreenHeight;
											
											HSURFACE hScreen = g_pClientDE->GetScreenSurface();
											
											g_pClientDE->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);
											
											if( ptTemp.x > nScreenWidth*0.3 && ptTemp.x < nScreenWidth*0.7
												&& ptTemp.y > nScreenHeight*0.3 && ptTemp.y < nScreenHeight*0.7)
											{
												fMinRange = fMagSq;
												hTarget = hCharacter;
												ptTarget = ptTemp;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if(!hTarget)
	{
		//green crosshair
		g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 0.0, 1.0, 0.0, 1.0);
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);

		//kill any looped sounds
		if(m_hLockedLoopSound)
		{
			g_pLTClient->KillSound(m_hLockedLoopSound);
			m_hLockedLoopSound = LTNULL;
		}
	}
	else
	{
		//we have a target
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTTRUE);
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPos(ptTarget);
	}

	//test to see it's not the same one we already have
	if(hTarget != m_pStats->GetAutoTarget())
	{
		m_pStats->SetAutoTarget(hTarget);


		if(hTarget != LTNULL)
		{
			//red crosshair
			g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 1.0, 0.0, 0.0, 1.0);

			if(!m_hLockedLoopSound)
				m_hLockedLoopSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Weapons\\Marine\\sadar\\sadarlock.wav", SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::UpdateMarineSADAR()
//
//	PURPOSE:	Updates auto-targeting for the Marine Smartgun
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::UpdateMarineSADAR()
{
	//sanity check
	if(!m_psfxMgr) return;

	// See if we are dead
	if(g_pGameClientShell->IsPlayerDead())
	{
		m_pStats->SetAutoTarget(LTNULL);

		//green crosshair
		g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 0.0, 1.0, 0.0, 1.0);
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);
		m_fNewTargetStartTime = 0.0f;
		m_hPotentialTarget = LTNULL;
		m_bLockSoundPlayed = LTFALSE;


		//kill any looped sounds
		if(m_hLockedLoopSound)
		{
			g_pLTClient->KillSound(m_hLockedLoopSound);
			m_hLockedLoopSound = LTNULL;
		}
		return;
	}

	//see about head bite
	ModelLT* pModelLT   = g_pClientDE->GetModelLT();

	LTFLOAT fMinRange	= MAX_AUTO_TARGET_RANGE_SQ;
	HOBJECT hTarget		= LTNULL;
	HOBJECT hPlayer		= g_pLTClient->GetClientObject();
	HOBJECT hCamera		= g_pGameClientShell->GetPlayerCameraObject();
	CWeaponModel* pWep	= g_pGameClientShell->GetWeaponModel();
	LTBOOL	bCanLock	= !pWep->IsOutOfAmmoOrReloading();
	LTIntPt ptTarget(0,0);

	//set targeting type based on vision mode

	CSpecialFXList* pList = m_psfxMgr->GetFXList(SFX_CHARACTER_ID);
    if (!pList) return;

	if(bCanLock)
	{
		//loop through players
		for( CSpecialFXList::Iterator iter = pList->Begin();
			 iter != pList->End(); ++iter)
		{
			//assign our character pointer
			const HOBJECT hCharacter = iter->pSFX ? iter->pSFX->GetServerObj() : LTNULL;

			//test that we are not comparing with ourself
			if(hCharacter && hPlayer != hCharacter)
			{
				uint32 dwFlags = g_pLTClient->GetObjectFlags(hCharacter);

				if(dwFlags & FLAG_VISIBLE)
				{
					//get our positions
					LTVector vCharPos, vPlayerPos;
					
					g_pLTClient->GetObjectPos(hCamera, &vPlayerPos);
					g_pLTClient->GetObjectPos(hCharacter, &vCharPos);
					
					// Do an interesect check to be sure nothing is blocking view.
					IntersectQuery	IQuery;
					IntersectInfo	IInfo;
					
					VEC_COPY(IQuery.m_From, vPlayerPos);
					VEC_COPY(IQuery.m_To, vCharPos);
					IQuery.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
					IQuery.m_FilterFn = SFXObjectFilterFn;

					SfxFilterStruct MyData;
					uint32 SfxTypes[2]={SFX_PROJECTILE_ID,SFX_BODYPROP_ID};
					MyData.nNumTypes	= 2;
					MyData.pTypes		= SfxTypes;

					IQuery.m_pUserData = &MyData;
					
					//test for intersect-segment
					if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
					{
						if(IInfo.m_hObject == hCharacter)
						{
							//test for infront
							LTVector vToTarget = vCharPos-vPlayerPos;
							vToTarget.Norm();
							
							LTRotation rRot;
							g_pLTClient->GetObjectRotation(hCamera, &rRot);
							
							LTVector vF, vR, vU;
							g_pLTClient->GetRotationVectors(&rRot,&vR,&vU,&vF);
							
							if(vF.Dot(vToTarget) > 0)
							{
								//final check
								LTIntPt ptTemp;

								//update the screen position of the target
								GetScreenPos(	g_pGameClientShell->GetPlayerCamera(), 
												hCharacter,
												ptTemp, LTTRUE);

								uint32 nScreenWidth, nScreenHeight;
								LTFLOAT nHalfWidth, nHalfHeight;

								HSURFACE hScreen = g_pClientDE->GetScreenSurface();

								g_pClientDE->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

								nHalfWidth = (LTFLOAT)(nScreenWidth>>1);
								nHalfHeight = (LTFLOAT)(nScreenHeight>>1);

								if( ptTemp.x > nScreenWidth*0.3 && ptTemp.x < nScreenWidth*0.7
									&& ptTemp.y > nScreenHeight*0.3 && ptTemp.y < nScreenHeight*0.7)
								{
									LTFLOAT fX = (LTFLOAT)(ptTemp.x - nHalfWidth);
									LTFLOAT fY = (LTFLOAT)(ptTemp.y - nHalfHeight);

									LTFLOAT fMagSq = (fX*fX) + (fY*fY);

									if(fMagSq < fMinRange)
									{
										fMinRange = fMagSq;
										hTarget = hCharacter;
										ptTarget = ptTemp;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if(!hTarget)
	{
		//green crosshair
		g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 0.0, 1.0, 0.0, 1.0);
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);
		m_fNewTargetStartTime = 0.0f;
		m_hPotentialTarget = LTNULL;
		m_bLockSoundPlayed = LTFALSE;

		if(m_hLockedLoopSound)
		{
			g_pLTClient->KillSound(m_hLockedLoopSound);
			m_hLockedLoopSound = LTNULL;
		}
	}
	else
	{
		if(hTarget == m_hPotentialTarget)
		{
			//see if we have been on this target long enuf
			if(g_pLTClient->GetTime()-m_fNewTargetStartTime > g_cvarSADARLockTime)
			{
				//its time!
				g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTTRUE);
				g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPos(ptTarget);

				if(!m_hLockedLoopSound)
				{
					m_hLockedLoopSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Weapons\\Marine\\sadar\\sadarlock.wav", SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
				}
			}
			else
			{
				//need to wait just a bit more
				return;
			}
		}
		else
		{
			//new prospect
			m_hPotentialTarget = hTarget;
			m_fNewTargetStartTime = g_pLTClient->GetTime();
			g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 1.0, 1.0, 0.0, 1.0);
			g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);
			m_bLockSoundPlayed = LTFALSE;
			if(m_hLockedLoopSound)
			{
				g_pLTClient->KillSound(m_hLockedLoopSound);
				m_hLockedLoopSound = LTNULL;
			}
			return;
		}
	}

	//test to see it's not the same one we already have
	if(hTarget != m_pStats->GetAutoTarget())
	{
		m_pStats->SetAutoTarget(hTarget);

		if(hTarget != LTNULL)
		{
			//red crosshair
			g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 1.0, 0.0, 0.0, 1.0);
		}
		else
		{
			if(m_hLockedLoopSound)
			{
				g_pLTClient->KillSound(m_hLockedLoopSound);
				m_hLockedLoopSound = LTNULL;
			}
		}
	}
}
