// ----------------------------------------------------------------------- //
//
// MODULE  : LightFX.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 02/04/98
//			  7/17/98 - Converted to client SFX.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "LightFX.h"
#include "ClientServerShared.h"
#include "ClientSoundMgr.h"

#include "ltbasedefs.h"


// Waveform values taken from Blood

// monotonic flicker -- very doom like
static char flicker1[] = {
	0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0,
	1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1,
};

// organic flicker -- good for torches
static DFLOAT flicker2[] = {
	1, 2, 4, 2, 3, 4, 3, 2, 0, 0, 1, 2, 4, 3, 2, 0,
	2, 1, 0, 1, 0, 2, 3, 4, 3, 2, 1, 1, 2, 0, 0, 1,
	1, 2, 3, 4, 4, 3, 2, 1, 2, 3, 4, 4, 2, 1, 0, 1,
	0, 0, 0, 0, 1, 2, 3, 4, 3, 2, 1, 2, 3, 4, 3, 2,
};

// mostly on flicker -- good for flaky fluourescents
static DFLOAT flicker3[] = {
	4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 2, 4, 3, 4, 4,
	4, 4, 2, 1, 3, 3, 3, 4, 3, 4, 4, 4, 4, 4, 2, 4,
	4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 1, 0, 1,
	0, 1, 0, 1, 0, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 4,
};

