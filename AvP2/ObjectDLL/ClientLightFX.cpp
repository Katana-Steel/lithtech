// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLightFX.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 07/18/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "cpp_server_de.h"
#include "ClientLightFX.h"
#include "SFXMsgIds.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "light_anim_lt.h"
#include "lt_pre_light_anim.h"
#include "LightGroup.h"
#include "ServerSoundMgr.h"

#ifndef __MSG_IDS_H__
#include "MsgIDs.h"
#endif

#include <string>
#include <algorithm>

#define UPDATE_DELTA			0.1f

ClientLightFX::ClientLightFXList ClientLightFX::m_sClientLightFXs;

// -----------------------------------------------------------------------
// ClientLightFX light animations are built off the name + an extension.
// -----------------------------------------------------------------------
static const std::string LIGHTFX_LIGHTANIM_EXTENSION("__LA");

// Register the classes so they can be read by DEdit.
BEGIN_CLASS(ClientLightFX)
 	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP1, 0)
	ADD_REALPROP(HitPoints, 1.0f)
    ADD_BOOLPROP(StartOn, DTRUE)
    ADD_REALPROP(LifeTime, 0.0f)
    ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)
    ADD_REALPROP(IntensityMin, 0.5f)
    ADD_REALPROP(IntensityMax, 1.0f)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_NONE)
	ADD_REALPROP(IntensityFreq, 4.0f)
	ADD_REALPROP(IntensityPhase, 0.0f)

 	ADD_BOOLPROP(DirLight, DFALSE)
	ADD_REALPROP_FLAG(DirLightRadius, 500.0f, PF_FOVRADIUS)
	ADD_REALPROP_FLAG(FOV, 90.0f, PF_FIELDOFVIEW)

	ADD_REALPROP_FLAG(RadiusMin, 200.0f, PF_RADIUS)
    ADD_REALPROP_FLAG(RadiusMax, 500.0f, PF_RADIUS)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_NONE)
	ADD_REALPROP(RadiusFreq, 4.0f)
	ADD_REALPROP(RadiusPhase, 0.0f)

	ADD_STRINGPROP_FLAG(OnSound, "None", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(OffSound, "None", PF_STATICLIST)

	ADD_BOOLPROP(CastShadowsFlag, DFALSE)
	ADD_BOOLPROP(SolidLightFlag, DFALSE)
	ADD_BOOLPROP(OnlyLightWorldFlag, DFALSE)
	ADD_BOOLPROP(DontLightBackfacingFlag, DFALSE)
	ADD_BOOLPROP(UseLightAnims, DTRUE)
	ADD_BOOLPROP(UseShadowMaps, DTRUE)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS_PLUGIN(ClientLightFX, BaseClass, NULL, NULL, 0, CLightFXPlugin)


// Additional light classes (w/ waveforms preset)

// Square waveform
BEGIN_CLASS(SquareWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SQUARE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SQUARE)
END_CLASS_DEFAULT_FLAGS(SquareWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Saw waveform
BEGIN_CLASS(SawWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SAW)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SAW)
END_CLASS_DEFAULT_FLAGS(SawWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Rampup waveform
BEGIN_CLASS(RampUpWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_RAMPUP)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_RAMPUP)
END_CLASS_DEFAULT_FLAGS(RampUpWaveLightFX, ClientLightFX, NULL, NULL, 0)


// RampDown waveform
BEGIN_CLASS(RampDownWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_RAMPDOWN)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_RAMPDOWN)
END_CLASS_DEFAULT_FLAGS(RampDownWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Sine waveform
BEGIN_CLASS(SineWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SINE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SINE)
END_CLASS_DEFAULT_FLAGS(SineWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker1 waveform
BEGIN_CLASS(Flicker1WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER1)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER1)
END_CLASS_DEFAULT_FLAGS(Flicker1WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker2 waveform
BEGIN_CLASS(Flicker2WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER2)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER2)
END_CLASS_DEFAULT_FLAGS(Flicker2WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker3 waveform
BEGIN_CLASS(Flicker3WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER3)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER3)
END_CLASS_DEFAULT_FLAGS(Flicker3WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker4 waveform
BEGIN_CLASS(Flicker4WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER4)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER4)
END_CLASS_DEFAULT_FLAGS(Flicker4WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Strobe waveform
BEGIN_CLASS(StrobeWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_STROBE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_STROBE)
END_CLASS_DEFAULT_FLAGS(StrobeWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Search waveform
BEGIN_CLASS(SearchWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SEARCH)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SEARCH)
END_CLASS_DEFAULT_FLAGS(SearchWaveLightFX, ClientLightFX, NULL, NULL, 0)


// For compatibility:
BEGIN_CLASS(FlickerLight)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER2)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER2)
END_CLASS_DEFAULT_FLAGS(FlickerLight, ClientLightFX, NULL, NULL, 0)

