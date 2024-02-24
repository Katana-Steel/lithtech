// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFX.cpp
//
// PURPOSE : Debris - Implementation
//
// CREATED : 5/31/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebrisFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "FXButeMgr.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack g_cvarMarkChance;

VarTrack g_cvarDebrisSizzleChance;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

DBOOL CDebrisFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	DEBRISCREATESTRUCT info;

	info.hServerObj = hServObj;
    g_pLTClient->ReadFromMessageRotation(hRead, &info.rRot);//, &cd.rRot);
    g_pLTClient->ReadFromMessageVector(hRead, &info.vPos);//, &cd.vPos);
    info.nDebrisId = g_pLTClient->ReadFromMessageByte(hRead);//, cd.nDebrisId);
    g_pLTClient->ReadFromMessageVector(hRead, &info.vVelOffset);//, &cd.vPos);

	return Init(&info);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	DEBRISCREATESTRUCT* pDCS = (DEBRISCREATESTRUCT*)psfxCreateStruct;

	m_ds = *(pDCS);

	// If a valid debris id was specified, use it to determine the debris
	// values...

	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_ds.nDebrisId);
	if (pDebris)
	{
		m_ds.bRotate		= pDebris->bRotate;
		m_ds.fMinLifeTime	= pDebris->fMinLifetime;
		m_ds.fMaxLifeTime	= pDebris->fMaxLifetime;
		m_ds.fFadeTime		= pDebris->fFadetime;
		m_ds.fMinScale		= pDebris->fMinScale;
		m_ds.fMaxScale		= pDebris->fMaxScale;
		m_ds.nNumDebris		= pDebris->nNumber;
		m_ds.vMaxVel		= pDebris->vMaxVel;
		m_ds.vMinVel		= pDebris->vMinVel;
		m_ds.vMinDOffset	= pDebris->vMinDOffset;
		m_ds.vMaxDOffset	= pDebris->vMaxDOffset;
		m_ds.nMaxBounce		= pDebris->nMaxBounce;
		m_ds.nMinBounce		= pDebris->nMinBounce;
		m_ds.fGravityScale	= pDebris->fGravityScale;
		m_ds.fAlpha			= pDebris->fAlpha;
	}

	// Update our velocities
	m_ds.vMaxVel += m_ds.vVelOffset;
	m_ds.vMinVel += m_ds.vVelOffset;

	m_ds.nNumDebris = (m_ds.nNumDebris > MAX_DEBRIS ? MAX_DEBRIS : m_ds.nNumDebris);

	if (GetConsoleInt("DebrisFXLevel", 2) == 0)
	{
		m_ds.nNumDebris = uint8(float(m_ds.nNumDebris) * 0.333f);
		m_ds.fMinLifeTime *= 0.333f;
		m_ds.fMaxLifeTime *= 0.333f;
	}
	else if (GetConsoleInt("DebrisFXLevel", 2) == 1)
	{
		m_ds.nNumDebris = uint8(float(m_ds.nNumDebris) * 0.666f);
		m_ds.fMinLifeTime *= 0.666f;
		m_ds.fMaxLifeTime *= 0.666f;
	}

	if (!g_cvarMarkChance.IsInitted())
	{
		g_cvarMarkChance.Init(g_pLTClient, "MarkChance", NULL, 0.5f);
    }

	if (!g_cvarDebrisSizzleChance.IsInitted())
	{
		g_cvarDebrisSizzleChance.Init(g_pLTClient, "DebrisSizzleChance", NULL, 1.0f);
    }
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	// Initialize the debris velocity ranges based on our rotation...

    LTVector vVelMin, vVelMax, vTemp, vU, vR, vF;
	vVelMin.Init(1.0f, 1.0f, 1.0f);
	vVelMax.Init(1.0f, 1.0f, 1.0f);

	m_pClientDE->GetRotationVectors(&m_ds.rRot, &vU, &vR, &vF);

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		vR.Init(1.0f, 0.0f, 0.0f);
		vU.Init(0.0f, 0.0f, -1.0f * vF.y);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		vR.Init(0.0f, 0.0f, -1.0f * vF.x);
		vU.Init(0.0f, 1.0f, 0.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		vR.Init(vF.z, 0.0f, 0.0f);
		vU.Init(0.0f, 1.0f, 0.0f);
	}

	vVelMin = vF * m_ds.vMinVel.y;
	vVelMax = vF * m_ds.vMaxVel.y;
	vVelMin += vR * m_ds.vMinVel.x;
	vVelMax += vR * m_ds.vMaxVel.x;
	vVelMin += vU * m_ds.vMinVel.z;
	vVelMax += vU * m_ds.vMaxVel.z;


	// Create the debris models

	g_pDebrisMgr->CreateDebris(m_ds.nDebrisId, m_hDebris, m_ds.nNumDebris, m_ds.vPos, m_nDebrisFlags);


	// Initialize our emitters...

    LTFLOAT fVal  = MATH_CIRCLE/2.0f;
    LTFLOAT fVal2 = MATH_CIRCLE;
    LTVector vVel;

	// Get the initial framerate state and use it
	// to modify the life of the debris
	m_FRState = g_pGameClientShell->GetFrameRateState();

	for(int i = 0; i < m_ds.nNumDebris; i++)
	{
		if (m_ds.bRotate)
		{
			m_fPitchVel[i] = GetRandom(-fVal, fVal);
			m_fYawVel[i]   = GetRandom(-fVal2, fVal2);
			m_fRollVel[i]  = GetRandom(-fVal2, fVal2);
		}

		m_fDebrisLife[i] = GetRandom(m_ds.fMinLifeTime, m_ds.fMaxLifeTime);

        m_ActiveEmitters[i] = LTTRUE;
		m_BounceCount[i]	= GetRandom(m_ds.nMinBounce, m_ds.nMaxBounce);

		vVel.Init(GetRandom(vVelMin.x, vVelMax.x),
				  GetRandom(vVelMin.y, vVelMax.y),
				  GetRandom(vVelMin.z, vVelMax.z));

		// Add a random offset to each debris item...
        LTVector vPos;

		if (m_ds.bDirOffsetOnly)
		{
			// Only offset in the direction the debris is traveling...
			vPos = vVel;
			vPos.Norm();

			// Only use y (forward) offset...
			vPos *= GetRandom(m_ds.vMinDOffset.y, m_ds.vMaxDOffset.y);
		}
		else
		{
			vPos.Init(GetRandom(m_ds.vMinDOffset.x, m_ds.vMaxDOffset.x),
					  GetRandom(m_ds.vMinDOffset.y, m_ds.vMaxDOffset.y),
					  GetRandom(m_ds.vMinDOffset.z, m_ds.vMaxDOffset.z));
		}

		vPos += m_ds.vPos;
		InitMovingObject(&(m_Emitters[i]), &vPos, &vVel);

		m_Emitters[i].m_fGravityScale = m_ds.fGravityScale;

		if(m_hDebris[i])
			g_pLTClient->SetObjectPos(m_hDebris[i], &vPos);

		// This is a virtual functions and needs to be here for the poly debris
		CreateDebris(i, vPos);
	}

	// Now update the life based on frame rate
	SetFrameRateMod();

	// Play the explode sound...

	if (m_ds.bPlayExplodeSound)
	{
		char* pSound = g_pDebrisMgr->GetExplodeSound(m_ds.nDebrisId);
		if(pSound && pSound[0])
		{
			g_pClientSoundMgr->PlaySoundFromPos(m_ds.vPos, pSound, 1000.0f, SOUNDPRIORITY_MISC_LOW);
		}
	}


	// Create the impact FX

	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_ds.nDebrisId);

	if(pDebris)
	{
		if(pDebris->szPShowerFX[0])
		{
			CPShowerFX *pFX = g_pFXButeMgr->GetPShowerFX(pDebris->szPShowerFX);

			if(pFX)
			{
				g_pFXButeMgr->CreatePShowerFX(pFX, m_ds.vPos, vU, vF);
			}
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::Update
//
//	PURPOSE:	Update the debris
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::Update()
{
    if (!m_pClientDE) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();
	int i = 0;

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate = LTFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;
	}

	// Check to see if we need to adjust the life time 
	// based on frame rate
	FRState eState = g_pGameClientShell->GetFrameRateState();
	if(eState < m_FRState)
	{
		// Things have gotten worse
		m_FRState = eState;
		SetFrameRateMod();
	}


	// Check to see if we should start fading the debris...

	for (i = 0; i < m_ds.nNumDebris; i++)
	{
        LTFLOAT fEndTime = m_fStartTime + m_fDebrisLife[i];
		LTFLOAT fAlpha = m_ds.fFadeTime > 0.0f ? (fEndTime - fTime) / m_ds.fFadeTime : 0.0f;
		fAlpha = (fAlpha > 1.0f) ? m_ds.fAlpha : (fAlpha * m_ds.fAlpha);

		SetDebrisAlpha(i, fAlpha);

		if (fTime > fEndTime)
		{
				RemoveDebris(i);
		}
	}

	// See if all the debris have been removed or not...

	for (i = 0; i < m_ds.nNumDebris; i++)
	{
		if (IsValidDebris(i)) break;
	}

	// All debris have been removed so remove us...

	if (i == m_ds.nNumDebris)
	{
        return LTFALSE;
	}


	// Loop over our list of emitters, updating the position of each

	for (i = 0; i < m_ds.nNumDebris; i++)
	{
		if (m_ActiveEmitters[i])
		{
            LTBOOL bRemove = LTFALSE;
            LTBOOL bBounced = LTFALSE;
			if (bBounced = UpdateEmitter(&m_Emitters[i], bRemove, i))
			{
				if (!(m_Emitters[i].m_dwPhysicsFlags & MO_LIQUID) && (IsValidDebris(i)))
				{
					if (m_ds.bPlayBounceSound && GetRandom(1, 4) == 1)
					{
						char* pSound = g_pDebrisMgr->GetBounceSound(m_ds.nDebrisId);

						// Play appropriate sound...

						if(pSound && pSound[0])
						{
							g_pClientSoundMgr->PlaySoundFromPos(m_Emitters[i].m_vPos,
								pSound, 500.0f, SOUNDPRIORITY_MISC_LOW);
						}
					}
				}

				m_BounceCount[i]--;

				if (m_BounceCount[i] <= 0)
				{
					m_Emitters[i].m_dwPhysicsFlags |= MO_RESTING;
				}
			}

			// Remove the debris if necessary...

			if (bRemove)
			{
				RemoveDebris(i);
                return LTTRUE;
			}

			if (m_Emitters[i].m_dwPhysicsFlags & MO_RESTING)
			{
                m_ActiveEmitters[i] = LTFALSE;

				if (m_ds.bRotate && IsValidDebris(i))
				{
					RotateDebrisToRest(i);
				}
			}
			else if (IsValidDebris(i))
			{
				SetDebrisPos(i, m_Emitters[i].m_vPos);

				if (m_ds.bRotate)
				{
					if (bBounced)
					{
						// Adjust due to the bounce...

                        LTFLOAT fVal    = MATH_CIRCLE/2.0f;
                        LTFLOAT fVal2   = MATH_CIRCLE;
						m_fPitchVel[i] = GetRandom(-fVal, fVal);
						m_fYawVel[i]   = GetRandom(-fVal2, fVal2);
						m_fRollVel[i]  = GetRandom(-fVal2, fVal2);
					}

					if (m_fPitchVel[i] != 0 || m_fYawVel[i] != 0 || m_fRollVel[i] != 0)
					{
                        LTFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

						m_fPitch[i] += m_fPitchVel[i] * fDeltaTime;
						m_fYaw[i]   += m_fYawVel[i] * fDeltaTime;
						m_fRoll[i]  += m_fRollVel[i] * fDeltaTime;

                        LTRotation rRot;
						m_pClientDE->SetupEuler(&rRot, m_fPitch[i], m_fYaw[i], m_fRoll[i]);

						SetDebrisRot(i, rRot);
					}
				}
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::UpdateEmitter
//
//	PURPOSE:	Update Emitter position
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::UpdateEmitter(MovingObject* pObject, LTBOOL & bRemove, int i)
{
    if (!m_pClientDE || !pObject || pObject->m_dwPhysicsFlags & MO_RESTING) return LTFALSE;
	
    bRemove = LTFALSE;
	
    LTBOOL bRet = LTFALSE;
    LTVector vNewPos;
	ClientIntersectInfo info;
	
	if (UpdateMovingObject(LTNULL, pObject, &vNewPos))
	{
		if(m_BounceCount[i] > 32)
			bRet = LTFALSE;
		else
	        bRet = BounceMovingObject(LTNULL, pObject, &vNewPos, &info, INTERSECT_HPOLY);
		
		// If we hit the sky/invisible surface we're done...
		
		SurfaceType eType = GetSurfaceType(info);
		if (eType == ST_SKY || eType == ST_INVISIBLE)
		{
            bRemove = LTTRUE;
            return LTFALSE;
		}
		
		pObject->m_vLastPos = pObject->m_vPos;
		pObject->m_vPos = vNewPos;
		
        if (m_pClientDE->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
			pObject->m_dwPhysicsFlags |= MO_RESTING;
			pObject->m_vPos = pObject->m_vLastPos;
		}
	}
	
	// See about making a bloody mark!
	if(bRet)
	{
		if(g_pGameClientShell->IsFirstPerson())
		{
			// Lets see what kind of debris we are...
			if( (m_nDebrisFlags[i] & DEBRISMGR_FLAG_PTRAIL_FILTER) && 
				(g_cvarMarkChance.GetFloat() > GetRandom(0.0f, 1.0f)) )
			{
				// See if the effect is going to be infront of the player
				HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());
				
				LTVector vDir;
				g_pLTClient->GetObjectPos(hCamera, &vDir);
				vDir = info.m_Point-vDir;
				
				LTRotation rRot;
				LTVector vNull, vF;
				g_pInterface->GetObjectRotation(hCamera, &rRot);
				g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);
				
				// Test of truth!
				if(vF.Dot(vDir) > 0.0f)
				{
					// Create a blood splat...
					BSCREATESTRUCT sc;
					
					// Randomly rotate the blood splat
					g_pLTClient->AlignRotation(&(sc.rRot), &(info.m_Plane.m_Normal), LTNULL);
					g_pLTClient->RotateAroundAxis(&(sc.rRot), &(info.m_Plane.m_Normal), GetRandom(0.0f, MATH_CIRCLE));
					
					sc.vPos				= info.m_Point + info.m_Plane.m_Normal * GetRandom(1.0f, 1.9f);  // Off the wall a bit
					sc.vVel				= LTVector(0.0f, 0.0f, 0.0f);
					sc.vInitialScale	= LTVector(0.2f, 0.2f, 1.0f);
					sc.vFinalScale		= sc.vInitialScale;
					sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
					sc.fLifeTime		= 15.0f;
					sc.fInitialAlpha	= 1.0f;
					sc.fFinalAlpha		= 0.0f;
					sc.nType			= OT_SPRITE;
					sc.ePriority		= BS_PRIORITY_LOW;
					
					char* pBloodFiles[] =
					{
						"Sprites\\MarBld1.spr",
							"Sprites\\MarBld2.spr",
							"Sprites\\MarBld3.spr",
							"Sprites\\PrdBld1.spr",
							"Sprites\\PrdBld2.spr",
							"Sprites\\PrdBld3.spr",
							"Sprites\\AlnBld1.spr",
							"Sprites\\AlnBld2.spr",
							"Sprites\\AlnBld3.spr",
							"Sprites\\SynBld1.spr",
							"Sprites\\SynBld2.spr",
							"Sprites\\SynBld3.spr",
					};
					
					
					if(m_nDebrisFlags[i] & DEBRISMGR_FLAG_HUMAN_PTRAIL)
					{
						sc.Filename = pBloodFiles[GetRandom(0, 2)];
					}
					else if(m_nDebrisFlags[i] & DEBRISMGR_FLAG_PRED_PTRAIL)
					{
						sc.Filename = pBloodFiles[GetRandom(3, 5)];
					}
					else if(m_nDebrisFlags[i] & DEBRISMGR_FLAG_ALIEN_TRAIL)
					{
						sc.Filename = pBloodFiles[GetRandom(6, 8)];
					}
					else if(m_nDebrisFlags[i] & DEBRISMGR_FLAG_SYNTH_TRAIL)
					{
						sc.Filename = pBloodFiles[GetRandom(9, 11)];
					}
					
					CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
					if(psfxMgr)
					{
						CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
						if(pFX) 
							pFX->Update();

						// Lastly, see if we should sizzle...
						if(m_nDebrisFlags[i] & DEBRISMGR_FLAG_ALIEN_TRAIL)
						{
							if(GetRandom(0.0f, 1.0f) < g_cvarDebrisSizzleChance.GetFloat())
							{
								// First see if we have a valid impact FX or not
								IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX("Alien_Blood_Splat");

								if(pImpactFX)
								{
									IFXCS ifxcs;

									// Fill in a create structure
									ifxcs.vPos = sc.vPos;
									ifxcs.rSurfRot = sc.rRot;
									ifxcs.vDir = info.m_Plane.m_Normal;
									ifxcs.vSurfNormal = info.m_Plane.m_Normal;
									ifxcs.bPlaySound = LTTRUE;

									// make another FX
									g_pFXButeMgr->CreateImpactFX(pImpactFX, ifxcs);
								}
							}
						}
					}
				}
			}
		}
	}
	
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::OkToRemoveDebris
//
//	PURPOSE:	See if this particular model can be removed.
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::OkToRemoveDebris(int i)
{
    if (i < 0 || i >= m_ds.nNumDebris) return LTTRUE;

    if (!m_pClientDE || !g_pGameClientShell || !IsValidDebris(i) || m_ds.bForceRemove) return LTTRUE;

	// The only constraint is that the client isn't currently looking
	// at the model...

    LTVector vPos, vCamPos;
    if (!GetDebrisPos(i, vPos)) return LTTRUE;

	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, vCamPos);


	// Determine if the client can see us...

    LTVector vDir;
	VEC_SUB(vDir, vPos, vCamPos);

    LTRotation rRot;
    LTVector vU, vR, vF;
	g_pGameClientShell->GetCameraMgr()->GetCameraRotation(hCamera, rRot);
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	vDir.Norm();
	vF.Norm();

    if (VEC_DOT(vDir, vF) <= 0.0f) return LTTRUE;


	// Client is looking our way, don't remove it yet...

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::RemoveDebris
//
//	PURPOSE:	Remove the specified debris object
//
// ----------------------------------------------------------------------- //

void CDebrisFX::RemoveDebris(int i)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	if (m_hDebris[i])
	{
		g_pGameClientShell->GetSFXMgr()->RemoveSpecialFX(m_hDebris[i]);

		m_pClientDE->DeleteObject(m_hDebris[i]);
        m_hDebris[i] = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::IsValidDebris
//
//	PURPOSE:	Is the debris valid
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::IsValidDebris(int i)
{
    if (i < 0 || i >= m_ds.nNumDebris) return LTFALSE;

	return !!(m_hDebris[i]);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::RotateDebrisToRest
//
//	PURPOSE:	Rotate the debris to the rest position
//
// ----------------------------------------------------------------------- //

void CDebrisFX::RotateDebrisToRest(int i)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

    LTRotation rRot;
	m_pClientDE->SetupEuler(&rRot, 0.0f, m_fYaw[i], 0.0f);
	m_pClientDE->SetObjectRotation(m_hDebris[i], &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::SetDebrisPos
//
//	PURPOSE:	Set the debris position
//
// ----------------------------------------------------------------------- //

void CDebrisFX::SetDebrisPos(int i, LTVector vPos)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	m_pClientDE->SetObjectPos(m_hDebris[i], &vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::GetDebrisPos
//
//	PURPOSE:	Get the debris position
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::GetDebrisPos(int i, LTVector & vPos)
{
    if (i < 0 || i >= m_ds.nNumDebris) return LTFALSE;

	m_pClientDE->GetObjectPos(m_hDebris[i], &vPos);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::SetDebrisRot
//
//	PURPOSE:	Set the debris rotation
//
// ----------------------------------------------------------------------- //

void CDebrisFX::SetDebrisRot(int i, LTRotation rRot)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	m_pClientDE->SetObjectRotation(m_hDebris[i], &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::SetDebrisAlpha
//
//	PURPOSE:	Set the debris rotation
//
// ----------------------------------------------------------------------- //

void CDebrisFX::SetDebrisAlpha(int i, LTFLOAT fAlpha)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hDebris[i], &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hDebris[i], r, g, b, fAlpha);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::SetFrameRateMod
//
//	PURPOSE:	Modify the lifetime based on frame rate
//
// ----------------------------------------------------------------------- //

void CDebrisFX::SetFrameRateMod()
{
	LTFLOAT fMod=1.0f;

	switch(m_FRState)
	{
	case FR_UNPLAYABLE:	fMod = 0.1f; break;
	case FR_POOR:		fMod = 0.2f; break;
	case FR_AVERAGE:	fMod = 0.4f; break;
	case FR_GOOD:		fMod = 0.7f; break;
	case FR_EXCELLENT:	fMod = 1.0f; break;
	}

	for(int i=0; i<m_ds.nNumDebris; ++i)
	{
		m_fDebrisLife[i] *= fMod;
	}
}


//-------------------------------------------------------------------------------------------------
// SFX_DebrisFactory
//-------------------------------------------------------------------------------------------------

const SFX_DebrisFactory SFX_DebrisFactory::registerThis;

CSpecialFX* SFX_DebrisFactory::MakeShape() const
{
	return new CDebrisFX();
}


