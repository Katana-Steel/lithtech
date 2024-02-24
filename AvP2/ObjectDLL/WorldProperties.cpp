// ----------------------------------------------------------------------- //
//
// MODULE  : WorldProperties.cpp
//
// PURPOSE : WorldProperties object - Implementation
//
// CREATED : 9/25/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "WorldProperties.h"
#include "cpp_engineobjects_de.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include <stdio.h>
#include "MusicMgr.h"

#ifdef _WIN32
	#define CHROME_TEXTURE	"WorldTextures\\Chrome.dtx"
	#define SKYPAN_TEXTURE	"WorldTextures\\SkyPan.dtx"
#else
	#define CHROME_TEXTURE	"WorldTextures/Chrome.dtx"
	#define SKYPAN_TEXTURE	"WorldTextures/SkyPan.dtx"
#endif

WorldProperties* g_pWorldProperties = DNULL;

BEGIN_CLASS(WorldProperties)

	ADD_REALPROP(MinFarZ, 50000.0f)
	ADD_REALPROP(FarZ, 50000.0f)
	ADD_BOOLPROP(AllSkyPortals, DFALSE)
	ADD_VECTORPROP_VAL(Wind, 0.0f, 0.0f, 0.0f)
	ADD_STRINGPROP(EnvironmentMap, CHROME_TEXTURE)
	ADD_COLORPROP(VoidColor, 0.0f, 0.0f, 0.0f)

	ADD_BOOLPROP(UseFogPerformance, LTFALSE)

	PROP_DEFINEGROUP(FogInfo, PF_GROUP1)
		ADD_BOOLPROP_FLAG(FogEnable, DTRUE, PF_GROUP1)
		ADD_COLORPROP_FLAG(FogColor, 127.0f, 127.0f, 127.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(MinFogNearZ, 1.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(MinFogFarZ, 5000.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(FogNearZ, 1.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(FogFarZ, 5000.0f, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SkyFogEnable, DFALSE, PF_GROUP1)
		ADD_REALPROP_FLAG(MinSkyFogNearZ, 100.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(MinSkyFogFarZ, 1000.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SkyFogNearZ, 100.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SkyFogFarZ, 1000.0f, PF_GROUP1)

	PROP_DEFINEGROUP(VFogInfo, PF_GROUP2)
		ADD_BOOLPROP_FLAG(VFog, 0.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMinY, 0.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMaxY, 1300.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMinYVal, 0.5f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMaxYVal, 0.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogDensity, 1800.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMax, 120.0f, PF_GROUP2)

	PROP_DEFINEGROUP(SkyPanning, PF_GROUP3)
		ADD_STRINGPROP_FLAG(PanSkyTexture, SKYPAN_TEXTURE, PF_GROUP3 | PF_FILENAME)
		ADD_BOOLPROP_FLAG(PanSky, 0, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyOffsetX, 10.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyOffsetZ, 10.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyScaleX, 10.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyScaleZ, 10.0f, PF_GROUP3)

	ADD_STRINGPROP_FLAG_HELP(Song, "", PF_STATICLIST, "Song to use for the level.")

    ADD_BOOLPROP(LMAnimStatic, LTTRUE)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS_PLUGIN(WorldProperties, BaseClass, NULL, NULL, 0, CWorldPropertiesPlugin)

static char szMinFarZ[] = "MinFarZ";
static char szFarZ[] = "FarZ";
static char szAllSkyPortals[] = "AllSkyPortals";
static char szWindX[] = "WindX";
static char szWindY[] = "WindY";
static char szWindZ[] = "WindZ";
static char szEnvironmentMap[] = "EnvironmentMap";
static char szVoidColor[] = "VoidColor";
static char szVoidR[] = "VoidR";
static char szVoidG[] = "VoidG";
static char szVoidB[] = "VoidB";
static char szFogEnable[] = "FogEnable";
static char szFogColor[] = "FogColor";
static char szFogR[] = "FogR";
static char szFogG[] = "FogG";
static char szFogB[] = "FogB";
static char szMinFogNearZ[] = "MinFogNearZ";
static char szMinFogFarZ[] = "MinFogFarZ";
static char szFogNearZ[] = "FogNearZ";
static char szFogFarZ[] = "FogFarZ";
static char szSkyFogEnable[] = "SkyFogEnable";
static char szMinSkyFogNearZ[] = "MinSkyFogNearZ";
static char szMinSkyFogFarZ[] = "MinSkyFogFarZ";
static char szSkyFogNearZ[] = "SkyFogNearZ";
static char szSkyFogFarZ[] = "SkyFogFarZ";
static char szPanSky[] = "PanSky";
static char szPanSkyTexture[] = "PanSkyTexture";
static char szPanSkyOffsetX[] = "PanSkyOffsetX";
static char szPanSkyOffsetZ[] = "PanSkyOffsetZ";
static char szPanSkyScaleX[] = "PanSkyScaleX";
static char szPanSkyScaleZ[] = "PanSkyScaleZ";
static char szSong[] = "Song";
static char szLMAnimStatic[] = "LMAnimStatic";

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::WorldProperties()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

WorldProperties::WorldProperties()
{
#ifndef _FINAL
	// There should be only one world properties object per level...
	_ASSERT(!g_pWorldProperties);
	if (g_pWorldProperties)
	{
		g_pServerDE->CPrint("WorldProperties ERROR!  More than one WorldProperties object detected!");
	}
#endif

	g_pWorldProperties = this;
	m_fWorldTimeSpeed = 600.0f;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::~WorldProperties()
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

WorldProperties::~WorldProperties( )
{
	g_pWorldProperties = DNULL;

// BEP 7/27/01
// Setting the environment variables here was causing issues when the
// player aborted a single player game.  The global variables, such as g_FarZ,
// were getting updated in the engine, but the information displayed in the 
// console was not.  The is because changing variables on the server updates
// the shared global variable, but sends a message to the client so it can
// update its display information.  Since the server is destroyed, the
// messages don't make it to the client.
/*
	g_pServerDE->SetGameConVar( szFarZ, "" );
	g_pServerDE->SetGameConVar( szAllSkyPortals, "" );
	g_pServerDE->SetGameConVar( szWindX, "" );
	g_pServerDE->SetGameConVar( szWindY, "" );
	g_pServerDE->SetGameConVar( szWindZ, "" );
	g_pServerDE->SetGameConVar( szEnvironmentMap, "" );
	g_pServerDE->SetGameConVar( szVoidR, "" );
	g_pServerDE->SetGameConVar( szVoidG, "" );
	g_pServerDE->SetGameConVar( szVoidB, "" );
	g_pServerDE->SetGameConVar( szFogEnable, "" );
	g_pServerDE->SetGameConVar( szFogR, "" );
	g_pServerDE->SetGameConVar( szFogG, "" );
	g_pServerDE->SetGameConVar( szFogB, "" );
	g_pServerDE->SetGameConVar( szFogNearZ, "" );
	g_pServerDE->SetGameConVar( szFogFarZ, "" );
	g_pServerDE->SetGameConVar( szSkyFogEnable, "" );
	g_pServerDE->SetGameConVar( szSkyFogNearZ, "" );
	g_pServerDE->SetGameConVar( szSkyFogFarZ, "" );
	g_pServerDE->SetGameConVar( szPanSky, "" );
	g_pServerDE->SetGameConVar( szPanSkyTexture, "" );
	g_pServerDE->SetGameConVar( szPanSkyOffsetX, "" );
	g_pServerDE->SetGameConVar( szPanSkyOffsetZ, "" );
	g_pServerDE->SetGameConVar( szPanSkyScaleX, "" );
	g_pServerDE->SetGameConVar( szPanSkyScaleZ, "" );
    g_pLTServer->SetGameConVar( szSong, "" );
    g_pServerDE->SetGameConVar( szLMAnimStatic, "" );
*/
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// --------------------------------------------------------------------------- //

DDWORD WorldProperties::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ReadProps();
		}
		break;

		case MID_INITIALUPDATE:
		{
			g_pServerDE->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
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

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::ReadProps()
//
//	PURPOSE:	Read in properties
//
// --------------------------------------------------------------------------- //

void WorldProperties::ReadProps()
{
	char buf[512];
	GenericProp genProp;
	DBOOL bVal;
	DFLOAT fVal;

	LTBOOL bUseFogPerformance = LTFALSE;

	if (g_pServerDE->GetPropBool("UseFogPerformance", &bVal) == DE_OK)
	{
		bUseFogPerformance = bVal;
	}


	if (bUseFogPerformance && (g_pServerDE->GetPropReal(szMinFarZ, &fVal) == DE_OK))
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szMinFarZ, buf);
	}

	if (g_pServerDE->GetPropReal(szFarZ, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szFarZ, buf);

		if(!bUseFogPerformance)
			g_pServerDE->SetGameConVar(szMinFarZ, buf);
	}

	if (g_pServerDE->GetPropBool(szAllSkyPortals, &bVal) == DE_OK)
	{
		sprintf(buf, "%d", bVal);
		g_pServerDE->SetGameConVar(szAllSkyPortals, buf);
	}

	if (g_pServerDE->GetPropGeneric("Wind", &genProp) == LT_OK)
	{
		sprintf(buf, "%f", genProp.m_Vec.x);
		g_pServerDE->SetGameConVar(szWindX, buf);

		sprintf(buf, "%f", genProp.m_Vec.y);
		g_pServerDE->SetGameConVar(szWindY, buf);

		sprintf(buf, "%f", genProp.m_Vec.z);
		g_pServerDE->SetGameConVar(szWindZ, buf);
	}

	if (g_pServerDE->GetPropGeneric(szVoidColor, &genProp) == LT_OK)
	{
		sprintf(buf, "%f", genProp.m_Vec.x);
		g_pServerDE->SetGameConVar(szVoidR, buf);

		sprintf(buf, "%f", genProp.m_Vec.y);
		g_pServerDE->SetGameConVar(szVoidG, buf);

		sprintf(buf, "%f", genProp.m_Vec.z);
		g_pServerDE->SetGameConVar(szVoidB, buf);
	}

	if (g_pServerDE->GetPropBool(szFogEnable, &bVal) == DE_OK)
	{
		sprintf(buf, "%d", bVal);
		g_pServerDE->SetGameConVar(szFogEnable, buf);
	}

	if (bUseFogPerformance && (g_pServerDE->GetPropReal(szMinFogNearZ, &fVal) == DE_OK))
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szMinFogNearZ, buf);
	}

	if (bUseFogPerformance && (g_pServerDE->GetPropReal(szMinFogFarZ, &fVal) == DE_OK))
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szMinFogFarZ, buf);
	}

	if (g_pServerDE->GetPropReal(szFogNearZ, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szFogNearZ, buf);

		if(!bUseFogPerformance)
			g_pServerDE->SetGameConVar(szMinFogNearZ, buf);
	}

	if (g_pServerDE->GetPropReal(szFogFarZ, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szFogFarZ, buf);

		if(!bUseFogPerformance)
			g_pServerDE->SetGameConVar(szMinFogFarZ, buf);
	}

	if (g_pServerDE->GetPropGeneric(szFogColor, &genProp) == LT_OK)
	{
		sprintf(buf, "%f", genProp.m_Vec.x);
		g_pServerDE->SetGameConVar(szFogR, buf);

		sprintf(buf, "%f", genProp.m_Vec.y);
		g_pServerDE->SetGameConVar(szFogG, buf);

		sprintf(buf, "%f", genProp.m_Vec.z);
		g_pServerDE->SetGameConVar(szFogB, buf);
	}

	if (g_pServerDE->GetPropBool(szSkyFogEnable, &bVal) == DE_OK)
	{
		sprintf(buf, "%d", bVal);
		g_pServerDE->SetGameConVar(szSkyFogEnable, buf);
	}

	if (bUseFogPerformance && (g_pServerDE->GetPropReal(szMinSkyFogNearZ, &fVal) == DE_OK))
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szMinSkyFogNearZ, buf);
	}

	if (bUseFogPerformance && (g_pServerDE->GetPropReal(szMinSkyFogFarZ, &fVal) == DE_OK))
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szMinSkyFogFarZ, buf);
	}

	if (g_pServerDE->GetPropReal(szSkyFogNearZ, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szSkyFogNearZ, buf);

		if(!bUseFogPerformance)
			g_pServerDE->SetGameConVar(szMinSkyFogNearZ, buf);
	}

	if (g_pServerDE->GetPropReal(szSkyFogFarZ, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szSkyFogFarZ, buf);

		if(!bUseFogPerformance)
			g_pServerDE->SetGameConVar(szMinSkyFogFarZ, buf);
	}

	if (g_pServerDE->GetPropBool(szPanSky, &bVal) == DE_OK)
	{
		sprintf(buf, "%d", bVal);
		g_pServerDE->SetGameConVar(szPanSky, buf);
	}

	buf[0] = '\0';
	if (g_pServerDE->GetPropString(szEnvironmentMap, buf, 500) == DE_OK)
	{
		if (buf[0]) g_pServerDE->SetGameConVar(szEnvironmentMap, buf);
	}

	buf[0] = '\0';
	if (g_pServerDE->GetPropString(szPanSkyTexture, buf, 500) == DE_OK)
	{
		if (buf[0]) g_pServerDE->SetGameConVar(szPanSkyTexture, buf);
	}

	if (g_pServerDE->GetPropReal(szPanSkyOffsetX, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szPanSkyOffsetX, buf);
	}

	if (g_pServerDE->GetPropReal(szPanSkyOffsetZ, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szPanSkyOffsetZ, buf);
	}

	if (g_pServerDE->GetPropReal(szPanSkyScaleX, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szPanSkyScaleX, buf);
	}

	if (g_pServerDE->GetPropReal(szPanSkyScaleZ, &fVal) == DE_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pServerDE->SetGameConVar(szPanSkyScaleZ, buf);
	}

	// VFog stuff.
	if (g_pServerDE->GetPropGeneric("VFog", &genProp) == LT_OK)
	{
		g_pServerDE->SetGameConVar("SC_VFog", genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("VFogMinY", &genProp) == LT_OK)
	{
		g_pServerDE->SetGameConVar("SC_VFogMinY", genProp.m_String);
	}
	
	if (g_pServerDE->GetPropGeneric("VFogMaxYVal", &genProp) == LT_OK)
	{
		g_pServerDE->SetGameConVar("SC_VFogMaxYVal", genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("VFogMinYVal", &genProp) == LT_OK)
	{
		g_pServerDE->SetGameConVar("SC_VFogMinYVal", genProp.m_String);
	}
	
	if (g_pServerDE->GetPropGeneric("VFogMaxY", &genProp) == LT_OK)
	{
		g_pServerDE->SetGameConVar("SC_VFogMaxY", genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("VFogDensity", &genProp) == LT_OK)
	{
		g_pServerDE->SetGameConVar("SC_VFogDensity", genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("VFogMax", &genProp) == LT_OK)
	{
		g_pServerDE->SetGameConVar("SC_VFogMax", genProp.m_String);
	}

	// Bias toward semi-static lightmap animations
    if (g_pServerDE->GetPropBool(szLMAnimStatic, &bVal) == LT_OK)
	{
		sprintf(buf, "%d", bVal);
        g_pServerDE->SetGameConVar(szLMAnimStatic, buf);
	}
	else
		// Default to static lightmap animations because of the lightgroups
		g_pServerDE->SetGameConVar(szLMAnimStatic, "1");

	// Get the music properties.
    if( g_pLTServer->GetPropGeneric( szSong, &genProp ) == LT_OK && 
		genProp.m_String[0] )
	{
        g_pLTServer->SetGameConVar( szSong, genProp.m_String );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD WorldProperties::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			HandleMsg(hSender, hMsg);
			pServerDE->FreeString(hMsg);
		}
		break;

		default : break;
	}
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void WorldProperties::HandleMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	CommonLT* pCommon = g_pServerDE->Common();
	if (!pCommon) return;

	ConParse parse;
	parse.Init(pMsg);

	DBOOL bChangedFog = DFALSE;

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "ON") == 0)
			{
				char buf[20];
				sprintf(buf,"%.2f", m_fWorldTimeSpeed);

				g_pServerDE->SetGameConVar("WorldTimeSpeed", buf);
			}
			else if (_stricmp(parse.m_Args[0], "OFF") == 0)
			{
				g_pServerDE->SetGameConVar("WorldTimeSpeed", "-1.0");
			}
			else if (_stricmp(parse.m_Args[0], "WorldTimeSpeed") == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					m_fWorldTimeSpeed = (DFLOAT) atof(parse.m_Args[1]);
					g_pServerDE->SetGameConVar("WorldTimeSpeed", parse.m_Args[1]);
				}
			}
			else if (_stricmp(parse.m_Args[0], "WorldTime") == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar("WorldTime", parse.m_Args[1]);
				}
			}
			else if (_stricmp(parse.m_Args[0], szVoidColor) == 0)
			{
				if (parse.m_nArgs > 3 && parse.m_Args[1] && parse.m_Args[2] && parse.m_Args[3])
				{
					g_pServerDE->SetGameConVar(szVoidR, parse.m_Args[1]);
					g_pServerDE->SetGameConVar(szVoidG, parse.m_Args[2]);
					g_pServerDE->SetGameConVar(szVoidB, parse.m_Args[3]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogColor) == 0)
			{
				if (parse.m_nArgs > 3 && parse.m_Args[1] && parse.m_Args[2] && parse.m_Args[3])
				{
					g_pServerDE->SetGameConVar(szFogR, parse.m_Args[1]);
					g_pServerDE->SetGameConVar(szFogG, parse.m_Args[2]);
					g_pServerDE->SetGameConVar(szFogB, parse.m_Args[3]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogEnable) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szFogEnable, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szMinFogNearZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szMinFogNearZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szMinFogFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szMinFogFarZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogNearZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szFogNearZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szFogFarZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szSkyFogEnable) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szSkyFogEnable, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szMinSkyFogFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szMinSkyFogFarZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szMinSkyFogNearZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szMinSkyFogNearZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szSkyFogFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szSkyFogFarZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szSkyFogNearZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szSkyFogNearZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szMinFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szMinFarZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					g_pServerDE->SetGameConVar(szFarZ, parse.m_Args[1]);
					bChangedFog = DTRUE;
				}
			}
		}
	}

	if (bChangedFog)
	{
		SendClientsFogValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::SendClientsFogValues
//
//	PURPOSE:	Send the fog values to the client(s)
//
// ----------------------------------------------------------------------- //

void WorldProperties::SendClientsFogValues()
{
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_CHANGE_FOG);
	g_pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	// First save global game server shell stuff...(this only works because the
	// GameServerShell saves the WorldProperties object first)

	g_pGameServerShell->Save(hWrite, dwSaveFlags);


	g_pServerDE->WriteToMessageFloat(hWrite, m_fWorldTimeSpeed);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	// First load global game server shell stuff...(this only works because the
	// GameServerShell saves the WorldProperties object first)

	g_pGameServerShell->Load(hRead, dwLoadFlags);


	m_fWorldTimeSpeed = g_pServerDE->ReadFromMessageFloat(hRead);
}

