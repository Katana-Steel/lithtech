// ----------------------------------------------------------------------- //
//
// MODULE  : PickupObjectFX.cpp
//
// PURPOSE : PickupObjectFX - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "iltclient.h"
#include "pickupobjectfx.h"
#include "pickupbutemgr.h"
#include "vartrack.h"

// ----------------------------------------------------------------------- //

static VarTrack		g_vtPickupGlowRate;
static VarTrack		g_vtPickupSpinRate;
static VarTrack		g_vtPickupBounceRate;
static VarTrack		g_vtPickupBounceHeight;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupObjectFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupObjectFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	PUFXCREATESTRUCT puObj;

	puObj.hServerObj	= hServObj;
	puObj.nFXType		= g_pLTClient->ReadFromMessageByte(hRead);

	return Init(&puObj);
}

// ----------------------------------------------------------------------- //

LTBOOL CPickupObjectFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PUFXCREATESTRUCT* pPUFX = (PUFXCREATESTRUCT*)psfxCreateStruct;

	m_nFXType = pPUFX->nFXType;

	// Make sure the FX type is within range of our bute mgr
	if((m_nFXType < 0) || (m_nFXType >= g_pPickupButeMgr->GetNumPickupFXButes()))
		return LTFALSE;

	// Make sure the console variables are setup
	if(!g_vtPickupGlowRate.IsInitted())
		g_vtPickupGlowRate.Init(g_pLTClient, "PickupGlowRate", LTNULL, 1.0f);
	if(!g_vtPickupSpinRate.IsInitted())
		g_vtPickupSpinRate.Init(g_pLTClient, "PickupSpinRate", LTNULL, 0.5f);
	if(!g_vtPickupBounceRate.IsInitted())
		g_vtPickupBounceRate.Init(g_pLTClient, "PickupBounceRate", LTNULL, 2.0f);
	if(!g_vtPickupBounceHeight.IsInitted())
		g_vtPickupBounceHeight.Init(g_pLTClient, "PickupBounceHeight", LTNULL, 15.0f);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupObjectFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupObjectFX::CreateObject(CClientDE *pClientDE)
{
	LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if(!bRet) return bRet;

	// Get the PickupFX we're planning to create
	PickupFXButes puFX = g_pPickupButeMgr->GetPickupFXButes(m_nFXType);

	// Setup a common object create structure
	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = FLAG_VISIBLE;

	// See if we need to create a particle system
	if(puFX.m_szParticle[0] && puFX.m_nNumParticles)
	{
		ocs.m_ObjectType = OT_PARTICLESYSTEM;
		ocs.m_Flags2 = FLAG2_ADDITIVE;
		m_hParticleSystem = g_pLTClient->CreateObject(&ocs);

		if(m_hParticleSystem)
		{
			g_pLTClient->SetupParticleSystem(m_hParticleSystem, puFX.m_szParticle,
											-500, 0, 1000.0f);
		}
	}

	// See if we need to create a light
	if(puFX.m_bLight)
	{
		ocs.m_ObjectType = OT_LIGHT;
		m_hLight = g_pLTClient->CreateObject(&ocs);

		if(m_hLight)
		{
			g_pLTClient->SetLightColor(m_hLight, 1.0f, 1.0f, 1.0f);
			g_pLTClient->SetLightRadius(m_hLight, (LTFLOAT)puFX.m_nLightRadius);
		}
	}

	// Get the current time to base our FX off of
	LTFLOAT fTime = g_pLTClient->GetTime();
	m_fGlowTime = fTime;
	m_fBounceTime = fTime;

	g_pLTClient->GetObjectPos(m_hServerObject, &m_vOrigPos);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupObjectFX::Update
//
//	PURPOSE:	Update the pickupitem
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupObjectFX::Update()
{
	if(IsWaitingForRemove() || !m_hServerObject) return LTFALSE;

	// Get some variables that we'll need in order to update properly
	PickupFXButes puFX = g_pPickupButeMgr->GetPickupFXButes(m_nFXType);
	LTFLOAT fFrameTime = g_pLTClient->GetFrameTime();

	// --------------------------------------------------------------------- //
	// Spin the pickup object if we're supposed to
	if(puFX.m_bSpin)
	{
		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		g_pLTClient->EulerRotateY(&rRot, (MATH_PI * g_vtPickupSpinRate.GetFloat()) * fFrameTime);
		g_pLTClient->SetObjectRotation(m_hServerObject, &rRot);
	}

	// --------------------------------------------------------------------- //
	// Bounce the pickup object if we're supposed to
	if(puFX.m_bBounce)
	{
		LTVector vPosOffset(0.0f, g_vtPickupBounceHeight.GetFloat(), 0.0f);
		LTFLOAT fBounceRate = g_vtPickupBounceRate.GetFloat();

		// Increase our bounce time
		m_fBounceTime += fFrameTime;

		// Make sure our bounce time stays within the bounce rate
		if(m_fBounceTime > fBounceRate)
		{
			m_fBounceTime = m_fBounceTime - ((int)(m_fBounceTime / fBounceRate) * fBounceRate);
		}

		// Calculate the ratio of our bounce time
		LTFLOAT fRatio = m_fBounceTime / fBounceRate;
		fRatio = sinf(fRatio * MATH_PI);

		vPosOffset *= fRatio;
		vPosOffset += m_vOrigPos;
		g_pLTClient->SetObjectPos(m_hServerObject, &vPosOffset);

		// If the light exists... move it with the object
		if(m_hLight)
		{
			g_pLTClient->SetObjectPos(m_hLight, &vPosOffset);
		}

		// If the particle system exists... move it with the object
		if(m_hParticleSystem)
		{
			g_pLTClient->SetObjectPos(m_hParticleSystem, &vPosOffset);
		}
	}

	// --------------------------------------------------------------------- //
	// Bounce the pickup object if we're supposed to
	if(puFX.m_bGlow)
	{
		LTFLOAT fGlowRate = g_vtPickupGlowRate.GetFloat();

		// Increase our glow time
		m_fGlowTime += fFrameTime;

		// Make sure our glow time stays within the glow rate
		if(m_fGlowTime > fGlowRate)
		{
			m_fGlowTime = m_fGlowTime - ((int)(m_fGlowTime / fGlowRate) * fGlowRate);
		}

		// Calculate the ratio of our glow time
		LTFLOAT fRatio = m_fGlowTime / fGlowRate;
		fRatio = sinf(fRatio * MATH_PI);

		// Make our light glow
		if(m_hLight)
		{
			g_pLTClient->SetLightRadius(m_hLight, (LTFLOAT)(puFX.m_nLightRadius * fRatio));
		}

		// Set the glow ratio console variable
		char szBuffer[16];
		sprintf(szBuffer, "GlowRatio %f", fRatio);
		g_pLTClient->RunConsoleString(szBuffer);
	}

	// --------------------------------------------------------------------- //
	// If the particle system is created... shoot out some particles
	if(m_hParticleSystem)
	{
		LTFLOAT fParticleRate = 1.0f / (LTFLOAT)puFX.m_nNumParticles;

		// Increase our particle shoot time
		m_fParticleTime += fFrameTime;

		// Make sure our particle time stays within the rate
		if(m_fParticleTime > fParticleRate)
		{
			m_fParticleTime = m_fParticleTime - ((int)(m_fParticleTime / fParticleRate) * fParticleRate);

			LTVector vDummy(0.0f, 0.0f, 0.0f);
			LTParticle *p = g_pLTClient->AddParticle(m_hParticleSystem, &vDummy, &vDummy, &vDummy, puFX.m_fParticleLifetime);

			p->m_Alpha = 0.75f;
			p->m_Size = GetRandom((LTFLOAT)(puFX.m_fParticleSize / 2), (LTFLOAT)puFX.m_fParticleSize);
			p->m_Velocity = LTVector(GetRandom(-50.0f, 50.0f), GetRandom(0.0f, 200.0f), GetRandom(-50.0f, 50.0f));
			p->m_Color = LTVector(255.0f, 255.0f, 255.0f);
		}
	}

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_PickupObjectFactory
//-------------------------------------------------------------------------------------------------

const SFX_PickupObjectFactory SFX_PickupObjectFactory::registerThis;

CSpecialFX* SFX_PickupObjectFactory::MakeShape() const
{
	return new CPickupObjectFX();
}