// mostly off flicker -- good for really flaky fluourescents
static DFLOAT flicker4[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 0, 0, 3, 0, 1, 0, 1, 0, 4, 4, 4, 4, 4, 2, 0,
	0, 0, 0, 4, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 4, 3, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static DFLOAT strobe[] = {
	64, 64, 64, 48, 36, 27, 20, 15, 11, 9, 6, 5, 4, 3, 2, 2,
	1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

//static int GetWaveValue( int nWave, int theta, int amplitude )
static DFLOAT GetWaveValue(DBYTE nWaveform, DFLOAT fMin, DFLOAT fMax, DFLOAT fTheta )
{
	DFLOAT fReturn = fMax;

	fTheta = (DFLOAT)fmod(fTheta, MATH_CIRCLE);

	switch (nWaveform)
	{
		case WAVE_SQUARE:
			fReturn = (fTheta <= MATH_PI) ? fMin : fMax;
			break;

		case WAVE_SAW:
		{
			if (fTheta < MATH_PI)
				fReturn = fMin + (fMax - fMin) * fTheta / MATH_PI;
			else 
				fReturn = fMin + (fMax - fMin) * (MATH_CIRCLE - fTheta) / MATH_PI;
			break;
		}

		case WAVE_RAMPUP:
			fReturn = fMin + (fMax - fMin) * fTheta / (MATH_CIRCLE);
			break;

		case WAVE_RAMPDOWN:
			fReturn = fMin + (fMax - fMin) * (MATH_CIRCLE - fTheta) / (MATH_CIRCLE);
			break;

		case WAVE_SINE:
			fReturn = fMin + (fMax - fMin) * ((DFLOAT)sin(fTheta)/2.0f + 0.5f);
			break;

		case WAVE_FLICKER1:
		{
			int index = (int)((fTheta/(MATH_CIRCLE))*63);
			fReturn = fMin + (fMax - fMin) * (flicker1[index]);
			break;
		}

		case WAVE_FLICKER2:
		{
			int index = (int)((fTheta/(MATH_CIRCLE))*63);
			fReturn = fMin + (fMax - fMin) * (flicker2[index]/4.0f);
			break;
		}

		case WAVE_FLICKER3:
		{
			int index = (int)((fTheta/(MATH_CIRCLE))*63);
			fReturn = fMin + (fMax - fMin) * (flicker3[index]/4.0f);
			break;
		}

		case WAVE_FLICKER4:
		{
			int index = (int)((fTheta/(MATH_CIRCLE))*127);
			fReturn = fMin + (fMax - fMin) * (flicker4[index]/4.0f);
			break;
		}

		case WAVE_STROBE:
		{
			int index = (int)((fTheta/(MATH_CIRCLE))*63);
			fReturn = fMin + (fMax - fMin) * (strobe[index]/64.0f);
			break;
		}

		case WAVE_SEARCH:
		{
			fTheta *= 2.0f;
			if ( fTheta > MATH_CIRCLE )
				fReturn = fMin;
			else
				fReturn = fMin + (fMax - fMin) * ((DFLOAT)-cos(fTheta)/2.0f + 0.5f);
			break;
		}
	}

	return fReturn;
};




CLightFX::CLightFX() : CSpecialFX() 
{
	m_vColor.Init(255, 255, 255);
	m_vOffset.Init();

	m_fStartTime		= -1.0f;
	m_fIntensityMin		= 0.5f;
	m_fIntensityMax		= 1.0f;
	m_fLastIntensity	= 0.5f;
	m_fRadiusMin		= 500.0f;
	m_fRadiusMax		= 0.0f;
	m_fLastRadius		= 500.0f;
	m_fLifeTime			= -1.0f;
	m_fCurrentRadius	= 0.0f;
	m_fIntensityTime	= 0.0f;
	m_fRadiusTime		= 0.0f;
	m_fStartTime		= 0.0f;

	m_bUseServerPos		= DFALSE;
	m_nOnSound			= -1;
	m_nOffSound			= -1;

	m_hLightAnim = INVALID_LIGHT_ANIM;
	m_bUpdate			= false;

	m_hSound			= LTNULL;
}


CLightFX::~CLightFX()
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Init()
//
//	PURPOSE:	Initialize data members
//
// ----------------------------------------------------------------------- //

DBOOL CLightFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	LIGHTCREATESTRUCT light;

	light.hServerObj	= hServObj;

	light.vColor = hRead->ReadVector();
	light.dwLightFlags = hRead->ReadDWord();
	light.fIntensityMin = hRead->ReadFloat();
	light.fIntensityMax = hRead->ReadFloat();
	light.nIntensityWaveform = hRead->ReadByte();
	light.fIntensityFreq = hRead->ReadFloat();
	light.fIntensityPhase = hRead->ReadFloat();
	light.fRadiusMin = hRead->ReadFloat();
	light.fRadiusMax = hRead->ReadFloat();
	light.nRadiusWaveform = hRead->ReadByte();
	light.fRadiusFreq = hRead->ReadFloat();
	light.fRadiusPhase = hRead->ReadFloat();
	light.nOnSound = hRead->ReadWord();
	light.nOffSound = hRead->ReadWord();

	DBOOL bUseLightAnim = (DBOOL)hRead->ReadByte();

	if(bUseLightAnim)
	{
		light.m_hLightAnim = (HLIGHTANIM)hRead->ReadDWord();
	}
	else
	{
		light.m_hLightAnim = INVALID_LIGHT_ANIM;
	}

	return Init(&light);
}

DBOOL CLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) 
		return DFALSE;

	LIGHTCREATESTRUCT* pLight = (LIGHTCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vColor, pLight->vColor);
	m_dwLightFlags =		pLight->dwLightFlags;
	m_fIntensityMin = 		pLight->fIntensityMin;
	m_fIntensityMax = 		pLight->fIntensityMax;
	m_nIntensityWaveform =	pLight->nIntensityWaveform;
	m_fIntensityFreq =		pLight->fIntensityFreq;
	m_fIntensityPhase =		pLight->fIntensityPhase;
	m_fRadiusMin = 			pLight->fRadiusMin;
	m_fRadiusMax = 			pLight->fRadiusMax;
	m_nRadiusWaveform =		pLight->nRadiusWaveform;
	m_fRadiusFreq =			pLight->fRadiusFreq;
	m_fRadiusPhase =		pLight->fRadiusPhase;
	m_nOnSound =			pLight->nOnSound;
	m_nOffSound =			pLight->nOffSound;
	m_vOffset =				pLight->vOffset;
	m_hLightAnim =			pLight->m_hLightAnim;

	m_fCurrentRadius		= m_fRadius = m_fRadiusMin;

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::CreateObject
//
//	PURPOSE:	Create object associated with the light
//
// ----------------------------------------------------------------------- //

