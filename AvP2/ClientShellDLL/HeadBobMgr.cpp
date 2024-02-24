// ----------------------------------------------------------------------- //
//
// MODULE  : HeadBobMgr.cpp
//
// PURPOSE : Head Bob Mgr - Implementation
//
// CREATED : 01/09/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HeadBobMgr.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "CharacterFX.h"

CHeadBobMgr* g_pHeadBobMgr = LTNULL;

extern CGameClientShell* g_pGameClientShell;
extern VarTrack	g_vtMaxVehicleYawDiff;
extern SurfaceType g_eClientLastSurfaceType;

VarTrack		g_vtRunPaceAdjust;
VarTrack		g_vtWalkPaceAdjust;
VarTrack		g_vtBobDecayTime;
VarTrack		g_vtBobV;
VarTrack		g_vtSwayH;
VarTrack		g_vtSwayV;
VarTrack		g_vtMaxBobAmp;
VarTrack		g_vtRollAdjust;
VarTrack		g_vtHeadBobAdjust;
VarTrack		g_vtWeaponSway;

static LTFLOAT   s_fBobDecayStartTime = -1.0f;
static LTFLOAT   s_fBobStartTime      = -1.0f;
static LTBOOL    s_bCanDoLeftFootstep = LTTRUE;
static LTBOOL    s_bCanDoRightFootstep = LTFALSE;



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::CHeadBobMgr
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

