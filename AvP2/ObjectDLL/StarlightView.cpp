// ----------------------------------------------------------------------- //
//
// MODULE  : StarLightView.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 07/18/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "cpp_server_de.h"
#include "StarlightView.h"
#include "SFXMsgIds.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "light_anim_lt.h"
#include "lt_pre_light_anim.h"
#include "MsgIDs.h"
#include "AIUtils.h"

#include <string>

#define UPDATE_DELTA			0.1f

// -----------------------------------------------------------------------
// StarLightView light animations are built off the name + an extension.
// -----------------------------------------------------------------------
static const std::string LIGHTFX_STARLIGHTVIEW_EXTENSION("__SV");

// Register the classes so they can be read by DEdit.
BEGIN_CLASS(StarLightView)

	ADD_BOOLPROP(StartOn, DTRUE)
	ADD_REALPROP(LifeTime, 0.0f)
	ADD_STRINGPROP(ActiveMode, "")
	ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)

	ADD_BOOLPROP(CastShadowsFlag, DFALSE)
	ADD_BOOLPROP(SolidLightFlag, DFALSE)
	ADD_BOOLPROP(OnlyLightWorldFlag, DFALSE)
	ADD_BOOLPROP(DontLightBackfacingFlag, DFALSE)

	ADD_COLORPROP(FogColor, 0.0f, 0.0f, 0.0f)
	ADD_LONGINTPROP(MinFogNearZ, 0)
	ADD_LONGINTPROP(MinFogFarZ, 5000)
	ADD_LONGINTPROP(FogNearZ, 0)
	ADD_LONGINTPROP(FogFarZ, 5000)

	ADD_COLORPROP(VertexTintColor, 0.0f, 0.0f, 0.0f)

	ADD_COLORPROP(ModelAddColor, 0.0f, 0.0f, 0.0f)

	ADD_COLORPROP(AmbientColor, 0.0f, 0.0f, 0.0f)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS_PLUGIN(StarLightView, BaseClass, NULL, NULL, 0, StarLightViewPlugin)






