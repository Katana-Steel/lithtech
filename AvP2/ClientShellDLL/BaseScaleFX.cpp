// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScaleFX.cpp
//
// PURPOSE : BaseScale special FX - Implementation
//
// CREATED : 5/27/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseScaleFX.h"
#include "iltclient.h"
#include "GameClientShell.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

static VarTrack	g_vtRotate;
static VarTrack	g_vtRotateVel;
static VarTrack	g_vtRotateLeft;
static VarTrack	g_vtFaceCamera;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	BSCREATESTRUCT* pBaseScale = (BSCREATESTRUCT*)psfxCreateStruct;

	m_rRot				= pBaseScale->rRot;
	m_vPos				= pBaseScale->vPos;
	m_vVel				= pBaseScale->vVel;
	m_vInitialScale		= pBaseScale->vInitialScale;
	m_vFinalScale		= pBaseScale->vFinalScale;
	m_vInitialColor		= pBaseScale->vInitialColor;
	m_vFinalColor		= pBaseScale->vFinalColor;
	m_bUseUserColors	= pBaseScale->bUseUserColors;
	m_dwFlags			= pBaseScale->dwFlags;
	m_fLifeTime			= pBaseScale->fLifeTime;
	m_fCSLifeTime		= m_fLifeTime;
	m_fDelayTime		= pBaseScale->fDelayTime;
	m_fInitialAlpha		= pBaseScale->fInitialAlpha;
	m_fFinalAlpha		= pBaseScale->fFinalAlpha;
	m_Filename			= pBaseScale->Filename;
	for(int i = 0; i < MAX_MODEL_TEXTURES; ++i)
	{
		m_Skins[i] = pBaseScale->Skins[i];
	}
	m_bLoop				= pBaseScale->bLoop;
	m_bAdditive			= pBaseScale->bAdditive;
	m_bMultiply			= pBaseScale->bMultiply;
	m_nType				= pBaseScale->nType;
	m_bRotate			= pBaseScale->bRotate;
	m_bFaceCamera		= pBaseScale->bFaceCamera;
	m_fRotateVel		= GetRandom(pBaseScale->fMinRotateVel, pBaseScale->fMaxRotateVel);
	m_nRotationAxis		= pBaseScale->nRotationAxis;
	m_fFadeOutTime		= pBaseScale->fFadeOutTime;
	m_ePriority			= pBaseScale->ePriority;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::CreateObject
//
//	PURPOSE:	Create object associated with the BaseScale
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || m_Filename.empty()) return LTFALSE;

	// Setup the BaseScale...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, m_Filename.c_str());
	if (!m_Skins[0].empty())
	{
		for (int i = 0; i < MAX_MODEL_TEXTURES; ++i)
		{
			SAFE_STRCPY(createStruct.m_SkinNames[i], m_Skins[i].c_str());
		}
	}

	// Allow create object to be called to re-init object...

	if (m_hObject)
	{
		// See if we are changing object types...

		if (pClientDE->GetObjectType(m_hObject) != m_nType)
		{
			// Shit, need to re-create object...

			pClientDE->DeleteObject(m_hObject);
            m_hObject = LTNULL;
		}
		else  // Cool, can re-use object...
		{
			pClientDE->Common()->SetObjectFilenames(m_hObject, &createStruct);
			pClientDE->SetObjectFlags(m_hObject, m_dwFlags);
			pClientDE->SetObjectPos(m_hObject, &m_vPos);
			pClientDE->SetObjectRotation(m_hObject, &m_rRot);
		}
	}


	// See if we need to create the object...

	if (!m_hObject)
	{
		createStruct.m_ObjectType	= m_nType;
		createStruct.m_Flags		= m_dwFlags;
		createStruct.m_Pos			= m_vPos;
		createStruct.m_Rotation		= m_rRot;

		m_hObject = pClientDE->CreateObject(&createStruct);
        if (!m_hObject) return LTFALSE;
	}


	// Set blend modes if applicable...

    uint32 dwFlags;
    g_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags);

	// Clear flags...
	dwFlags &= ~FLAG2_ADDITIVE;
	dwFlags &= ~FLAG2_MULTIPLY;

    LTBOOL bFog = LTTRUE;
	if (m_bAdditive)
	{
		dwFlags |= FLAG2_ADDITIVE;
        bFog = LTFALSE;
	}
	else if (m_bMultiply)
	{
		dwFlags |= FLAG2_MULTIPLY;
        bFog = LTFALSE;
	}
    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags);


	// Enable/Disable fog as appropriate...

    g_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if (bFog)
	{
		dwFlags &= ~FLAG_FOGDISABLE;
	}
	else
	{
		dwFlags |= FLAG_FOGDISABLE;
	}

    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);

	return Reset();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::Reset