CHeadBobMgr::CHeadBobMgr()
{
	g_pHeadBobMgr	= this;

	m_fBobHeight	= 0.0f;
	m_fBobAmp		= 0.0f;
	m_fBobPhase		= 0.0f;
	m_fSwayPhase	= 0.0f;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::Init
//
//	PURPOSE:	Init
//
// --------------------------------------------------------------------------- //

LTBOOL CHeadBobMgr::Init()
{
    g_vtRunPaceAdjust.Init(g_pLTClient, "BobSwayRunPaceAdjust", NULL, 1.5f);
    g_vtWalkPaceAdjust.Init(g_pLTClient, "BobSwayWalkPaceAdjust", NULL, 1.0f);

    g_vtBobDecayTime.Init(g_pLTClient, "BobDecayTime", NULL, 0.1f);
    g_vtBobV.Init(g_pLTClient, "BobV", NULL, 0.45f);
    g_vtSwayH.Init(g_pLTClient, "SwayH", NULL, 0.005f);
    g_vtSwayV.Init(g_pLTClient, "SwayV", NULL, 0.002f);
    g_vtMaxBobAmp.Init(g_pLTClient, "MaxBobAmp", NULL, 10.0f);

    g_vtRollAdjust.Init(g_pLTClient, "BobRollAdjust", NULL, 0.005f);
    g_vtHeadBobAdjust.Init(g_pLTClient, "HeadBob", NULL, 1.0f);
    g_vtWeaponSway.Init(g_pLTClient, "WeaponSway", NULL, 1.0f);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::Update
//
//	PURPOSE:	Update all variables
//
// --------------------------------------------------------------------------- //

void CHeadBobMgr::Update()
{
	// We check CanDoFootstep instead of on ground since CanDoFootstep
	// handles stairs much better...

	if (CanDoFootstep() && g_pGameClientShell->IsFirstPerson())
	{
		UpdateHeadBob();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::CanDoFootstep
//
//	PURPOSE:	Can we make a footstep sound or footprint?
//
// ----------------------------------------------------------------------- //

LTBOOL CHeadBobMgr::CanDoFootstep()
{
	PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();
	if(!pMovement) return LTFALSE;

	switch (pMovement->GetMovementState())
	{
	case (CMS_WALKING):						// Walking in any direction *
	case (CMS_RUNNING):						// Running in any direction *
	case (CMS_PRE_CRAWLING):				// Start the crawling state
	case (CMS_CRAWLING):					// Ducking down or crawling
	case (CMS_POST_CRAWLING):				// After the crawling state is done
	case (CMS_WALLWALKING):					// Walk around the edge of the walls
	case (CMS_CLIMBING):					// Contained within a ladder brush
		return LTTRUE;

	case (CMS_IDLE):						// Generally when the velocity is zero
	case (CMS_SWIMMING):					// Swimming below the surface of a liquid
	case (CMS_POST_CLIPPING):				// What to do after we stop clipping
	case (CMS_STAND_LAND):					// Landed on something after a falling state
	case (CMS_WALK_LAND):					// Landed on something after a falling state
	case (CMS_RUN_LAND):					// Landed on something after a falling state
	case (CMS_WWALK_LAND):					// Landed on something after a falling state
	case (CMS_PRE_JUMPING):					// Start the jump process **
	case (CMS_PRE_POUNCE_JUMP):				// Start the pounce jump process **
	case (CMS_PRE_FALLING):					// Start the falling process
	case (CMS_POUNCE_JUMP):					// Increasing height **
	case (CMS_STAND_JUMP):					// Increasing height **
	case (CMS_WALK_JUMP):					// Increasing height **
	case (CMS_RUN_JUMP):					// Increasing height **
	case (CMS_WWALK_JUMP):					// Increasing height **
	case (CMS_STAND_FALL):					// Decreasing height **
	case (CMS_WALK_FALL):					// Decreasing height **
	case (CMS_RUN_FALL):					// Decreasing height **
	case (CMS_WWALK_FALL):					// Decreasing height **
	case (CMS_POUNCE_FALL):					// Decreasing height **
	case (CMS_FLYING):						// Free movement through the air
	case (CMS_SPECIAL):						// Any special movement state
	case (CMS_CLIPPING):					// Clip through geometry
	case (CMS_POUNCING):					// Pouncing alien attack
	case (CMS_FACEHUG):						// During the facehug attack
	case (CMS_FACEHUG_ATTACH):				// During the facehug implantation
	default:
		return LTFALSE;		
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::UpdateHeadBob
//
//	PURPOSE:	Adjusts the head bobbing & swaying
//
// --------------------------------------------------------------------------- //

void CHeadBobMgr::UpdateHeadBob()
{
	if (g_pGameClientShell->IsGamePaused()) return;

    uint32 dwPlayerFlags = g_pGameClientShell->GetPlayerFlags();

	PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();
	if(!pMovement) return;

	// This frame time is used since unlike ClientDE::GetFrameTime() the
	// max value is controlled by the game...

    LTFLOAT fFrameTime = g_pGameClientShell->GetFrameTime();

    LTFLOAT fTime      = g_pLTClient->GetTime();
    LTBOOL	bRunning    = (pMovement->GetCharacterVars()->m_dwCtrl & CM_FLAG_RUN);
    LTFLOAT fMoveDist  = pMovement->GetVelocity().Mag() * fFrameTime;

    LTBOOL bFootstep = LTFALSE;
	LTBOOL bLeftFoot = LTFALSE;
    LTFLOAT fPace = 0.0f;

    if (bRunning)
	{
		fPace = MATH_CIRCLE * g_vtRunPaceAdjust.GetFloat();
	}
	else
	{
		fPace = MATH_CIRCLE * g_vtWalkPaceAdjust.GetFloat();
	}

	// Make sure bob phase and sway phase start at the right values...

	if (m_fBobAmp == 0.0f)
	{
		m_fBobPhase  = 0.0f;
		m_fSwayPhase = 0.0f;
	}
	else  // Normal processing...
	{
		// Bob phase should be between MATH_PI and MATH_CIRCLE so that the
		// sin(m_fBobPhase) is always between -1 and 0...

		m_fBobPhase += (fFrameTime * fPace);

		if (m_fBobPhase > MATH_CIRCLE)
		{
			m_fBobPhase -= MATH_PI;
		}
		else if (m_fBobPhase < MATH_PI)
		{
			m_fBobPhase += MATH_PI;
		}

		m_fSwayPhase += (fFrameTime * fPace);

		if (m_fSwayPhase > MATH_CIRCLE)
		{
			m_fSwayPhase -= MATH_CIRCLE;
		}
	}


	// See if it is time to play a footstep sound...

	if ((m_fSwayPhase > MATH_CIRCLE * 0.25f) &&
		(m_fSwayPhase <= MATH_CIRCLE * 0.75f))
	{
		if (s_bCanDoLeftFootstep)
		{
			bLeftFoot = LTFALSE;
            bFootstep = LTTRUE;
            s_bCanDoLeftFootstep = LTFALSE;
            s_bCanDoRightFootstep = LTTRUE;
		}
	}
	else if (m_fSwayPhase > MATH_CIRCLE * 0.75f)
	{
		if (s_bCanDoRightFootstep)
		{
			bLeftFoot = LTTRUE;
            bFootstep = LTTRUE;
            s_bCanDoLeftFootstep = LTTRUE;
            s_bCanDoRightFootstep = LTFALSE;
		}
	}


    LTBOOL bMoving = (pMovement->GetCharacterVars()->m_dwCtrl & CM_FLAG_DIRECTIONAL);
    LTFLOAT t;

	// If we're not moving, decay the head bob...

	if (!bMoving)
	{
		s_fBobStartTime = -1.0f;

		if (s_fBobDecayStartTime < 0.0f)
		{
			// Calculate what the current bobamp percent is...

			t = (1.0f - m_fBobAmp / g_vtMaxBobAmp.GetFloat());

			s_fBobDecayStartTime = fTime - (g_vtBobDecayTime.GetFloat() * t);
		}

        LTFLOAT fDur = (fTime - s_fBobDecayStartTime);
		if (fDur <= g_vtBobDecayTime.GetFloat())
		{
			t = fDur / g_vtBobDecayTime.GetFloat();	// 0 to 1
			t = WaveFn_SlowOff(t);
			t = 1.0f - t;				// 1 to 0

			m_fBobAmp = t * g_vtMaxBobAmp.GetFloat();

			if (m_fBobAmp < 0.0f)
			{
				m_fBobAmp = 0.0f;
			}
		}
		else
		{
			m_fBobAmp = 0.0f;
		}
	}
	else  // We're moving...
	{
		s_fBobDecayStartTime = -1.0f;

		// If we just started bobing, ramp up the bob...

		if (s_fBobStartTime < 0.0f)
		{
			// Calculate what the current bobamp percent is...

			t = m_fBobAmp / g_vtMaxBobAmp.GetFloat();

			s_fBobStartTime = fTime - (g_vtBobDecayTime.GetFloat() * t);
		}

        LTFLOAT fDur = (fTime - s_fBobStartTime);
		if (fDur <= g_vtBobDecayTime.GetFloat())
		{
			t = fDur / g_vtBobDecayTime.GetFloat();	// 0 to 1
			t = WaveFn_SlowOn(t);

			m_fBobAmp = t * g_vtMaxBobAmp.GetFloat();

			if (m_fBobAmp > g_vtMaxBobAmp.GetFloat())
			{
				m_fBobAmp = g_vtMaxBobAmp.GetFloat();
			}
		}
		else
		{
			m_fBobAmp = g_vtMaxBobAmp.GetFloat();
		}
	}


	// Update the bob...
	m_fBobHeight = g_vtBobV.GetFloat() * m_fBobAmp * (float)sin(m_fBobPhase);

	LTBOOL bDoBob = (LTBOOL)GetConsoleInt("HeadBob",1);
	if(bDoBob)
		CameraMgrFX_SetHeadBob(g_pGameClientShell->GetPlayerCamera(), m_fBobHeight * g_vtHeadBobAdjust.GetFloat());


	// Update the weapon model bobbing...
	CWeaponModel* pWeaponModel = g_pGameClientShell->GetWeaponModel();
	if (pWeaponModel)
	{
        LTFLOAT fSwayHeight = g_vtSwayV.GetFloat() * m_fBobAmp * (float)sin(m_fSwayPhase * 2);
        LTFLOAT fSwayWidth  = g_vtSwayH.GetFloat() * m_fBobAmp * (float)sin(m_fSwayPhase - (MATH_PI/3));

		fSwayHeight *= g_vtWeaponSway.GetFloat();
		fSwayWidth *= g_vtWeaponSway.GetFloat();

		LTBOOL bDoSway = (LTBOOL)GetConsoleInt("WeaponSway",1);
		if(bDoSway)
			pWeaponModel->UpdateBob(fSwayWidth, fSwayHeight);
		else
			pWeaponModel->UpdateBob(0, 0);
	}

	// Play foot step sounds at the appropriate time...
	if (bMoving && bFootstep)
	{
		//Play foot step sound here
		CCharacterFX* pFX = dynamic_cast<CCharacterFX*>(g_pGameClientShell->GetSFXMgr()->GetClientCharacterFX());

		if(pFX) pFX->ForceFootstep();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::AdjustCameraPos()
//
//	PURPOSE:	Adjust the camera's bob position...
//
// ----------------------------------------------------------------------- //

void CHeadBobMgr::AdjustCameraPos(LTVector &vPos)
{
	vPos.y += m_fBobHeight * g_vtHeadBobAdjust.GetFloat();
}