static bool		s_bInitialized = false;

DRESULT	CWorldPropertiesPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength)
{
	char szFile[256];

	// Make sure we have the butes file loaded.
	if( !s_bInitialized )
	{
		// Initialize the music mgr.
		if( g_pMusicButeMgr && !g_pMusicButeMgr->IsValid( ))
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, "attributes\\music.txt" );
#else
			sprintf(szFile, "%s/%s", szRezPath, "Attributes/music.txt" );
#endif

			g_pMusicButeMgr->SetInRezFile(LTFALSE);
			g_pMusicButeMgr->Init(g_pLTServer, szFile);
		}

		s_bInitialized = true;
	}

	// Handle the music properties.
	if( stricmp("Song",szPropName) == 0 )
	{
		// Add a blank entry.
		strcpy( *aszStrings, "" );
		++aszStrings;
		++(*pcStrings);

		if( g_pMusicButeMgr )
		{
			int nSongIndex = 0;
			CString sSongIndexKey;
			CString sSongTag;

			// Iterate through the songs listed in the "Songs" tag.
			while( 1 )
			{
				// Create the songindex key.  They are labeled sequencially as in Song0, Song1, etc.
				sSongIndexKey.Format( "Song%i", nSongIndex );

				// Increment the song index for the next iteration.
				nSongIndex++;

				// Get the song for this index key.
				sSongTag = g_pMusicButeMgr->GetButeMgr( )->GetString( "Songs", sSongIndexKey, "" );

				// If we didn't find the song, then there are no more in the list.
				if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
					break;

				// Add a the song.
				strcpy( *aszStrings, sSongTag );
				++aszStrings;
				++(*pcStrings);
			}	

			return LT_OK;
		}
	}

	// No one wants it

	return LT_UNSUPPORTED;
}