//
//	PURPOSE:	Reset the object
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::Reset()
{
    if (!m_hObject) return LTFALSE;

    LTFLOAT r, g, b, a;
	if (m_bUseUserColors)
	{
		r = m_vInitialColor.x;
		g = m_vInitialColor.y;
		b = m_vInitialColor.z;
	}
	else
	{
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	}

	m_pClientDE->SetObjectScale(m_hObject, &m_vInitialScale);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fInitialAlpha);

	m_fStartTime = m_pClientDE->GetTime() + m_fDelayTime;
	m_fEndTime	 = m_fStartTime + m_fCSLifeTime;

	if (m_vVel.x != 0.0f || m_vVel.y != 0.0 || m_vVel.z != 0.0)
	{
		InitMovingObject(&m_movingObj, &m_vPos, &m_vVel);
		m_movingObj.m_dwPhysicsFlags |= MO_NOGRAVITY;
	}

	if (m_nType == OT_MODEL)
	{
		m_pClientDE->SetModelLooping(m_hObject, m_bLoop);
	}


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::Update
//
//	PURPOSE:	Update the BaseScale
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::Update()
{
    if(!m_hObject || !m_pClientDE || m_bWantRemove) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if(fTime > m_fEndTime)
	{
		if(m_bLoop)
		{
			m_fStartTime = fTime;
			m_fEndTime = m_fStartTime + m_fCSLifeTime;
			m_FRState = FR_EXCELLENT;
		}
		else
		{
			if(fTime > m_fEndTime )
				//time to vamoose!
				return LTFALSE;
		}
	}
	else if (fTime < m_fStartTime)
	{
        uint32 dwFlags = m_pClientDE->GetObjectFlags(m_hObject);
		m_pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
        return LTTRUE;  // not yet...
	}
	else
	{
        uint32 dwFlags = m_pClientDE->GetObjectFlags(m_hObject);
		m_pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);

		// See about adjusting lifetime here
		if(m_ePriority != BS_PRIORITY_HIGH)
		{
			FRState eState = g_pGameClientShell->GetFrameRateState();
			if(eState < m_FRState)
			{
				// Things have gotten worse
				m_FRState = eState;
				SetFrameRateMod();
			}
		}
	}

    LTFLOAT fTimeDelta = fTime - m_fStartTime;

	if (m_fFinalAlpha != m_fInitialAlpha || m_vInitialColor.x != m_vFinalColor.x ||
		m_vInitialColor.y != m_vFinalColor.y || m_vInitialColor.z != m_vFinalColor.z|| 
		m_fFadeOutTime != 0.0f)
	{
		UpdateAlpha(fTimeDelta);
	}

	if (m_vInitialScale.x != m_vFinalScale.x ||
		m_vInitialScale.y != m_vFinalScale.y ||
		m_vInitialScale.z != m_vFinalScale.z )
	{
		UpdateScale(fTimeDelta);
	}

	if (m_vVel.x != 0.0f || m_vVel.y != 0.0 || m_vVel.z != 0.0)
	{
		UpdatePos(fTimeDelta);
	}

	UpdateRot(fTimeDelta);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdateAlpha
//
//	PURPOSE:	Update the BaseScale alpha
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdateAlpha(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	LTFLOAT fAlpha;
	if(fTimeDelta < m_fLifeTime-m_fFadeOutTime)
		fAlpha = m_fInitialAlpha + (fTimeDelta * (m_fFinalAlpha - m_fInitialAlpha) / (m_fLifeTime-m_fFadeOutTime));
	else
	{
		fAlpha = m_fFinalAlpha - (fTimeDelta-(m_fLifeTime-m_fFadeOutTime))/m_fFadeOutTime*m_fFinalAlpha;
	}


    LTVector vColor;
	if (m_bUseUserColors)
	{
		vColor.x = m_vInitialColor.x + (fTimeDelta * (m_vFinalColor.x - m_vInitialColor.x) / m_fLifeTime);
		vColor.y = m_vInitialColor.y + (fTimeDelta * (m_vFinalColor.y - m_vInitialColor.y) / m_fLifeTime);
		vColor.z = m_vInitialColor.z + (fTimeDelta * (m_vFinalColor.z - m_vInitialColor.z) / m_fLifeTime);

		//m_pClientDE->CPrint("Color = (%.2f, %.2f, %.2f), Alpha = %.2f", vColor.x, vColor.y, vColor.z, fAlpha);
	}
	else
	{
        LTFLOAT a;
		m_pClientDE->GetObjectColor(m_hObject, &(vColor.x), &(vColor.y), &(vColor.z), &a);
	}

	m_pClientDE->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, fAlpha);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdateScale
//
//	PURPOSE:	Update the BaseScale alpha
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdateScale(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

    LTVector vScale;
	vScale.Init();

	vScale.x = m_vInitialScale.x + (fTimeDelta * (m_vFinalScale.x - m_vInitialScale.x) / m_fLifeTime);
	vScale.y = m_vInitialScale.y + (fTimeDelta * (m_vFinalScale.y - m_vInitialScale.y) / m_fLifeTime);
	vScale.z = m_vInitialScale.z + (fTimeDelta * (m_vFinalScale.z - m_vInitialScale.z) / m_fLifeTime);

	m_pClientDE->SetObjectScale(m_hObject, &vScale);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdatePos
//
//	PURPOSE:	Update the BaseScale's pos
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdatePos(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	if (m_movingObj.m_dwPhysicsFlags & MO_RESTING) return;

    LTVector vNewPos;
    if (UpdateMovingObject(LTNULL, &m_movingObj, &vNewPos))
	{
		m_movingObj.m_vLastPos = m_movingObj.m_vPos;
		m_movingObj.m_vPos = vNewPos;

		m_pClientDE->SetObjectPos(m_hObject, &vNewPos);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdateRot
//
//	PURPOSE:	Update the BaseScale's rotation
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdateRot(LTFLOAT fTimeDelta)
{
	if (!m_bRotate || !m_hObject) return;

    LTVector vU, vR, vF, vAxis;
    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hObject, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// See if this is a rotatable sprite and we want it to face the
	// camera...
	if (m_bFaceCamera)
	{
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hObject);
		if (dwFlags & FLAG_ROTATEABLESPRITE)
		{
			// Okay, make sure we're facing the camera...

            LTVector vCamPos, vPos;
			HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
			g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, vCamPos, LTTRUE);
            g_pLTClient->GetObjectPos(m_hObject, &vPos);

			vF = vCamPos - vPos;
			vF.Norm();
            g_pLTClient->AlignRotation(&rRot, &vF, &vU);
		}
	}

	if (m_nType == OT_MODEL && m_nRotationAxis == 1)
	{
		VEC_COPY(vAxis,vU);
	}
	else if (m_nType == OT_MODEL && m_nRotationAxis == 2)
	{
		VEC_COPY(vAxis,vR);
	}
	else
		VEC_COPY(vAxis,vF);

    g_pLTClient->RotateAroundAxis(&rRot, &vAxis, m_fRotateVel * g_pLTClient->GetFrameTime());
    g_pLTClient->SetObjectRotation(m_hObject, &rRot);
}

void CBaseScaleFX::AdjustScale(LTFLOAT fScaleMultiplier)
{
	VEC_MULSCALAR(m_vInitialScale, m_vInitialScale, fScaleMultiplier);
	VEC_MULSCALAR(m_vFinalScale, m_vFinalScale, fScaleMultiplier);

	m_pClientDE->SetObjectScale(m_hObject, &m_vInitialScale);

}

void CBaseScaleFX::MakeInvisible(LTBOOL vis)
{
	DDWORD flags = g_pLTClient->GetObjectFlags(m_hObject);
	if (vis)
	{
		g_pLTClient->SetObjectFlags(m_hObject, flags & FLAG_VISIBLE);
	}
	else
	{
		g_pLTClient->SetObjectFlags(m_hObject, flags | FLAG_VISIBLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::SetFrameRateMod
//
//	PURPOSE:	Modify the lifetime based on frame rate
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::SetFrameRateMod()
{
	LTFLOAT fMod=1.0f;

	switch(m_FRState)
	{
	case FR_UNPLAYABLE:	fMod = m_ePriority==BS_PRIORITY_LOW?0.1f:0.2f; break;
	case FR_POOR:		fMod = m_ePriority==BS_PRIORITY_LOW?0.2f:0.4f; break;
	case FR_AVERAGE:	fMod = m_ePriority==BS_PRIORITY_LOW?0.4f:0.6f; break;
	case FR_GOOD:		fMod = m_ePriority==BS_PRIORITY_LOW?0.7f:0.9f; break;
	case FR_EXCELLENT:	fMod = 1.0f; break;
	}

    LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fTimeMod = (m_fEndTime-fTime)*(1.0f-fMod);

	// Adjust both the end time and life time
	m_fEndTime	-= fTimeMod;
	m_fLifeTime	-= fTimeMod;
}



//-------------------------------------------------------------------------------------------------
// SFX_ScaleFactory
//-------------------------------------------------------------------------------------------------

const SFX_ScaleFactory SFX_ScaleFactory::registerThis;

CSpecialFX* SFX_ScaleFactory::MakeShape() const
{
	return new CBaseScaleFX();
}