DBOOL CLightFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	DVector vZero, vPos;
	LAInfo info;
	vZero.Init();

	DRotation rRot;
	ROT_INIT(rRot);

	if (m_hServerObject)
	{
		g_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		vPos += m_vOffset;

		pClientDE->GetObjectRotation(m_hServerObject, &rRot);
	}
	else
	{
		return DFALSE;
	}

	// Setup the light...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	createStruct.m_Flags = (FLAG_ONLYLIGHTOBJECTS | FLAG_VISIBLE) | m_dwLightFlags;
	
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	m_hObject = m_pClientDE->CreateObject(&createStruct);

	if (m_hObject)
	{
		m_pClientDE->GetLightAnimLT()->GetLightAnimInfo(m_hLightAnim, info);
		UpdateLightRadius(info);
		UpdateLightIntensity(info);
		m_pClientDE->GetLightAnimLT()->SetLightAnimInfo(m_hLightAnim, info);
	}
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
DBOOL CLightFX::Update()
{
	LightAnimLT *pLightAnimLT;
	LAInfo info;

	if (!m_pClientDE) return DFALSE;

	if (m_hServerObject)
	{
		DVector vPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		vPos += m_vOffset;
		m_pClientDE->SetObjectPos(m_hObject, &vPos);
	}
	else
	{
		// We have gone away
		m_fRadius = m_fRadiusMin;  // Effectively turn light off
		SetRadius(m_fRadius, info);

		// Setup LightAnim info..
		pLightAnimLT = m_pClientDE->GetLightAnimLT();
		pLightAnimLT->GetLightAnimInfo(m_hLightAnim, info);

		// Disable..
		info.m_iFrames[0] = info.m_iFrames[1] = LIGHTANIMFRAME_NONE;
		pLightAnimLT->SetLightAnimInfo(m_hLightAnim, info);

		if(m_hSound)
		{
			g_pLTClient->KillSound(m_hSound);
			m_hSound = LTNULL;
		}

		return DFALSE;
	}

	DDWORD dwFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwFlags);
	DBOOL bOn = ((dwFlags & USRFLG_VISIBLE) != 0);

	dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

	// Setup LightAnim info..
	pLightAnimLT = m_pClientDE->GetLightAnimLT();
	pLightAnimLT->GetLightAnimInfo(m_hLightAnim, info);
	
	// Use our object's position.
	m_pClientDE->GetObjectPos(m_hObject, &info.m_vLightPos);

	if (bOn)
	{
		if (m_bUpdate || m_nIntensityWaveform != WAVE_NONE)
			UpdateLightIntensity(info);
		if (m_bUpdate || m_nRadiusWaveform != WAVE_NONE)
			UpdateLightRadius(info);

		// Check the sound
		if(m_hSound)
		{
			if(g_pLTClient->IsDone(m_hSound))
			{
				g_pLTClient->KillSound(m_hSound);
				m_hSound = LTNULL;
			}
		}

		// If we passed through above then we are updated
		m_bUpdate = false;
		// Make sure we're animating...
		info.m_iFrames[0] = info.m_iFrames[1] = 0;
		dwFlags |= FLAG_VISIBLE;
	}
	else
	{
        // Its NOT turned on, so reset the start time
        // So if there is a duration, then it will start timing when the switch is turned on
    	m_fStartTime = m_pClientDE->GetTime();
        
		m_fRadius = m_fRadiusMin;  // Effectively turn light off
		SetRadius(m_fRadius, info);

		// Disable..
		info.m_iFrames[0] = info.m_iFrames[1] = LIGHTANIMFRAME_NONE;

		dwFlags &= ~FLAG_VISIBLE;
	}

	pLightAnimLT->SetLightAnimInfo(m_hLightAnim, info);

	m_pClientDE->SetObjectFlags(m_hObject, dwFlags);
	return DTRUE;
}

