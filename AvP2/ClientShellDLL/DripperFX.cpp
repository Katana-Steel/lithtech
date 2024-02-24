// ----------------------------------------------------------------------- //
//
// MODULE  : DripperFX.cpp
//
// PURPOSE : Dripper special FX - Implementation
//
// CREATED : 1/6/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DripperFX.h"
#include "iltclient.h"
#include "SFXMgr.h"
#include "VarTrack.h"
#include <math.h>


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::Init
//
//	PURPOSE:	Init the dripper fx
//
// ----------------------------------------------------------------------- //

LTBOOL CDripperFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
	if (!hMessage) return LTFALSE;

	// Read in the init info from the message...

	DRIPFXCREATESTRUCT dcs;

	dcs.hServerObj = hServObj;
	dcs.nDripEffect = g_pLTClient->ReadFromMessageByte(hMessage);
	g_pLTClient->ReadFromMessageVector(hMessage, &dcs.vDims);

	return Init(&dcs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::Init
//
//	PURPOSE:	Init the dripper fx
//
// ----------------------------------------------------------------------- //

LTBOOL CDripperFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	DRIPFXCREATESTRUCT* pDFX = (DRIPFXCREATESTRUCT*)psfxCreateStruct;

	m_pDripperFX = g_pFXButeMgr->GetDripperObjFX(pDFX->nDripEffect);
	_ASSERT( m_pDripperFX );
	if( m_pDripperFX )
	{
		m_pImpactFX = g_pFXButeMgr->GetImpactFX(m_pDripperFX->nImpactFX);
	}

	m_vServObjDims = pDFX->vDims;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::CreateObject
//
//	PURPOSE:	Create object associated the object
//
// ----------------------------------------------------------------------- //

LTBOOL CDripperFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	// Validate our init info...

    if (!m_hServerObject || !m_pDripperFX || !m_pDripperFX->szFile[0])
		return LTFALSE;


	// Create the particle system

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_PARTICLESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN | FLAG_FOGDISABLE;
	g_pLTClient->GetObjectPos(m_hServerObject, &createStruct.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &createStruct.m_Rotation);

	m_hObject = m_pClientDE->CreateObject(&createStruct);
    if(!m_hObject) return LTFALSE;

	g_pLTClient->SetupParticleSystem(m_hObject, m_pDripperFX->szFile, 0, PS_NEVERDIE, (m_vServObjDims.y * 1.2f));


	// Set the additive flag

	if(m_pDripperFX->bAdditive)
		g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_ADDITIVE);
	else if(m_pDripperFX->bMultiply)
		g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_MULTIPLY);


	// Now initialize some other data

	m_fLastDripTime = 0.0f;
	m_fDripDelay = 0.0f;

	m_nNumDrips = 0;


	// Alright... we're good to start creating drips

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CDripperFX::OnServerMessage(HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::OnServerMessage(hMessage)) return LTFALSE;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::Update
//
//	PURPOSE:	Update the dripper
//
// ----------------------------------------------------------------------- //

