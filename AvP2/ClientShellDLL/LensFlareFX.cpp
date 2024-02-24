// ----------------------------------------------------------------------- //
//
// MODULE  : LensFlareFX.cpp
//
// PURPOSE : LensFlare FX - Implementation
//
// CREATED : 5/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LensFlareFX.h"
#include "GameClientShell.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarMinFlareAngle;
VarTrack	g_cvarMinFlareSprAlpha;
VarTrack	g_cvarMaxFlareSprAlpha;
VarTrack	g_cvarMinFlareSprScale;
VarTrack	g_cvarMaxFlareSprScale;

VarTrack	g_cvarBlindMinScale;
VarTrack	g_cvarBlindMaxScale;
VarTrack	g_cvarBlindObjectAngle;
VarTrack	g_cvarBlindCameraAngle;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::Init
//
//	PURPOSE:	Init the lens flare fx
//
// ----------------------------------------------------------------------- //

DBOOL CLensFlareFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return DFALSE;
	if (!hMessage) return DFALSE;

	LENSFLARECREATESTRUCT lens;

	lens.InitFromMessage(lens, hMessage);
	lens.hServerObj			= hServObj;

	return Init(&lens);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LENSFLARECREATESTRUCT::InitFromMessage
//
//	PURPOSE:	Init the lens flare fx create struct based on the message
//
// ----------------------------------------------------------------------- //

