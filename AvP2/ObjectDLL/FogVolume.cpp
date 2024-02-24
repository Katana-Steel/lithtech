// ----------------------------------------------------------------------- //
//
// MODULE  : SpriteControl.cpp
//
// PURPOSE : SpriteControl implementation
//
// CREATED : 7/12/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FogVolume.h"
#include "cpp_server_de.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "ClientServerShared.h"

// ----------------------------------------------------------------------- //

BEGIN_CLASS(FogVolume)
	ADD_VECTORPROP_VAL_FLAG_HELP(Dims, 128.0f, 128.0f, 128.0f, PF_DIMS,	"The dimensions of the fog volume.")
    ADD_COLORPROP_FLAG_HELP(Color, 255.0f, 255.0f, 255.0f, 0,			"The color of the fog.")

	ADD_REALPROP_FLAG_HELP(Density, 1.0f, 0,							"The default alpha value for this object.")
	ADD_REALPROP_FLAG_HELP(MinOpacity, 0.0f, 0,							"The default alpha value for this object.")
	ADD_REALPROP_FLAG_HELP(MaxOpacity, 1.0f, 0,							"The default alpha value for this object.")

	ADD_STRINGPROP_FLAG_HELP(MapRes, "32", PF_STATICLIST,				"The resolution of the fog map.")
	ADD_STRINGPROP_FLAG_HELP(Shape, "", PF_STATICLIST,					"The shape of the fog.")
	ADD_STRINGPROP_FLAG_HELP(Gradient, "", PF_STATICLIST,				"The gradient type of the fog.")
	ADD_STRINGPROP_FLAG_HELP(DensityFn, "", PF_STATICLIST,				"The type of calculations used for the density.")

	ADD_BOOLPROP_FLAG_HELP(FogWorld, LTTRUE, 0,							"Tells if this fog should affect the world polys.")
	ADD_BOOLPROP_FLAG_HELP(FogModels, LTTRUE, 0,						"Tells if this fog should affect polys on models (ie. Props, etc.)")
	ADD_BOOLPROP_FLAG_HELP(Visible, LTTRUE, 0,							"Should this fog start as being visible?")

END_CLASS_DEFAULT_FLAGS_PLUGIN(FogVolume, GameBase, LTNULL, LTNULL, 0, CFogVolumePlugin)

// ----------------------------------------------------------------------- //

const int g_nMapRes = 6;
static char *g_szMapRes[g_nMapRes] =
{
	"2",
	"4",
	"8",
	"16",
	"32",
	"64",
};

const int g_nShapes = 3;
static char *g_szShapes[g_nShapes] =
{
	"Box",
	"Sphere",
	"Cylinder",
};

const int g_nGradients = 4;
static char *g_szGradients[g_nGradients] =
{
	"Box",
	"Sphere",
	"Cylinder",
	"Vertical",
};