BEGIN_CLASS(GlowingLight)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SINE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SINE)
END_CLASS_DEFAULT_FLAGS(GlowingLight, ClientLightFX, NULL, NULL, 0)




// ----------------------------------------------------------------------- //
// Preprocessor callback to build a light animation frame.
// ----------------------------------------------------------------------- //
// PreLightLT is a preprocessor lighting interface
// HPREOBJECT is a handle used to identify the object being worked on
DRESULT	CLightFXPlugin::PreHook_Light(
	PreLightLT *pInterface, 
	HPREOBJECT hObject)
{
	//-------------------------------------------------------------------------
	// Only generate an animation if we need to.
	//-------------------------------------------------------------------------
	GenericProp genProp;
	if(pInterface->GetPropGeneric(hObject, "UseLightAnims", &genProp) != LT_OK || !genProp.m_Bool)
		return LT_OK;

	// Get name
	pInterface->GetPropGeneric(hObject, "Name", &genProp);
	std::string objectName(genProp.m_String);

	// Check to see if this is in a group
	if(LightManager::GetInstance()->InGroup(objectName))
	{
		return LT_OK;
	}
	//-------------------------------------------------------------------------

	// OK, so now we need to.
	PreLightInfo lightInfo;

	// Make animation Name
	std::string animName = objectName + LIGHTFX_LIGHTANIM_EXTENSION;

	// Get position
	pInterface->GetPropGeneric(hObject, "Pos", &genProp);
	lightInfo.m_vPos = genProp.m_Vec;

	// Get Color
	pInterface->GetPropGeneric(hObject, "Color", &genProp);
	lightInfo.m_vInnerColor = genProp.m_Vec;
	lightInfo.m_vOuterColor.Init();

	// Is this a directional light?
	pInterface->GetPropGeneric(hObject, "DirLight", &genProp);
	lightInfo.m_bDirectional = genProp.m_Bool;

	// Get radius based on what type of light it is.
	if(lightInfo.m_bDirectional)
		pInterface->GetPropGeneric(hObject, "DirLightRadius", &genProp);
	else
		pInterface->GetPropGeneric(hObject, "RadiusMax", &genProp);
	
	lightInfo.m_Radius = genProp.m_Float;

	// Get Field Of View
	pInterface->GetPropGeneric(hObject, "FOV", &genProp);
	lightInfo.m_FOV = MATH_DEGREES_TO_RADIANS(genProp.m_Float);

	// Get rotation
	DVector vRight, vUp;
	pInterface->GetPropGeneric(hObject, "Rotation", &genProp);
	pInterface->GetMathLT()->GetRotationVectors(genProp.m_Rotation, vRight, vUp, lightInfo.m_vDirection);

	// Is this a shadow map? otherwise it is a light map.
	pInterface->GetPropGeneric(hObject, "UseShadowMaps", &genProp);
	DBOOL bUseShadowMaps = genProp.m_Bool;

	// Make the Frame object
	PreLightAnimFrameInfo frame;		// This should have a constructor that takes these arguments !!!JKE
	frame.m_bSunLight = DFALSE;			// No Sunlight
	frame.m_Lights = &lightInfo;		// Give frame the address of the light info
	frame.m_nLights = 1;				// How many lights did we give above.

	// We have all the information we need, so Create the light maps.
	pInterface->CreateLightAnim(animName.c_str(), &frame, 1, bUseShadowMaps);
	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFXPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CLightFXPlugin::PreHook_EditStringList(const char* szRezPath, 
												 const char* szPropName, 
												 char* const * aszStrings, 
												 uint32* pcStrings, 
												 const uint32 cMaxStrings, 
												 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if((_stricmp("OnSound", szPropName) == 0) || (_stricmp("OffSound", szPropName) == 0))
	{
		m_SoundButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength, "BaseSFXObjSound");

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


ClientLightFXCreateStruct::ClientLightFXCreateStruct() : m_bOn(DTRUE), m_vColor(255.0f, 255.0f, 255.0f),
	m_dwLightFlags(0), 	m_fIntensityMin(0.5f), m_fIntensityMax(1.0f), m_nIntensityWaveform(WAVE_NONE),
	m_fIntensityFreq(4.0f), m_fIntensityPhase(0), m_fRadiusMin(0.0f), m_fRadiusMax(500.0f),
	m_nRadiusWaveform(WAVE_NONE), m_fRadiusFreq(0.0f), m_fRadiusPhase(4.0f), m_nOnSound(-1),
	m_nOffSound(-1), m_bUseLightAnims(DTRUE), m_bDynamic(DTRUE),	m_fLifeTime(-1.0f),
	m_fStartTime(0.0f), m_fHitPts(1.0f)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ClientLightFX::ClientLightFX() : m_fIntensityDelta(0.0f)
{
	m_sClientLightFXs.push_back(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::~ClientLightFX()	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ClientLightFX::~ClientLightFX()
{
	ClientLightFXList::iterator iter_to_me = std::find(	m_sClientLightFXs.begin(), m_sClientLightFXs.end(), this);
	if( iter_to_me != m_sClientLightFXs.end() )
	{
		m_sClientLightFXs.erase(iter_to_me);
	}
}


LTVector ClientLightFX::GetPos() const
{
	ASSERT( g_pLTServer );
	ASSERT( m_hObject );

	LTVector vResult;
	g_pLTServer->GetObjectPos(m_hObject, &vResult);

	return vResult;
}

LTFLOAT ClientLightFX::GetRadiusSqr() const
{
	
	if( g_pLTServer->GetObjectUserFlags(m_hObject) & USRFLG_VISIBLE )
		return m_LightData.m_fRadiusMax * m_LightData.m_fRadiusMax;

	return 0.0f;
}

LTVector ClientLightFX::GetColor(const LTVector & /*unused*/) const
{
	if( g_pLTServer->GetObjectUserFlags(m_hObject) & USRFLG_VISIBLE )
		return m_LightData.m_vColor * 255.0f;

	return LTVector(0,0,0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ClientLightFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				m_LightData.m_bDynamic = DFALSE;
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;
    
		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
				SendEffectMessage();
			}
			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{
    		if (!Update()) 
            {
		    	CServerDE* pServerDE = BaseClass::GetServerDE();
			    if (pServerDE) pServerDE->RemoveObject(m_hObject);
            }
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
DDWORD ClientLightFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}

		default : break;
	}

	return BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
void ClientLightFX::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	std::string message = g_pServerDE->GetStringData( hMsg );

	std::transform(message.begin(), message.end(), message.begin(), toupper);

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	DDWORD dwFlags    = g_pServerDE->GetObjectFlags(m_hObject);

    if ( message == "TOGGLE")
    {
		// Toggle the flag
		if (dwUsrFlags & USRFLG_VISIBLE)
		{
			dwUsrFlags &= ~USRFLG_VISIBLE;
			dwFlags	   &= ~FLAG_VISIBLE;
		}
		else
		{
			dwUsrFlags |= USRFLG_VISIBLE;
			dwFlags	   |= FLAG_VISIBLE;
		}
    } 
    else if (message == "ON")
    {
		dwUsrFlags |= USRFLG_VISIBLE;
		dwFlags	   |= FLAG_VISIBLE;
    }            
    else if ( message == "OFF")
    {
		dwUsrFlags &= ~USRFLG_VISIBLE;
 		dwFlags	   &= ~FLAG_VISIBLE;
	}        
	else
	{
		std::string::size_type commandStart = message.find("DIM ");
		if (commandStart != std::string::npos)
		{
			// We have a DIM command.
			// Get the tail off

			std::string::size_type valueStart = message.find("(");
			std::string::size_type valueEnd = message.find(")");
			if (std::string::npos == valueStart)
			{
				valueStart = commandStart+4;
			}
			else
			{
				++valueStart;
			}

			const std::string value = message.substr(valueStart, valueEnd - valueStart);
			const float amount = (LTFLOAT)atof(value.c_str());

			m_fIntensityDelta += amount;

			// Cap values here...
			if(m_fIntensityDelta > 1.0f)	m_fIntensityDelta = 1.0f;
			if(m_fIntensityDelta < -1.0f)	m_fIntensityDelta = -1.0f;

			float intMin = m_LightData.m_fIntensityMin + m_fIntensityDelta;
			float intMax = m_LightData.m_fIntensityMax + m_fIntensityDelta;

			if (intMin < 0)
			{
				intMin = 0;
			}

			if (intMin > 1.0)
			{
				intMin = 1.0;
			}

			if (intMax < 0)
			{
				intMax = 0;
			}

			if (intMax > 1.0)
			{
				intMax = 1.0;
			}

			float radMin = m_LightData.m_fRadiusMin * (1.0f + m_fIntensityDelta);
			float radMax = m_LightData.m_fRadiusMax * (1.0f + m_fIntensityDelta);

			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_LIGHT_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);

			g_pLTServer->WriteToMessageFloat(hMessage, intMin);
			g_pLTServer->WriteToMessageFloat(hMessage, intMax);

			g_pLTServer->WriteToMessageFloat(hMessage, radMin);
			g_pLTServer->WriteToMessageFloat(hMessage, radMax);

			g_pLTServer->EndMessage(hMessage);
		}
	}
    
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	g_pServerDE->FreeString( hMsg );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
DBOOL ClientLightFX::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("HitPoints", &genProp) == DE_OK)
		m_LightData.m_fHitPts = genProp.m_Float;

	if (pServerDE->GetPropGeneric("StartOn", &genProp) == DE_OK)
		m_LightData.m_bOn = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("LifeTime", &genProp) == DE_OK)
		m_LightData.m_fLifeTime = genProp.m_Float;

    if (m_LightData.m_fLifeTime < 0.0f) m_LightData.m_fLifeTime = 0.0f;

	if (pServerDE->GetPropGeneric("Color", &genProp) == DE_OK)
		VEC_COPY(m_LightData.m_vColor, genProp.m_Color);

	if (pServerDE->GetPropGeneric("IntensityMin", &genProp) == DE_OK)
		m_LightData.m_fIntensityMin = genProp.m_Float;

    if (m_LightData.m_fIntensityMin < 0.0f) m_LightData.m_fIntensityMin = 0.0f;

	if (pServerDE->GetPropGeneric("IntensityMax", &genProp) == DE_OK)
		m_LightData.m_fIntensityMax = genProp.m_Float;

    if (m_LightData.m_fIntensityMax > 255.0f)   m_LightData.m_fIntensityMax = 255.0f;

	if (pServerDE->GetPropGeneric("IntensityWaveform", &genProp) == DE_OK)
		m_LightData.m_nIntensityWaveform = (DBYTE) genProp.m_Long;

	if (pServerDE->GetPropGeneric("IntensityFreq", &genProp) == DE_OK)
		m_LightData.m_fIntensityFreq = genProp.m_Float;

	if (pServerDE->GetPropGeneric("IntensityPhase", &genProp) == DE_OK)
		m_LightData.m_fIntensityPhase = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusMin", &genProp) == DE_OK)
		m_LightData.m_fRadiusMin = genProp.m_Float;

    if (m_LightData.m_fRadiusMin < 0.0f) m_LightData.m_fRadiusMin = 0.0f;

	if (pServerDE->GetPropGeneric("RadiusMax", &genProp) == DE_OK)
		m_LightData.m_fRadiusMax = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusWaveform", &genProp) == DE_OK)
		m_LightData.m_nRadiusWaveform = (DBYTE) genProp.m_Long;

	if (pServerDE->GetPropGeneric("RadiusFreq", &genProp) == DE_OK)
		m_LightData.m_fRadiusFreq = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusPhase", &genProp) == DE_OK)
		m_LightData.m_fRadiusPhase = genProp.m_Float;


	m_LightData.m_nOnSound = -1;
	m_LightData.m_nOffSound = -1;

	if (pServerDE->GetPropGeneric("OnSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_LightData.m_nOnSound = ( uint16 )g_pSoundButeMgr->GetSoundSetFromName(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("OffSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_LightData.m_nOffSound = ( uint16 )g_pSoundButeMgr->GetSoundSetFromName(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("CastShadowsFlag", &genProp) == DE_OK)
	{
		m_LightData.m_dwLightFlags |= genProp.m_Bool ? FLAG_CASTSHADOWS : 0;
	}

	if (pServerDE->GetPropGeneric("SolidLightFlag", &genProp) == DE_OK)
	{
		m_LightData.m_dwLightFlags |= genProp.m_Bool ? FLAG_SOLIDLIGHT : 0;
	}

	if (pServerDE->GetPropGeneric("OnlyLightWorldFlag", &genProp) == DE_OK)
	{
		m_LightData.m_dwLightFlags |= genProp.m_Bool ? FLAG_ONLYLIGHTWORLD : 0;
	}

	if (pServerDE->GetPropGeneric("DontLightBackfacingFlag", &genProp) == DE_OK)
	{
		m_LightData.m_dwLightFlags |= genProp.m_Bool ? FLAG_DONTLIGHTBACKFACING : 0;
	}

	m_LightData.m_bUseLightAnims = DFALSE;
	if(pServerDE->GetPropGeneric("UseLightAnims", &genProp) == DE_OK)
	{
		m_LightData.m_bUseLightAnims = genProp.m_Bool;
	}

	return DTRUE;
}

      
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void ClientLightFX::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags = FLAG_VISIBLE;
	pStruct->m_Flags |= FLAG_GOTHRUWORLD;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL ClientLightFX::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	//now reset the rayhit flag
//	uint32 dwFlags;
//	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
//	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags & ~FLAG_RAYHIT);


    // Set Next update (randomize it if this object was loaded from the
	// level - so we don't have all the lights updating on the same frame)...
	
	DFLOAT fOffset = 0.0f;
	if (!m_LightData.m_bDynamic) fOffset = pServerDE->Random(0.01f, 0.5f);

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA + fOffset);

	Init();


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Init()
//
//	PURPOSE:	Initialize data members
//
// ----------------------------------------------------------------------- //

DBOOL ClientLightFX::Init()
{
	DVector vDims;
	float fDims;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;


	VEC_DIVSCALAR(m_LightData.m_vColor, m_LightData.m_vColor, 255.0f);

	m_LightData.m_fStartTime = pServerDE->GetTime();

	m_LightData.m_fIntensityPhase = MATH_DEGREES_TO_RADIANS(m_LightData.m_fIntensityPhase);
	m_LightData.m_fRadiusPhase = MATH_DEGREES_TO_RADIANS(m_LightData.m_fRadiusPhase);

	if (m_LightData.m_bOn)
	{
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		dwUsrFlags |= USRFLG_VISIBLE;
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}

	// Set the dims to something to avoid situations where the object is considered
	// invisible even though it's visible.
	fDims = DMAX(m_LightData.m_fRadiusMin, 5.0f);
	vDims.Init(fDims, fDims, fDims);
	pServerDE->SetObjectDims(m_hObject, &vDims);

	return DTRUE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
DBOOL ClientLightFX::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	DBOOL bRemove = DFALSE;
	if (m_LightData.m_fLifeTime > 0 && (pServerDE->GetTime() - m_LightData.m_fStartTime) >= m_LightData.m_fLifeTime)
	{
		bRemove = DTRUE;
	}

	return (!bRemove);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::SendEffectMessage
//
//	PURPOSE:	Sends a message to the client to start a light effect
//
// ----------------------------------------------------------------------- //

void ClientLightFX::SendEffectMessage(LTBOOL bForceMessage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Get our name
	std::string objectName(pServerDE->GetObjectName(m_hObject));

	// Check to see if this is in a group
	if(LightManager::GetInstance()->InGroup(objectName) && !bForceMessage)
	{
		return;
	}

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_LIGHT_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_LightData.m_vColor);
	pServerDE->WriteToMessageDWord(hMessage, m_LightData.m_dwLightFlags);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fIntensityMin);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fIntensityMax);
	pServerDE->WriteToMessageByte(hMessage, m_LightData.m_nIntensityWaveform);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fIntensityFreq);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fIntensityPhase);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fRadiusMin);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fRadiusMax);
	pServerDE->WriteToMessageByte(hMessage, m_LightData.m_nRadiusWaveform);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fRadiusFreq);
	pServerDE->WriteToMessageFloat(hMessage, m_LightData.m_fRadiusPhase);
	pServerDE->WriteToMessageWord(hMessage, m_LightData.m_nOnSound);
	pServerDE->WriteToMessageWord(hMessage, m_LightData.m_nOffSound);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_LightData.m_bUseLightAnims);

	// Write the animation handle.	
	if(m_LightData.m_bUseLightAnims)
	{
		std::string lightAnimName;
		
		// Check to see if this is in a group
		if(LightManager::GetInstance()->InGroup(objectName))
		{
			lightAnimName = 
				LightManager::GetInstance()->GetGroup(objectName) + LightGroup::LIGHTFX_LIGHTGROUP_EXTENSION;
		}
		else
		{
			lightAnimName = 
 				objectName + LIGHTFX_LIGHTANIM_EXTENSION;
		}

		HLIGHTANIM hLightAnim = INVALID_LIGHT_ANIM;
		pServerDE->GetLightAnimLT()->FindLightAnim(lightAnimName.c_str(), hLightAnim);

		pServerDE->WriteToMessageDWord(hMessage, (DDWORD)hLightAnim);
	}

	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...

	pServerDE->WriteToMessageByte(hWrite, m_LightData.m_bOn);
	pServerDE->WriteToMessageByte(hWrite, m_LightData.m_bDynamic);
	pServerDE->WriteToMessageFloat(hWrite, m_LightData.m_fLifeTime);
	pServerDE->WriteToMessageFloat(hWrite, (m_LightData.m_fStartTime - g_pLTServer->GetTime()) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_LightData.m_bOn			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_LightData.m_bDynamic	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_LightData.m_fLifeTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_LightData.m_fStartTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_LightData.m_fStartTime += g_pLTServer->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::CacheFiles()
{

}


void ClientLightFX::Setup(ClientLightFXCreateStruct createLight)
{
	m_LightData = createLight;

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	dwUsrFlags &= ~USRFLG_VISIBLE;
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	InitialUpdate(NULL);
	SendEffectMessage(LTTRUE);
}