DBOOL LENSFLARECREATESTRUCT::InitFromMessage(LENSFLARECREATESTRUCT & lens, 
											 HMESSAGEREAD hMessage)
{
	if (!hMessage) return DFALSE;

	lens.bInSkyBox			= (DBOOL) g_pClientDE->ReadFromMessageByte(hMessage);
	lens.bCreateSprite		= (DBOOL) g_pClientDE->ReadFromMessageByte(hMessage);
	lens.bSpriteOnly		= (DBOOL) g_pClientDE->ReadFromMessageByte(hMessage);
	lens.bUseObjectAngle	= (DBOOL) g_pClientDE->ReadFromMessageByte(hMessage);
	lens.bSpriteAdditive	= (DBOOL) g_pClientDE->ReadFromMessageByte(hMessage);
	lens.fSpriteOffset		= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fMinAngle			= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fMinSpriteAlpha	= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fMaxSpriteAlpha	= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fMinSpriteScale	= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fMaxSpriteScale	= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.hstrSpriteFile		= g_pClientDE->ReadFromMessageHString(hMessage);
	lens.bBlindingFlare		= (DBOOL) g_pClientDE->ReadFromMessageByte(hMessage);
	lens.fBlindObjectAngle	= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fBlindCameraAngle	= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fMinBlindScale		= g_pClientDE->ReadFromMessageFloat(hMessage);
	lens.fMaxBlindScale		= g_pClientDE->ReadFromMessageFloat(hMessage);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::Init
//
//	PURPOSE:	Init the lens flare fx
//
// ----------------------------------------------------------------------- //

DBOOL CLensFlareFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	m_cs = *((LENSFLARECREATESTRUCT*)psfxCreateStruct);

	g_cvarMinFlareAngle.Init(g_pClientDE, "FlareMin", NULL, -1.0f);
	g_cvarMinFlareSprAlpha.Init(g_pClientDE, "FlareSprAlphaMin", NULL, -1.0f);
	g_cvarMaxFlareSprAlpha.Init(g_pClientDE, "FlareSprAlphaMax", NULL, -1.0f);
	g_cvarMinFlareSprScale.Init(g_pClientDE, "FlareSprScaleMin", NULL, -1.0f);
	g_cvarMaxFlareSprScale.Init(g_pClientDE, "FlareSprScaleMax", NULL, -1.0f);

	g_cvarBlindMinScale.Init(g_pClientDE, "FlareBlindScaleMin", NULL, -1.0f);
	g_cvarBlindMaxScale.Init(g_pClientDE, "FlareBlindScaleMax", NULL, -1.0f);
	g_cvarBlindObjectAngle.Init(g_pClientDE, "FlareBlindObjAngle", NULL, -1.0f);
	g_cvarBlindCameraAngle.Init(g_pClientDE, "FlareBlindCamAngle", NULL, -1.0f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

DBOOL CLensFlareFX::CreateObject(CClientDE* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return DFALSE;

	if (!m_cs.bCreateSprite) return DTRUE;   // Don't want a sprite		
	if (!m_cs.hstrSpriteFile) return DFALSE; // Wanted a sprite, but no filename

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	// Create the lens flare sprite...

	m_pClientDE->GetObjectPos(m_hServerObject, &(createStruct.m_Pos));
	createStruct.m_ObjectType = OT_SPRITE;

	char* pFilename = m_pClientDE->GetStringData(m_cs.hstrSpriteFile);
	if (!pFilename) return DFALSE;

	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	createStruct.m_Flags = FLAG_NOLIGHT | FLAG_SPRITEBIAS;

	if (m_cs.bSpriteAdditive)
	{
		createStruct.m_Flags2 = FLAG2_ADDITIVE;  
	}

	m_hFlare = m_pClientDE->CreateObject(&createStruct);
	if (!m_hFlare) return DFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::SetMode
//
//	PURPOSE:	Do somting about vision modes
//
// ----------------------------------------------------------------------- //

void  CLensFlareFX::SetMode(const std::string& mode)
{
	if(mode == "NightVision")
	{
		m_bWashout = LTTRUE;
	}
	else
	{
		//re-set the bool
		m_bWashout = LTFALSE;

		//turn off any screen tint
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::Update
//
//	PURPOSE:	Update the lens flare fx
//
// ----------------------------------------------------------------------- //

DBOOL CLensFlareFX::Update()
{
	if (!m_pClientDE || !m_hServerObject || IsWaitingForRemove() ) return DFALSE;

	DDWORD dwFlags = 0;

	// Hide/show the flare object if necessary...

	if (m_hServerObject)
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			if (m_hFlare)
			{
				dwFlags = m_pClientDE->GetObjectFlags(m_hFlare);
				m_pClientDE->SetObjectFlags(m_hFlare, dwFlags & ~FLAG_VISIBLE);
			}

			return DTRUE;
		}
		else
		{
			if (m_hFlare)
			{
				dwFlags = m_pClientDE->GetObjectFlags(m_hFlare);
				m_pClientDE->SetObjectFlags(m_hFlare, dwFlags | FLAG_VISIBLE);
			}
		}
	}

	
	// Only show the flare if the camera is looking at it...

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	if (!hCamera) return DFALSE;

	DVector vPos, vCamPos;
	m_pClientDE->GetObjectPos(hCamera, &vCamPos);
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	// If our server object is in the sky box, convert its position to
	// real world coordinates...

	if (m_cs.bInSkyBox)
	{
		SkyDef sky;
		m_pClientDE->GetSkyDef(&sky);

		DVector vSkyCenter = (sky.m_Min + ((sky.m_Max - sky.m_Min) / 2.0f));
		vPos = vCamPos + (vPos - vSkyCenter);
	}


	DRotation rCamRot, rObjRot;
	m_pClientDE->GetObjectRotation(hCamera, &rCamRot);
	m_pClientDE->GetObjectRotation(m_hServerObject, &rObjRot);

	DVector vU, vR, vF, vObjU, vObjR, vObjF;
	m_pClientDE->GetRotationVectors(&rCamRot, &vU, &vR, &vF);
	m_pClientDE->GetRotationVectors(&rObjRot, &vObjU, &vObjR, &vObjF);

	DVector vDir = vPos - vCamPos;
	vDir.Norm();

	DFLOAT fCameraAngle = VEC_DOT(vDir, vF);
	fCameraAngle = fCameraAngle < 0.0f ? 0.0f : fCameraAngle;
	fCameraAngle = fCameraAngle > 1.0f ? 1.0f : fCameraAngle;
	fCameraAngle = MATH_RADIANS_TO_DEGREES((LTFLOAT)acos(fCameraAngle));

	DFLOAT fObjectAngle = VEC_DOT(-vDir, vObjF);
	fObjectAngle = fObjectAngle < 0.0f ? 0.0f : fObjectAngle;
	fObjectAngle = fObjectAngle > 1.0f ? 1.0f : fObjectAngle;
	fObjectAngle = MATH_RADIANS_TO_DEGREES((LTFLOAT)acos(fObjectAngle));

	DFLOAT fMinAngle = m_cs.fMinAngle;

	if (fObjectAngle + fCameraAngle > fMinAngle*2)
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
		g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags & ~FLAG_VISIBLE);
	}
	else
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
		g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags | FLAG_VISIBLE);

		DFLOAT fVal;
		
		if(m_cs.bUseObjectAngle)
			fVal = 1-(fObjectAngle + fCameraAngle)/45.0f;
		else
			fVal = 1-fCameraAngle/45.0f;

		// Cap it
		if(fVal > 1.0f)
			fVal = 1.0f;

		if(fVal < 0)
			fVal = 0;
	
		// Calculate new alpha...

		DFLOAT fMinAlpha = m_cs.fMinSpriteAlpha;
		DFLOAT fMaxAlpha = m_cs.fMaxSpriteAlpha;
		DFLOAT fAlphaRange = fMaxAlpha - fMinAlpha;

		DFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(m_hFlare, &r, &g, &b, &a);

		a = fMinAlpha + (fVal * fAlphaRange);
		g_pLTClient->SetObjectColor(m_hFlare, r, g, b, a);

		// Calculate new scale...

		DFLOAT fMinScale = m_cs.fMinSpriteScale;
		DFLOAT fMaxScale = m_cs.fMaxSpriteScale;
		DFLOAT fScaleRange = fMaxScale - fMinScale;

		DFLOAT fScale = fMinScale + (fVal * fScaleRange);
		DVector vScale(fScale, fScale, fScale);
		g_pLTClient->SetObjectScale(m_hFlare, &vScale);

		// Don't do any more processing if the alpha is 0...

		if (a < 0.001f) return LTTRUE;

		//  See if we should wash out the screen
		if(m_bWashout)
		{
			LTFLOAT fWash = (90.0f - (fCameraAngle + fObjectAngle)) / 90.0f;
			CameraMgrFX_SetScreenWash(g_pGameClientShell->GetPlayerCamera(), fWash);
		}
		
		// See if we should make a "bliding" flare, and if so see if the
		// camera is looking directly at the flare...
		DDWORD dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
		dwFlags &= ~FLAG_SPRITE_NOZ;
		dwFlags |= FLAG_SPRITEBIAS;
		g_pLTClient->SetObjectFlags(m_hFlare, dwFlags);

		if (m_cs.bBlindingFlare)
		{
			DFLOAT fBlindObjAngle = m_cs.fBlindObjectAngle;
			DFLOAT fBlindCamAngle = m_cs.fBlindCameraAngle;

			LTBOOL bDoBlind = LTFALSE;

			if(m_cs.bUseObjectAngle)
				bDoBlind = (fObjectAngle < fBlindObjAngle) && (fCameraAngle < fBlindCamAngle);
			else
				bDoBlind = (fCameraAngle < fBlindCamAngle);

		
			if (bDoBlind)
			{
				// Update the no-z flare if possible...
				DDWORD dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);

				// Make sure there is a clear path from the flare to the camera...
					
				HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
				HOBJECT hFilterList[] = {hPlayerObj, g_pGameClientShell->GetPlayerMovement()->GetObject(), DNULL};

				IntersectInfo iInfo;
				IntersectQuery qInfo;
				qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
				qInfo.m_FilterFn	= ObjListFilterFn;
				qInfo.m_pUserData	= g_pGameClientShell->IsFirstPerson() ? hFilterList : DNULL;
				qInfo.m_From		= vPos;
				qInfo.m_To			= vCamPos;

				if (g_pInterface->IntersectSegment(&qInfo, &iInfo))
				{
					// Doesn't look quite as good, but will clip on world/objects...

					dwFlags &= ~FLAG_SPRITE_NOZ;
					dwFlags |= FLAG_SPRITEBIAS;
				}
				else
				{
					// Calculate new flare scale...
					fMaxScale = m_cs.fMaxBlindScale;
					fMinScale = m_cs.fMinBlindScale;
					fScaleRange =  fMaxScale - fMinScale;

					fScale = fMinScale + (fVal * fScaleRange);
					vScale.Init(fScale, fScale, fScale);
					g_pLTClient->SetObjectScale(m_hFlare, &vScale);


					// This looks better, but will show through world/objects...

					dwFlags &= ~FLAG_SPRITEBIAS;
					dwFlags |= FLAG_SPRITE_NOZ;
				}
				
				g_pLTClient->SetObjectFlags(m_hFlare, dwFlags);
			}
		}
	}

	return DTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_LensFlareFactory
//-------------------------------------------------------------------------------------------------

const SFX_LensFlareFactory SFX_LensFlareFactory::registerThis;

CSpecialFX* SFX_LensFlareFactory::MakeShape() const
{
	return new CLensFlareFX();
}