const int g_nDensityFns = 2;
static char *g_szDensityFns[g_nDensityFns] =
{
	"Linear",
	"Exponential",
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::FogVolume()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

FogVolume::FogVolume() : GameBase(OT_NORMAL)
{
	m_vDims			= LTVector(128.0f, 128.0f, 128.0f);
	m_vColor		= LTVector(1.0f, 1.0f, 1.0f);

	m_fDensity		= 1.0f;
	m_fMinOpacity	= 0.0f;
	m_fMaxOpacity	= 1.0f;

	m_nRes			= 32;
	m_nShape		= 0;
	m_nGradient		= 0;
	m_nDensityFn	= 0;

	m_bFogWorld		= LTTRUE;
	m_bFogModels	= LTTRUE;
	m_bVisible		= LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::~FogVolume()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

FogVolume::~FogVolume()
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::EngineMessageFn
//
//	PURPOSE:	Handle engine EngineMessageFn messages
//
// ----------------------------------------------------------------------- //

uint32 FogVolume::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData)
{
	uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, lData);

	switch (messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;

			if(pStruct)
			{
				// Read the properties
				ReadProp(pStruct);
			}

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			// Set the user flag to determine if it should be visible
			uint32 dwUserFlags = 0;
			dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
			dwUserFlags = m_bVisible ? (dwUserFlags | USRFLG_VISIBLE) : (dwUserFlags & ~USRFLG_VISIBLE);
			g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

			// Setup the special effect message
			CreateSpecialFXMessage();
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)lData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)lData);
			break;
		}

		default: break;
	}

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 FogVolume::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			if(HandleTriggerMsg(hSender, hRead))
			{
				CreateSpecialFXMessage();
				UpdateClients();
			}

			break;
		}
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL FogVolume::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric("Dims", &genProp) == DE_OK)
	{
		m_vDims = genProp.m_Vec;
	}

	if(g_pLTServer->GetPropGeneric("Color", &genProp) == DE_OK)
	{
		m_vColor = genProp.m_Color;
		m_vColor /= 255.0f;
	}

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric("Density", &genProp) == DE_OK)
	{
		m_fDensity = genProp.m_Float;
	}

	if(g_pLTServer->GetPropGeneric("MinOpacity", &genProp) == DE_OK)
	{
		m_fMinOpacity = genProp.m_Float;
	}

	if(g_pLTServer->GetPropGeneric("MaxOpacity", &genProp) == DE_OK)
	{
		m_fMaxOpacity = genProp.m_Float;
	}

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric("MapRes", &genProp) == DE_OK)
	{
		m_nRes = 0;

		if(genProp.m_String)
		{
			for(int i = 0; i < g_nMapRes; i++)
			{
				if(!strcmp(genProp.m_String, g_szMapRes[i]))
				{
					m_nRes = i;
					break;
				}
			}
		}
	}

	if(g_pLTServer->GetPropGeneric("Shape", &genProp) == DE_OK)
	{
		m_nShape = 0;

		if(genProp.m_String)
		{
			for(int i = 0; i < g_nShapes; i++)
			{
				if(!strcmp(genProp.m_String, g_szShapes[i]))
				{
					m_nShape = i;
					break;
				}
			}
		}
	}

	if(g_pLTServer->GetPropGeneric("Gradient", &genProp) == DE_OK)
	{
		m_nGradient = 0;

		if(genProp.m_String)
		{
			for(int i = 0; i < g_nGradients; i++)
			{
				if(!strcmp(genProp.m_String, g_szGradients[i]))
				{
					m_nGradient = i;
					break;
				}
			}
		}
	}

	if(g_pLTServer->GetPropGeneric("DensityFn", &genProp) == DE_OK)
	{
		m_nDensityFn = 0;

		if(genProp.m_String)
		{
			for(int i = 0; i < g_nDensityFns; i++)
			{
				if(!strcmp(genProp.m_String, g_szDensityFns[i]))
				{
					m_nDensityFn = i;
					break;
				}
			}
		}
	}

	// ----------------------------------------------------------------------- //

	if(g_pLTServer->GetPropGeneric("FogWorld", &genProp) == DE_OK)
	{
		m_bFogWorld = genProp.m_Bool;
	}

	if(g_pLTServer->GetPropGeneric("FogModels", &genProp) == DE_OK)
	{
		m_bFogModels = genProp.m_Bool;
	}

	if(g_pLTServer->GetPropGeneric("Visible", &genProp) == DE_OK)
	{
		m_bVisible = genProp.m_Bool;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::HandleTriggerMsg
//
//	PURPOSE:	Handle the message send to this object
//
// ----------------------------------------------------------------------- //

LTBOOL FogVolume::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon || !m_hObject) return LTFALSE;

	HSTRING msg = g_pLTServer->ReadFromMessageHString(hRead);
	LTBOOL bRet = LTFALSE;

	ConParse parse;
	parse.Init(g_pLTServer->GetStringData(msg));

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			switch(parse.m_nArgs)
			{
				// ----------------------------------------------------------------------- //
				// Check all the single parameter tokens
				case 1:
				{

				}
				break;

				// ----------------------------------------------------------------------- //
				// Check all the tokens with 2 parameters
				case 2:
				{
					if(_stricmp(parse.m_Args[0], "VISIBLE") == 0)
					{
						m_bVisible = (LTBOOL)atoi(parse.m_Args[1]);

						uint32 dwUserFlags = 0;
						dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
						dwUserFlags = m_bVisible ? (dwUserFlags | USRFLG_VISIBLE) : (dwUserFlags & ~USRFLG_VISIBLE);
						g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
					}
					else if(_stricmp(parse.m_Args[0], "DENSITY") == 0)
					{
						m_fDensity = (LTFLOAT)atof(parse.m_Args[1]);
						bRet = LTTRUE;
					}
				}
				break;

				// ----------------------------------------------------------------------- //
				// Check all the tokens with 3 parameters
				case 3:
				{
					if(_stricmp(parse.m_Args[0], "OPACITY") == 0)
					{
						m_fMinOpacity = (LTFLOAT)atof(parse.m_Args[1]);
						m_fMaxOpacity = (LTFLOAT)atof(parse.m_Args[2]);
						bRet = LTTRUE;
					}
				}
				break;

				// ----------------------------------------------------------------------- //
				// Check all the tokens with 4 parameters
				case 4:
				{
					if(_stricmp(parse.m_Args[0], "DIMS") == 0)
					{
						m_vDims.x = (LTFLOAT)atof(parse.m_Args[1]);
						m_vDims.y = (LTFLOAT)atof(parse.m_Args[2]);
						m_vDims.z = (LTFLOAT)atof(parse.m_Args[3]);
						bRet = LTTRUE;
					}
					else if(_stricmp(parse.m_Args[0], "COLOR") == 0)
					{
						m_vColor.x = (LTFLOAT)atof(parse.m_Args[1]);
						m_vColor.y = (LTFLOAT)atof(parse.m_Args[2]);
						m_vColor.z = (LTFLOAT)atof(parse.m_Args[3]);

						if(m_vColor.x > 1.0f || m_vColor.y > 1.0f || m_vColor.z > 1.0f)
							m_vColor /= 255.0f;

						bRet = LTTRUE;
					}
				}
				break;

				default:
				break;
			}
		}
	}

	// We're done... so return the bool value that says whether we need
	// to recreate the sfx message
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::CreateSpecialFXMessage
//
//	PURPOSE:	Creates a special effect message to send to all clients
//				when this object gets into the vis area
//
// ----------------------------------------------------------------------- //