LTBOOL CLightFX::OnServerMessage(HMESSAGEREAD hRead)
{
	if(!hRead) return LTFALSE;

	// Read in the new property values
	m_fIntensityMin = hRead->ReadFloat();
	m_fIntensityMax = hRead->ReadFloat();

	m_fRadiusMin = hRead->ReadFloat();
	m_fRadiusMax = hRead->ReadFloat();

	m_bUpdate = true;

	return LTTRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////



// The following are Light Radius related methods /////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CLightFX::UpdateLightRadius(LAInfo &info)
{
	// Determine a theta between 0 and MATH_CIRCLE
	DFLOAT fTheta = m_pClientDE->GetTime() * m_fRadiusFreq + m_fRadiusPhase;

	DFLOAT fValue = GetWaveValue(m_nRadiusWaveform, m_fRadiusMin, m_fRadiusMax, fTheta);


	// Play a sound
	if(m_nRadiusWaveform != WAVE_NONE)
	{
		if((m_fLastRadius == m_fRadiusMin) && (fValue > m_fRadiusMin))
		{
			PlaySound(m_nOnSound);
		}
		else if((m_fLastRadius == m_fRadiusMax) && (fValue < m_fRadiusMax))
		{
			PlaySound(m_nOffSound);
		}

		m_fLastRadius = fValue;
//		g_pLTClient->CPrint("CLightFX::UpdateLightRadius!!  Wave Value: %f  Min: %f  Max: %f", fValue, m_fRadiusMin, m_fRadiusMax);
	}


	SetRadius(fValue, info);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CLightFX::UpdateLightIntensity(LAInfo &info)
{
	float intensityMin = m_fIntensityMin;
	float intensityMax = m_fIntensityMax;

	// Determine a theta between 0 and MATH_CIRCLE
	DFLOAT fTheta = m_pClientDE->GetTime() * m_fIntensityFreq + m_fIntensityPhase;

	DFLOAT fValue = GetWaveValue(m_nIntensityWaveform, intensityMin, intensityMax, fTheta);


	// If the radius isn't being updated... play the sound here instead
	if(m_nRadiusWaveform == WAVE_NONE)
	{
		if((m_fLastIntensity == intensityMin) && (fValue > intensityMin))
		{
			PlaySound(m_nOnSound);
		}
		else if((m_fLastIntensity == intensityMax) && (fValue < intensityMax))
		{
			PlaySound(m_nOffSound);
		}

		m_fLastIntensity = fValue;
//		g_pLTClient->CPrint("CLightFX::UpdateLightIntensity!!  Wave Value: %f  Min: %f  Max: %f", fValue, intensityMin, intensityMax);
	}


	info.m_fBlendPercent = fValue;

	SetColor(m_vColor.x * fValue, m_vColor.y * fValue, m_vColor.z * fValue, info);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CLightFX::PlaySound(uint16 nBute)
{
	if(nBute == ( uint16 )-1) return;

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	if(m_hSound)
	{
		g_pLTClient->KillSound(m_hSound);
		m_hSound = LTNULL;
	}

	m_hSound = g_pClientSoundMgr->PlaySoundFromPos(vPos, nBute);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void CLightFX::SetRadius(DFLOAT fRadius, LAInfo &info)
{
	if (fRadius > 10000.0f) fRadius = 10000.0f;
    if (fRadius < 0.0f)     fRadius = 0.0f;

	info.m_fLightRadius = fRadius;

	m_pClientDE->SetLightRadius(m_hObject, fRadius);
	m_fCurrentRadius = fRadius;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void CLightFX::SetColor(DFLOAT fRedValue, DFLOAT fGreenValue, DFLOAT fBlueValue, LAInfo &info)
{
	if (fRedValue > 1.0f)       fRedValue = 1.0f;
    if (fRedValue < 0.0f)       fRedValue = 0.0f;
        
    if (fGreenValue > 1.0f)     fGreenValue = 1.0f;
    if (fGreenValue < 0.0f)     fGreenValue = 0.0f;
        
    if (fBlueValue > 1.0f)      fBlueValue = 1.0f;
    if (fBlueValue < 0.0f)      fBlueValue = 0.0f;

   	m_pClientDE->SetLightColor(m_hObject, fRedValue, fGreenValue, fBlueValue);
    
	// Update the LightAnim.
	info.m_vLightColor.Init(fRedValue*255.0f, fGreenValue*255.0f, fBlueValue*255.0f);
}



//-------------------------------------------------------------------------------------------------
// SFX_LightFactory
//-------------------------------------------------------------------------------------------------

const SFX_LightFactory SFX_LightFactory::registerThis;

CSpecialFX* SFX_LightFactory::MakeShape() const
{
	return new CLightFX();
}

