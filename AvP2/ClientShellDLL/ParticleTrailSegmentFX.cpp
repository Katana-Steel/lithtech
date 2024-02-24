// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailSegmentFX.cpp
//
// PURPOSE : ParticleTrail segment special FX - Implementation
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleTrailSegmentFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "GameSettings.h"
#include "GameClientShell.h"
#include "custom_draw_lt.h"
#include "FXButeMgr.h"

extern LTVector g_vWorldWindVel;
extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailSegmentFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	PTSCREATESTRUCT* pPTS = (PTSCREATESTRUCT*)psfxCreateStruct;

	m_pPTFX = g_pFXButeMgr->GetParticleTrailFX(pPTS->nType);
	m_vLastPos = pPTS->vLastPos;
	m_rLastRot = pPTS->rLastRot;
	m_vCurPos = pPTS->vCurPos;
	m_rCurRot = pPTS->rCurRot;

	// We'll control the particle system's position...
	m_basecs.bClientControlsPos = LTTRUE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailSegmentFX::CreateObject(ILTClient *pLTClient)
{
	if (!pLTClient || !m_pPTFX) return LTFALSE;

	// Setup the variables from the base particle system
	m_pTextureName = m_pPTFX->szTexture;
	m_fGravity = m_pPTFX->fGravity;
	m_fRadius = m_pPTFX->fRadius;

	m_vPos = m_vLastPos;
	m_rRot.Init();

	m_vColor1 = m_pPTFX->vMinColor;
	m_vColor2 = m_pPTFX->vMaxColor;

	m_basecs.bAdditive = m_pPTFX->bAdditive;
	m_basecs.bMultiply = m_pPTFX->bMultiply;
	m_basecs.bAdjustParticleScale = LTTRUE;
	m_basecs.fStartParticleScale  = m_pPTFX->fStartScale;
	m_basecs.fEndParticleScale	  = m_pPTFX->fEndScale;
	m_basecs.bAdjustParticleAlpha = LTTRUE;
	m_basecs.fStartParticleAlpha  = m_pPTFX->fStartAlpha;
	m_basecs.fEndParticleAlpha	  = m_pPTFX->fEndAlpha;
	m_basecs.bIgnoreParentRemoval = LTTRUE;

	return CBaseParticleSystemFX::CreateObject(pLTClient);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailSegmentFX::Update()
{
	if(!m_hObject || !g_pLTClient || !m_pPTFX) 
		return LTFALSE;

	CBaseParticleSystemFX::Update();

//	if(!CBaseParticleSystemFX::Update()) 
//		return LTFALSE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if(!pSettings) return LTFALSE;

	uint8 nDetailLevel = pSettings->SpecialFXSetting();


	// Get the current time to update with
	LTFLOAT fTime = g_pLTClient->GetTime();


	// Handle the first update
	if(m_bFirstUpdate)
	{
		// Set the starting values
		m_bFirstUpdate = LTFALSE;
		m_fStartTime   = fTime;

		// Figure out how many steps we need to emit particles from
		LTVector vPosDelta = m_vCurPos - m_vLastPos;
		LTVector vDir = vPosDelta;
		vDir.Norm(m_pPTFX->fEmitDistance);

		LTFLOAT fPosDeltaMag = vPosDelta.Mag();

		int nEmitSteps = (int)(fPosDeltaMag / m_pPTFX->fEmitDistance);
		if(nEmitSteps <= 0) return LTFALSE;


		// Go through each step and create particles for it
		LTVector vStepPos(0.0f, 0.0f, 0.0f);
		LTVector vDrift;
		LTVector vColor;

		for(int i = 0; i < nEmitSteps; i++)
		{
			for(int j = 0; j < m_pPTFX->nEmitAmount; j++)
			{
				// Get a random drift amount
				vDrift.x = GetRandom(m_pPTFX->vMinDrift.x, m_pPTFX->vMaxDrift.x);
				vDrift.y = GetRandom(m_pPTFX->vMinDrift.y, m_pPTFX->vMaxDrift.y);
				vDrift.z = GetRandom(m_pPTFX->vMinDrift.z, m_pPTFX->vMaxDrift.z);

				// Get a random color
				GetRandomColorInRange(vColor);

				// Add the particle
				g_pLTClient->AddParticle(m_hObject, &vStepPos, &vDrift, &vColor, m_pPTFX->fLifetime);
			}

			// Increment the base step position
			vStepPos += vDir;
		}
	}


	// When the time is greater than our starttime plus lifetime, we're gone
	if(fTime > m_fStartTime + m_pPTFX->fLifetime)
		return LTFALSE;


	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_ParticleTrailSegFactory
//-------------------------------------------------------------------------------------------------

const SFX_ParticleTrailSegFactory SFX_ParticleTrailSegFactory::registerThis;

CSpecialFX* SFX_ParticleTrailSegFactory::MakeShape() const
{
	return new CParticleTrailSegmentFX();
}