void FogVolume::CreateSpecialFXMessage()
{
	// Send the inital SFX message
	HMESSAGEWRITE hWrite = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hWrite, SFX_FOG_VOLUME_ID);

	g_pLTServer->WriteToMessageVector(hWrite, &m_vDims);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vColor);

	g_pLTServer->WriteToMessageFloat(hWrite, m_fDensity);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMinOpacity);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxOpacity);

	g_pLTServer->WriteToMessageByte(hWrite, m_nRes);
	g_pLTServer->WriteToMessageByte(hWrite, m_nShape);
	g_pLTServer->WriteToMessageByte(hWrite, m_nGradient);
	g_pLTServer->WriteToMessageByte(hWrite, m_nDensityFn);

	g_pLTServer->WriteToMessageByte(hWrite, m_bFogWorld);
	g_pLTServer->WriteToMessageByte(hWrite, m_bFogModels);

	g_pLTServer->EndMessage(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::UpdateClients()
//
//	PURPOSE:	Updates the fog on the client who know about it
//
// ----------------------------------------------------------------------- //

void FogVolume::UpdateClients()
{
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hWrite, SFX_FOG_VOLUME_ID);
	g_pLTServer->WriteToMessageObject(hWrite, m_hObject);

	g_pLTServer->WriteToMessageVector(hWrite, &m_vDims);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vColor);

	g_pLTServer->WriteToMessageFloat(hWrite, m_fDensity);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMinOpacity);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxOpacity);

	g_pLTServer->WriteToMessageByte(hWrite, m_nRes);
	g_pLTServer->WriteToMessageByte(hWrite, m_nShape);
	g_pLTServer->WriteToMessageByte(hWrite, m_nGradient);
	g_pLTServer->WriteToMessageByte(hWrite, m_nDensityFn);

	g_pLTServer->WriteToMessageByte(hWrite, m_bFogWorld);
	g_pLTServer->WriteToMessageByte(hWrite, m_bFogModels);

	g_pLTServer->EndMessage(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void FogVolume::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	g_pLTServer->WriteToMessageVector(hWrite, &m_vDims);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vColor);

	g_pLTServer->WriteToMessageFloat(hWrite, m_fDensity);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMinOpacity);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxOpacity);

	g_pLTServer->WriteToMessageByte(hWrite, m_nRes);
	g_pLTServer->WriteToMessageByte(hWrite, m_nShape);
	g_pLTServer->WriteToMessageByte(hWrite, m_nGradient);
	g_pLTServer->WriteToMessageByte(hWrite, m_nDensityFn);

	g_pLTServer->WriteToMessageByte(hWrite, m_bFogWorld);
	g_pLTServer->WriteToMessageByte(hWrite, m_bFogModels);
	g_pLTServer->WriteToMessageByte(hWrite, m_bVisible);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FogVolume::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void FogVolume::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	g_pLTServer->ReadFromMessageVector(hRead, &m_vDims);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vColor);

	m_fDensity		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fMinOpacity	= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fMaxOpacity	= g_pLTServer->ReadFromMessageFloat(hRead);

	m_nRes			= g_pLTServer->ReadFromMessageByte(hRead);
	m_nShape		= g_pLTServer->ReadFromMessageByte(hRead);
	m_nGradient		= g_pLTServer->ReadFromMessageByte(hRead);
	m_nDensityFn	= g_pLTServer->ReadFromMessageByte(hRead);

	m_bFogWorld		= g_pLTServer->ReadFromMessageByte(hRead);
	m_bFogModels	= g_pLTServer->ReadFromMessageByte(hRead);
	m_bVisible		= g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFogVolumePlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CFogVolumePlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...
	if(_stricmp("MapRes", szPropName) == 0)
	{
		for(int i = 0; i < g_nMapRes; i++)
		{
			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], g_szMapRes[i]);
			}
		}

		return LT_OK;
	}

	// See if we can handle the property...
	if(_stricmp("Shape", szPropName) == 0)
	{
		for(int i = 0; i < g_nShapes; i++)
		{
			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], g_szShapes[i]);
			}
		}

		return LT_OK;
	}

	// See if we can handle the property...
	if(_stricmp("Gradient", szPropName) == 0)
	{
		for(int i = 0; i < g_nGradients; i++)
		{
			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], g_szGradients[i]);
			}
		}

		return LT_OK;
	}

	// See if we can handle the property...
	if(_stricmp("DensityFn", szPropName) == 0)
	{
		for(int i = 0; i < g_nDensityFns; i++)
		{
			if(cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], g_szDensityFns[i]);
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