LTBOOL CDripperFX::Update()
{
	// See if we want to just get rid of this object yet
	if(m_bWantRemove || !m_hServerObject || !m_pDripperFX) return LTFALSE;


	// Get the current time to work with
    LTFLOAT fTime = g_pLTClient->GetTime();


	// See if we're allowed to create any more drips at all...
    uint32 dwUserFlags;
    g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

	if(dwUserFlags & USRFLG_VISIBLE)
	{
		// Ok... now check to see if it's the right time to add drips
		if((fTime - m_fLastDripTime) >= m_fDripDelay)
		{
			// Now pick a random amount of drips to create
			int nNumDrips = GetRandom(m_pDripperFX->nMinDripAmount, m_pDripperFX->nMaxDripAmount);

			// Create 'em!
			for(int i = 0; i < nNumDrips; i++)
			{
				CreateDrip();
			}

			// Set the current drip time and a new delay
			m_fLastDripTime = fTime;
			m_fDripDelay = GetRandom(m_pDripperFX->fMinDripFrequency, m_pDripperFX->fMaxDripFrequency);
		}
	}


	// Update all the current drips
	UpdateDrips();


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::CreateDrip
//
//	PURPOSE:	Create a new drip
//
// ----------------------------------------------------------------------- //

void CDripperFX::CreateDrip()
{
	// Find the first open drip spot
	int nDripSpot = -1;

	for(int i = 0; i < MAX_DRIPS; i++)
	{
		if(m_pDrips[i].nState == DRIP_STATE_NONE)
		{
			nDripSpot = i;
			break;
		}
	}

	// This means all the spots are full... so just return
	if(nDripSpot == -1)
		return;


	// We're ok by this point... so figure out how many particles to create
	m_pDrips[nDripSpot].nParticles = (int)(m_pDripperFX->fDripTrailLength * m_pDripperFX->fDripTrailDensity) + 1;

	if(m_pDrips[nDripSpot].nParticles > MAX_DRIP_PARTICLES)
		m_pDrips[nDripSpot].nParticles = MAX_DRIP_PARTICLES;

	// Now add them to the system
	LTFLOAT fXOffset, fZOffset;

	if(m_pDripperFX->bCylinderShape)
	{
		fXOffset = GetRandom(-m_vServObjDims.x, m_vServObjDims.x);
		LTFLOAT fRange = (LTFLOAT)sqrt((m_vServObjDims.z * m_vServObjDims.z) * (1 - ((fXOffset * fXOffset) / (m_vServObjDims.x * m_vServObjDims.x))));
		fZOffset = GetRandom(-fRange, fRange);
	}
	else
	{
		fXOffset = GetRandom(-m_vServObjDims.x, m_vServObjDims.x);
		fZOffset = GetRandom(-m_vServObjDims.z, m_vServObjDims.z);
	}

	LTVector vPos(fXOffset, m_vServObjDims.y, fZOffset);
	LTVector vVel(0.0f, 0.0f, 0.0f);

	LTFLOAT fTemp;
	m_pDrips[nDripSpot].vColor = m_pDripperFX->vMinColor + ((m_pDripperFX->vMaxColor - m_pDripperFX->vMinColor) * GetRandom(0.0f, 1.0f));
	m_pDrips[nDripSpot].fSize = GetRandom(m_pDripperFX->fMinSize, m_pDripperFX->fMaxSize);

	// Set the state of the drip
	m_pDrips[nDripSpot].nState = DRIP_STATE_DRIPPING;
	m_pDrips[nDripSpot].fStartTime = g_pLTClient->GetTime();
	m_pDrips[nDripSpot].fDripTime = GetRandom(m_pDripperFX->fMinDripTime, m_pDripperFX->fMaxDripTime);


	// Create the particles
	for(i = 0; i < m_pDrips[nDripSpot].nParticles; i++)
	{
		// We don't specify the liftetime, since we are creating a PS_NEVERDIE particle system.
		// This means particles won't be removed without our knowledge.
		m_pDrips[nDripSpot].pParticles[i] = g_pLTClient->AddParticle(m_hObject, &vPos, &vVel, &m_pDrips[nDripSpot].vColor, 0.0f );
		m_pDrips[nDripSpot].pParticles[i]->m_Alpha = m_pDripperFX->fAlpha;

		// Calculate the size of the particles in an hourglass shape
		fTemp = (float)i / (float)(m_pDrips[nDripSpot].nParticles - 1);
		m_pDrips[nDripSpot].pParticles[i]->m_Size = (1.0f - (sinf(fTemp * MATH_PI) / 2.0f)) * m_pDrips[nDripSpot].fSize;
	}


	// Make sure to increment our drip number
	m_nNumDrips++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::RemoveDrip
//
//	PURPOSE:	Take a drip out of the update list
//
// ----------------------------------------------------------------------- //

void CDripperFX::RemoveDrip(int nDrip)
{
	// Check the range
	if((nDrip < 0) || (nDrip >= MAX_DRIPS))
		return;


	// Go through the particles and remove them
	for(int i = 0; i < MAX_DRIP_PARTICLES; i++)
	{
		if(m_pDrips[nDrip].pParticles[i])
		{
			g_pLTClient->RemoveParticle(m_hObject, m_pDrips[nDrip].pParticles[i]);
			m_pDrips[nDrip].pParticles[i] = LTNULL;
		}
	}

	// Set the state of the drip
	m_pDrips[nDrip].nParticles = 0;
	m_pDrips[nDrip].nState = DRIP_STATE_NONE;


	// Make sure to decrement our drip number
	m_nNumDrips--;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::UpdateDrips
//
//	PURPOSE:	Move the drips around
//
// ----------------------------------------------------------------------- //

void CDripperFX::UpdateDrips()
{
	// If there's no drips to update... just leave!
	if(m_nNumDrips <= 0) return;


	// Go through each drip and update them
	for(int i = 0; i < MAX_DRIPS; i++)
	{
		// If we're dripping... go through the drip process
		if(m_pDrips[i].nState == DRIP_STATE_DRIPPING)
		{
			UpdateDripping(i);
		}

		// Do some falling
		if(m_pDrips[i].nState == DRIP_STATE_FALLING)
		{
			UpdateFalling(i);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::UpdateDripping
//
//	PURPOSE:	Move the drips around
//
// ----------------------------------------------------------------------- //

void CDripperFX::UpdateDripping(int nDrip)
{
	// If we don't have any particles... just return
	if(m_pDrips[nDrip].nParticles < 1)
		return;


	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fTimeDif = fTime - m_pDrips[nDrip].fStartTime;


	// See if it's time to fall yet
	if(fTimeDif > m_pDrips[nDrip].fDripTime)
	{
		// Setup the particles so they're in a trail
		LTVector vPos;
		LTFLOAT fSize = m_pDrips[nDrip].fSize;

		m_pDrips[nDrip].pParticles[0]->m_Size = fSize;
		m_pDrips[nDrip].pParticles[0]->m_Color = m_pDrips[nDrip].vColor;

		LTFLOAT fSizeOffset = (m_pDrips[nDrip].fSize / m_pDrips[nDrip].nParticles);

		for(int i = 1; i < m_pDrips[nDrip].nParticles; i++)
		{
			fSize -= fSizeOffset;

			m_pDrips[nDrip].pParticles[i]->m_Size = fSize;
			m_pDrips[nDrip].pParticles[i]->m_Color = m_pDrips[nDrip].vColor;
		}


		// Set the state to falling
		m_pDrips[nDrip].nState = DRIP_STATE_FALLING;
		m_pDrips[nDrip].fStartTime = g_pLTClient->GetTime();
	}
	else
	{
		// See if this object is using additive blending...
		uint32 dwFlags2;
		g_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags2);
		LTBOOL bAdditive = (dwFlags2 & FLAG2_ADDITIVE);

		LTFLOAT fScale = (m_pDrips[nDrip].fDripTime <= 0.0f) ? 1.0f : (fTimeDif / m_pDrips[nDrip].fDripTime);
		LTFLOAT fTemp;

		LTVector vColor = (dwFlags2 & FLAG2_ADDITIVE) ? (m_pDrips[nDrip].vColor * (fScale * 0.75f)) : m_pDrips[nDrip].vColor;

		LTVector vPos, vTemp;
		g_pLTClient->GetParticlePos(m_hObject, m_pDrips[nDrip].pParticles[0], &vPos);
		vPos.y = m_vServObjDims.y;

		for(int i = 0; i < m_pDrips[nDrip].nParticles; i++)
		{
			// Calculate the size of the particles in an hourglass shape
			fTemp = (float)i / (float)(m_pDrips[nDrip].nParticles - 1);

			// Update their positions based off the trail length
			vTemp = vPos;
			vTemp.y -= (m_pDripperFX->fDripDistance * m_pDrips[nDrip].fSize) * fScale * (1.0f - fTemp);
			g_pLTClient->SetParticlePos(m_hObject, m_pDrips[nDrip].pParticles[i], &vTemp);

			m_pDrips[nDrip].pParticles[i]->m_Color = vColor;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::UpdateFalling
//
//	PURPOSE:	Move the drips around
//
// ----------------------------------------------------------------------- //

void CDripperFX::UpdateFalling(int nDrip)
{
	// If we don't have any particles... just return
	if(m_pDrips[nDrip].nParticles < 1)
		return;


	// Get the offset to move the particles
	LTFLOAT fScale = (m_pDripperFX->fAccelerateTime <= 0.0f) ? 1.0f : ((g_pLTClient->GetTime() - m_pDrips[nDrip].fStartTime) / m_pDripperFX->fAccelerateTime);
	if(fScale > 1.0f) fScale = 1.0f;

	LTFLOAT fOffset = m_pDripperFX->fDripSpeed * fScale * g_pLTClient->GetFrameTime();


	// Move the first particle with the gravity
	LTVector vPos;
	g_pLTClient->GetParticlePos(m_hObject, m_pDrips[nDrip].pParticles[0], &vPos);
	vPos.y -= fOffset;

	// If we're still above the bottom, move the particle...
	if(vPos.y > -m_vServObjDims.y)
	{
		g_pLTClient->SetParticlePos(m_hObject, m_pDrips[nDrip].pParticles[0], &vPos);
	}
	else
	{
		// Otherwise, remove the drip and play the impact...
		RemoveDrip(nDrip);
		PlayImpactFX(vPos.x, vPos.z);
		return;
	}


	// Now go through and setup the rest of the particles in a trail formation
	LTVector vPos2;
	fScale = (fScale * 0.75f) + 0.25f;

	for(int i = 1; i < m_pDrips[nDrip].nParticles; i++)
	{
		// Setup the particles so they're in a trail
		vPos2 = vPos;
		vPos2.y += (m_pDripperFX->fDripTrailLength * fScale * ((float)i / (float)(m_pDrips[nDrip].nParticles - 1)));
		g_pLTClient->SetParticlePos(m_hObject, m_pDrips[nDrip].pParticles[i], &vPos2);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperFX::PlayImpactFX
//
//	PURPOSE:	Play an impact FX at this particles hit location
//
// ----------------------------------------------------------------------- //

void CDripperFX::PlayImpactFX(LTFLOAT fR, LTFLOAT fF)
{
	// First see if we have a valid impact FX or not
	if(!m_pImpactFX)
		return;

	// Now test the percentage to see if we want to continue or not
	if(GetRandom(0.0f, 1.0f) > m_pDripperFX->fImpactFXPercent)
		return;

	// Alright... calculate the data we need to play the impact FX
	LTVector vPos, vR, vU, vF;
	LTRotation rRot;

	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
	g_pLTClient->GetMathLT()->GetRotationVectors(rRot, vR, vU, vF);

	// Fill in a create structure
	IFXCS cs;
	cs.vPos = vPos + (vR * fR) + (vU * -(m_vServObjDims.y - 0.1f)) + (vF * fF);
	cs.rSurfRot = rRot;
	cs.vDir = -vU;
	cs.vSurfNormal = vU;
	cs.bPlaySound = LTTRUE;

	// Get container information
/*	HLOCALOBJ objList[1];
	uint32 nListSize = g_pLTClient->GetPointContainers(&cs.vPos, objList, 1);

	if(nListSize > 0)
	{
		uint16 nCode;

		if(g_pLTClient->GetContainerCode(objList[0], &nCode))
		{
			cs.eCode = (ContainerCode)nCode;
		}
	}
*/
	// Create the effect
	g_pFXButeMgr->CreateImpactFX(m_pImpactFX, cs);
}


//-------------------------------------------------------------------------------------------------
// SFX_DripperFactory
//-------------------------------------------------------------------------------------------------

const SFX_DripperFactory SFX_DripperFactory::registerThis;

CSpecialFX* SFX_DripperFactory::MakeShape() const
{
	return new CDripperFX();
}