// ----------------------------------------------------------------------- //
// Preprocessor callback to build a light animation frame.
// ----------------------------------------------------------------------- //
// PreLightLT is a preprocessor lighting interface
// HPREOBJECT is a handle used to identify the object being worked on
DRESULT	StarLightViewPlugin::PreHook_Light(
	PreLightLT *pInterface, 
	HPREOBJECT hObject)
{
	// Get name
	GenericProp genProp;
	pInterface->GetPropGeneric(hObject, "Name", &genProp);
	std::string objectName(genProp.m_String);

	PreLightInfo lightInfo;

	// Make animation Name
	std::string animName = objectName + LIGHTFX_STARLIGHTVIEW_EXTENSION;

	// Get position
	pInterface->GetPropGeneric(hObject, "Pos", &genProp);
	lightInfo.m_vPos = genProp.m_Vec;

	// Get Color
	pInterface->GetPropGeneric(hObject, "Color", &genProp);
	lightInfo.m_vInnerColor.Init();
	lightInfo.m_vOuterColor.Init();
	lightInfo.m_Radius = 0;

	// Get rotation
	DVector vRight, vUp;
	pInterface->GetPropGeneric(hObject, "Rotation", &genProp);
	pInterface->GetMathLT()->GetRotationVectors(genProp.m_Rotation, vRight, vUp, lightInfo.m_vDirection);

	// Make the Frame object
	PreLightAnimFrameInfo frame;		// This should have a constructor that takes these arguments !!!JKE
	frame.m_bSunLight = DFALSE;			// No Sunlight
	pInterface->GetPropGeneric(hObject, "Color", &genProp);
	frame.m_vSunLightInnerColor = frame.m_vSunLightOuterColor = genProp.m_Vec;

	// Get ambient color
	pInterface->GetPropGeneric(hObject, "AmbientColor", &genProp);
	frame.m_vAmbientColor = genProp.m_Vec;

	frame.m_Lights = &lightInfo;		// Give frame the address of the light info
	frame.m_nLights = 1;				// How many lights did we give above.

	// We have all the information we need, so Create the light maps.
	pInterface->CreateLightAnim(animName.c_str(), &frame, 1, DFALSE);
	return LT_OK;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

StarLightView::StarLightView() : BaseClass(OT_NORMAL /*OT_LIGHT*/), 
	m_bOn(DTRUE), m_vColor(255.0f, 255.0f, 255.0f),
	m_dwLightFlags(0), m_bDynamic(DTRUE), m_fLifeTime(-1.0f), m_fStartTime(0.0f),
	m_vFogColor(0.0f, 0.0f, 0.0f), m_MinFogNearZ(0), m_MinFogFarZ(5000), m_FogNearZ(0), m_FogFarZ(5000), 
	m_bFirstUpdate(true), 
//	m_vLightTintColor(0.0f, 0.0f, 0.0f),	
	m_vVertexTintColor(0.0f, 0.0f, 0.0f),
	m_vModelAddColor(0.0f, 0.0f, 0.0f),
	m_vAmbientColor(0.0f, 0.0f, 0.0f)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightView::~StarLightView()	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

StarLightView::~StarLightView()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD StarLightView::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				m_bDynamic = DFALSE;
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
			}
			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{

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
//	ROUTINE:	StarLightView::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
DDWORD StarLightView::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	StarLightView::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
void StarLightView::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData( hMsg );

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	DDWORD dwFlags    = g_pServerDE->GetObjectFlags(m_hObject);

    if ( _stricmp(pszMessage, "TOGGLE") == 0)
    {
		// Toggle the flag
		if (dwUsrFlags & USRFLG_VISIBLE)
		{
			dwUsrFlags &= ~USRFLG_VISIBLE;
		}
		else
		{
			dwUsrFlags |= USRFLG_VISIBLE;
		}
    } 
    else if ( _stricmp(pszMessage, "ON") == 0)
    {
		dwUsrFlags |= USRFLG_VISIBLE;
    }            
    else if ( _stricmp(pszMessage, "OFF") == 0)
    {
		dwUsrFlags &= ~USRFLG_VISIBLE;
   }        
    
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	g_pServerDE->FreeString( hMsg );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightView::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
DBOOL StarLightView::ReadProp(ObjectCreateStruct * pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;


	LTBOOL bVal;
	LTBOOL bUseFogPerformance = LTFALSE;

	if (g_pServerDE->GetPropBool("UseFogPerformance", &bVal) == DE_OK)
	{
		bUseFogPerformance = bVal;
	}


	// Save off our given name and re-name us more generically
	m_ObjectName = pStruct->m_Name;
	SAFE_STRCPY(pStruct->m_Name, "StarlightObject");

	if (pServerDE->GetPropGeneric("StartOn", &genProp) == DE_OK)
		m_bOn = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("LifeTime", &genProp) == DE_OK)
		m_fLifeTime = genProp.m_Float;

    if (m_fLifeTime < 0.0f) m_fLifeTime = 0.0f;

	if (pServerDE->GetPropGeneric("ActiveMode", &genProp) == DE_OK)
		m_ActiveMode = std::string(genProp.m_String);

	if (pServerDE->GetPropGeneric("Color", &genProp) == DE_OK)
		VEC_COPY(m_vColor, genProp.m_Color);

	if (pServerDE->GetPropGeneric("FogColor", &genProp) == DE_OK)
		VEC_COPY(m_vFogColor, genProp.m_Color);

	if (pServerDE->GetPropGeneric("MinFogNearZ", &genProp) == DE_OK)
		m_MinFogNearZ = genProp.m_Long;

	if (pServerDE->GetPropGeneric("MinFogFarZ", &genProp) == DE_OK)
		m_MinFogFarZ = genProp.m_Long;

	if (pServerDE->GetPropGeneric("FogNearZ", &genProp) == DE_OK)
		m_FogNearZ = genProp.m_Long;

	if (pServerDE->GetPropGeneric("FogFarZ", &genProp) == DE_OK)
		m_FogFarZ = genProp.m_Long;


	if(!bUseFogPerformance)
	{
		m_MinFogFarZ = m_FogFarZ;
		m_MinFogNearZ = m_FogNearZ;
	}



	if (pServerDE->GetPropGeneric("VertexTintColor", &genProp) == DE_OK)
	{
		VEC_COPY(m_vVertexTintColor, genProp.m_Color);
		m_vVertexTintColor /= 255.0f;
	}

	if (pServerDE->GetPropGeneric("ModelAddColor", &genProp) == DE_OK)
	{
		VEC_COPY(m_vModelAddColor, genProp.m_Color);
	}

	if (pServerDE->GetPropGeneric("AmbientColor", &genProp) == DE_OK)
	{
		VEC_COPY(m_vAmbientColor, genProp.m_Color);
	}

	if (pServerDE->GetPropGeneric("CastShadowsFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_CASTSHADOWS : 0;
	}

	if (pServerDE->GetPropGeneric("SolidLightFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_SOLIDLIGHT : 0;
	}

	if (pServerDE->GetPropGeneric("OnlyLightWorldFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_ONLYLIGHTWORLD : 0;
	}

	if (pServerDE->GetPropGeneric("DontLightBackfacingFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_DONTLIGHTBACKFACING : 0;
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

void StarLightView::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL StarLightView::InitialUpdate(DVector *pMovement)
{
	g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
	g_pLTServer->SetDeactivationTime(m_hObject, 0.001f);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);


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

DBOOL StarLightView::Init()
{
	m_fStartTime = g_pLTServer->GetTime();

	if (m_bOn)
	{
		DDWORD dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		dwUsrFlags |= USRFLG_VISIBLE;
		g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}

	return DTRUE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightView::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
DBOOL StarLightView::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	DBOOL bRemove = DFALSE;
	if (m_fLifeTime > 0 && (pServerDE->GetTime() - m_fStartTime) >= m_fLifeTime)
	{
		bRemove = DTRUE;
	}

	return (!bRemove);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightView::SendEffectMessage
//
//	PURPOSE:	Sends a message to the client to start a light effect
//
// ----------------------------------------------------------------------- //

void StarLightView::SendEffectMessage(HCLIENT hClient)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartMessage(hClient, MID_STARLIGHT);

	// Make an HString
	HSTRING hStr = g_pServerDE->CreateString(const_cast<char*>(m_ActiveMode.c_str()));

	// Send it to the client
	hMessage->WriteHString(hStr);

	// We are done with it.
	g_pServerDE->FreeString(hStr);

	hMessage->WriteVector(m_vColor);
	hMessage->WriteVector(m_vFogColor);
	hMessage->WriteDWord(m_MinFogNearZ);
	hMessage->WriteDWord(m_MinFogFarZ);
	hMessage->WriteDWord(m_FogNearZ);
	hMessage->WriteDWord(m_FogFarZ);
	hMessage->WriteDWord(m_dwLightFlags);

	// Write the animation handle.	
	std::string lightAnimName = m_ObjectName + LIGHTFX_STARLIGHTVIEW_EXTENSION;

	HLIGHTANIM hLightAnim = INVALID_LIGHT_ANIM;
	pServerDE->GetLightAnimLT()->FindLightAnim(lightAnimName.c_str(), hLightAnim);

	hMessage->WriteDWord((DDWORD)hLightAnim);

	hMessage->WriteVector(m_vVertexTintColor);

	hMessage->WriteVector(m_vModelAddColor);
	hMessage->WriteVector(m_vAmbientColor);

	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightView::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StarLightView::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	*hWrite << m_bOn;
	*hWrite << m_bDynamic;
	*hWrite << m_fLifeTime;
	hWrite->WriteFloat(m_fStartTime - g_pLTServer->GetTime());
	*hWrite << m_ActiveMode;
	*hWrite << m_ObjectName;
	*hWrite << m_vColor;
	*hWrite << m_dwLightFlags;
	*hWrite << m_vFogColor;	
	*hWrite << m_MinFogNearZ;
	*hWrite << m_MinFogFarZ;
	*hWrite << m_FogNearZ;
	*hWrite << m_FogFarZ;
	*hWrite << m_vVertexTintColor;
	*hWrite << m_vModelAddColor;
	*hWrite << m_vAmbientColor;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightView::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void StarLightView::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	*hRead >> m_bOn;
	*hRead >> m_bDynamic;
	*hRead >> m_fLifeTime;
	m_fStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_ActiveMode;
	*hRead >> m_ObjectName;
	*hRead >> m_vColor;
	*hRead >> m_dwLightFlags;
	*hRead >> m_vFogColor;
	*hRead >> m_MinFogNearZ;
	*hRead >> m_MinFogFarZ;
	*hRead >> m_FogNearZ;
	*hRead >> m_FogFarZ;
	*hRead >> m_vVertexTintColor;
	*hRead >> m_vModelAddColor;
	*hRead >> m_vAmbientColor;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightView::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void StarLightView::CacheFiles()
{
}
