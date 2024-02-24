// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.cpp
//
// PURPOSE : VolumeBrush implementation
//
// CREATED : 1/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VolumeBrush.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "PolyGrid.h"
#include "Character.h"
#include "SFXMsgIds.h"
#include "ObjectMsgs.h"

#define UPDATE_DELTA					0.01f
#define LIQUID_GRAVITY					-200.0f
#define TRIGGER_MSG_ON					"ON"
#define TRIGGER_MSG_OFF					"OFF"

BEGIN_CLASS(VolumeBrush)
	ADD_VISIBLE_FLAG(0, 0)
	ADD_BOOLPROP(Hidden, LTFALSE)
	ADD_REALPROP(Viscosity, 0.0f)
	ADD_REALPROP(Friction, 1.0f)
	ADD_VECTORPROP_VAL(Current, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(LiquidGravity, LIQUID_GRAVITY)
	ADD_REALPROP(Damage, 0.0f)
	ADD_LONGINTPROP(DamageType, DT_CHOKE)
	ADD_BOOLPROP_FLAG_HELP(DamageCharactersOnly, LTFALSE, 0, "Tells whether to only damage objects of type 'Character'")
	ADD_BOOLPROP_FLAG_HELP(DamagePlayersOnly, LTFALSE, 0, "Tells whether to only damage objects of type 'Player'")
    ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, 0)
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, 0)
	ADD_BOOLPROP_FLAG_HELP(StartOn, LTTRUE, 0, "Tells whether this object should start on or not.")
	PROP_DEFINEGROUP(SurfaceStuff, PF_GROUP1)
 		ADD_BOOLPROP_FLAG(ShowSurface, LTTRUE, PF_GROUP1)
		ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1|PF_FILENAME)
		ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(XScaleDuration, 60.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(YScaleDuration, 60.0f, PF_GROUP1)
		ADD_BOOLPROP_FLAG(PanWithCurrent, LTTRUE, PF_GROUP1)
		ADD_REALPROP_FLAG(XPan, 10.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(YPan, 10.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(Frequency, 50.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1)
		ADD_COLORPROP_FLAG(SurfaceColor1, 255.0f, 255.0f, 255.0f, PF_GROUP1)
		ADD_COLORPROP_FLAG(SurfaceColor2, 255.0f, 255.0f, 255.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1)
	PROP_DEFINEGROUP(FogStuff, PF_GROUP2)
		ADD_BOOLPROP_FLAG(FogEnable, LTFALSE, PF_GROUP2)
		ADD_REALPROP_FLAG(MinFogFarZ, 300.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(MinFogNearZ, -100.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(FogFarZ, 300.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(FogNearZ, -100.0f, PF_GROUP2)
		ADD_COLORPROP_FLAG(FogColor, 0.0f, 0.0f, 0.0f, PF_GROUP2)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(VolumeBrush, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::VolumeBrush()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

VolumeBrush::VolumeBrush() : BaseClass(OT_CONTAINER)
{
	m_nSfxMsgId			= SFX_VOLUMEBRUSH_ID;
	m_dwSaveFlags		= 0;
	m_eContainerCode	= CC_VOLUME;
	m_fDamage			= 0.0f;
	m_eDamageType		= DT_UNSPECIFIED;
	m_bShowSurface		= LTTRUE;
	m_fSurfaceHeight	= 5.0f;
	m_fViscosity		= 0.0f;
	m_fFriction			= 1.0f;
	m_hSurfaceObj		= LTNULL;
	m_bHidden			= LTFALSE;
	m_fLiquidGravity	= LIQUID_GRAVITY;

	VEC_INIT(m_vLastPos);
	VEC_INIT(m_vCurrent);
	VEC_SET(m_vTintColor, 255.0f, 255.0f, 255.0f);
	VEC_INIT(m_vLightAdd);

	m_bFogEnable	= LTFALSE;
	m_fMinFogFarZ	= 300.0f;
	m_fMinFogNearZ	= -100.0f;
	m_fFogFarZ		= 300.0f;
	m_fFogNearZ		= -100.0f;
	VEC_INIT(m_vFogColor);

	// Surface related stuff...

	m_fXScaleMin = 15.0f;
	m_fXScaleMax = 25.0f;
	m_fYScaleMin = 15.0f;
	m_fYScaleMax = 25.0f;
	m_fXScaleDuration = 10.0f;
	m_fYScaleDuration = 10.0f;
	m_bPanWithCurrent = LTTRUE;
	m_fXPan = 10.0f;
	m_fYPan = 10.0f;
	m_fFrequency = 50.0f;
	m_hstrSurfaceSprite = LTNULL;
	m_dwNumSurfPolies = 160;
	m_fSurfAlpha = 0.7f;
	m_bAdditive = LTFALSE;
	m_bMultiply = LTFALSE;

	VEC_SET(m_vSurfaceColor1, 255.0f, 255.0f, 255.0f);
	VEC_SET(m_vSurfaceColor2, 255.0f, 255.0f, 255.0f);

	m_dwFlags = FLAG_CONTAINER | FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;

	m_bStartOn = LTTRUE;
	m_bDamageCharactersOnly = LTFALSE;
	m_bDamagePlayersOnly = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::~VolumeBrush
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

VolumeBrush::~VolumeBrush()
{ 
	if (m_hstrSurfaceSprite)
	{
		g_pLTServer->FreeString(m_hstrSurfaceSprite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 VolumeBrush::EngineMessageFn(uint32 messageID, void *pData, DFLOAT fData)
{
	uint32 dwRet;

	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_AFFECTPHYSICS:
		{
			UpdatePhysics((ContainerPhysics*)pData);
			break;
		}

		case MID_PRECREATE:
		{
			dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 VolumeBrush::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			HandleTrigger(hSender, hMsg);
			g_pLTServer->FreeString(hMsg);
		}
		break;
	}
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::TriggerMsg()
//
//	PURPOSE:	Handler for volume brush trigger messages.
//
// --------------------------------------------------------------------------- //

void VolumeBrush::HandleTrigger(HOBJECT hSender, HSTRING hMsg)
{
	char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg || !pMsg[0]) return;

	if (m_bHidden && (stricmp(pMsg, TRIGGER_MSG_ON) == 0))
	{
		uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_VISIBLE);
		g_pLTServer->SetObjectFlags(m_hObject, m_dwSaveFlags);

		if (m_hSurfaceObj)
		{
			dwFlags = g_pLTServer->GetObjectUserFlags(m_hSurfaceObj);
			g_pLTServer->SetObjectUserFlags(m_hSurfaceObj, dwFlags | USRFLG_VISIBLE);
		}

		m_bHidden = LTFALSE;
	}
	else if (!m_bHidden && (stricmp(pMsg, TRIGGER_MSG_OFF) == 0))
	{
		m_dwSaveFlags  = g_pLTServer->GetObjectFlags(m_hObject);
		uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags & ~USRFLG_VISIBLE);
		g_pLTServer->SetObjectFlags(m_hObject, 0);

		if (m_hSurfaceObj)
		{
			dwFlags = g_pLTServer->GetObjectUserFlags(m_hSurfaceObj);
			g_pLTServer->SetObjectUserFlags(m_hSurfaceObj, dwFlags & ~USRFLG_VISIBLE);
		}

		m_bHidden = LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void VolumeBrush::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char szData[MAX_CS_FILENAME_LEN+1];

	long nLongVal;
	LTBOOL bVal;


	LTBOOL bUseFogPerformance = LTFALSE;

	if (g_pServerDE->GetPropBool("UseFogPerformance", &bVal) == DE_OK)
	{
		bUseFogPerformance = bVal;
	}


	g_pLTServer->GetPropBool("Hidden", &m_bHidden);
	g_pLTServer->GetPropBool("ShowSurface", &m_bShowSurface);

	g_pLTServer->GetPropVector("TintColor", &m_vTintColor);
	g_pLTServer->GetPropVector("LightAdd", &m_vLightAdd);

	g_pLTServer->GetPropBool("FogEnable", &m_bFogEnable);
	g_pLTServer->GetPropReal("MinFogFarZ", &m_fMinFogFarZ);
	g_pLTServer->GetPropReal("MinFogNearZ", &m_fMinFogNearZ);
	g_pLTServer->GetPropReal("FogFarZ", &m_fFogFarZ);
	g_pLTServer->GetPropReal("FogNearZ", &m_fFogNearZ);
	g_pLTServer->GetPropVector("FogColor", &m_vFogColor);

	if(!bUseFogPerformance)
	{
		m_fMinFogFarZ = m_fFogFarZ;
		m_fMinFogNearZ = m_fFogNearZ;
	}

	if (g_pLTServer->GetPropLongInt("DamageType", &nLongVal) == DE_OK)
	{
		m_eDamageType = (DamageType)nLongVal;
	}

	g_pLTServer->GetPropBool("DamageCharactersOnly", &m_bDamageCharactersOnly);
	g_pLTServer->GetPropBool("DamagePlayersOnly", &m_bDamagePlayersOnly);

	// Polygrid stuff...

	if (g_pLTServer->GetPropString("SpriteSurfaceName", szData, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		m_hstrSurfaceSprite = g_pLTServer->CreateString( szData );
	}

	g_pLTServer->GetPropReal("XScaleMin", &m_fXScaleMin);
	g_pLTServer->GetPropReal("XScaleMax", &m_fXScaleMax);
	g_pLTServer->GetPropReal("YScaleMin", &m_fYScaleMin);
	g_pLTServer->GetPropReal("YScaleMax", &m_fYScaleMax);
	g_pLTServer->GetPropReal("XScaleDuration", &m_fXScaleDuration);
	g_pLTServer->GetPropReal("YScaleDuration", &m_fYScaleDuration);
	g_pLTServer->GetPropBool("PanWithCurrent", &m_bPanWithCurrent);
	g_pLTServer->GetPropReal("XPan", &m_fXPan);
	g_pLTServer->GetPropReal("YPan", &m_fYPan);
	g_pLTServer->GetPropReal("Frequency", &m_fFrequency);
	g_pLTServer->GetPropReal("SurfaceHeight", &m_fSurfaceHeight);
	g_pLTServer->GetPropColor("SurfaceColor1", &m_vSurfaceColor1);
	g_pLTServer->GetPropColor("SurfaceColor2", &m_vSurfaceColor2);
	g_pLTServer->GetPropReal("Viscosity", &m_fViscosity);
	g_pLTServer->GetPropReal("Friction", &m_fFriction);
	g_pLTServer->GetPropVector("Current", &m_vCurrent);
	g_pLTServer->GetPropReal("LiquidGravity", &m_fLiquidGravity);
	g_pLTServer->GetPropReal("Damage", &m_fDamage);
	g_pLTServer->GetPropReal("SurfaceAlpha", &m_fSurfAlpha);
	g_pLTServer->GetPropBool("Additive", &m_bAdditive);
	g_pLTServer->GetPropBool("Multiply", &m_bMultiply);

	if (g_pLTServer->GetPropLongInt("NumSurfacePolies", &nLongVal) == DE_OK)
	{
		m_dwNumSurfPolies = (uint32)nLongVal;
	}

	g_pLTServer->GetPropBool("StartOn", &m_bStartOn);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::PostPropRead
//
//	PURPOSE:	Set some final values.
//
// ----------------------------------------------------------------------- //

void VolumeBrush::PostPropRead(ObjectCreateStruct *pStruct) 
{
	if (!pStruct) return;

	pStruct->m_Flags |= m_dwFlags;
	pStruct->m_ObjectType = OT_CONTAINER;
	SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
	pStruct->m_ContainerCode = (D_WORD)m_eContainerCode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void VolumeBrush::InitialUpdate()
{
	// Tell the client about any special fx (fog)...

	CreateSpecialFXMsg();

	
	// Save volume brush's initial flags...

	m_dwSaveFlags = g_pLTServer->GetObjectFlags(m_hObject);


	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUserFlags |= USRFLG_IGNORE_PROJECTILES;
	if (!m_bHidden) dwUserFlags |= USRFLG_VISIBLE;

	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);


	// Create the surface if necessary.  We only need to do updates if we have
	// a surface (in case somebody decides to move the brush, we need to update
	// the surface's position)...

	if (m_bShowSurface) 
	{
		CreateSurface();
		g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);
	}

	g_pLTServer->SetDeactivationTime(m_hObject, UPDATE_DELTA + 1.0f);


	// Normalize friction (1 = normal, 0 = no friction, 2 = double)...

	if (m_fFriction < 0.0) m_fFriction = 0.0f;
	else if (m_fFriction > 1.0) m_fFriction = 1.0f;


	// Normalize viscosity (1 = no movement, 0 = full movement)...

	if (m_fViscosity < 0.0) m_fViscosity = 0.0f;
	else if (m_fViscosity > 1.0) m_fViscosity = 1.0f;


	// Check to see if we should start on or not
	HSTRING hMsg = g_pLTServer->CreateString(m_bStartOn ? const_cast<char *>(TRIGGER_MSG_ON) : const_cast<char *>(TRIGGER_MSG_OFF));
	HandleTrigger(LTNULL, hMsg);
	g_pLTServer->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CreateSpecialFXMsg()
//
//	PURPOSE:	Create the special fx message
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CreateSpecialFXMsg()
{
	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	WriteSFXMsg(hMessage);
	g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::WriteSFXMsg()
//
//	PURPOSE:	Write the volume brush specific sfx
//
// ----------------------------------------------------------------------- //

void VolumeBrush::WriteSFXMsg(HMESSAGEWRITE hMessage)
{
	if (!hMessage) return;

	// See if this type of volume should ignore the light add and tint
	LTVector vTintColor(255.0f, 255.0f, 255.0f);
	LTVector vLightAdd(0.0f, 0.0f, 0.0f);
	LTBOOL bIgnoreTints = LTFALSE;

	switch(m_eContainerCode)
	{
		case CC_NO_CONTAINER:
		case CC_ICE:
		case CC_POISON_GAS:
		case CC_ELECTRICITY:
		case CC_ENDLESS_FALL:
		case CC_WIND:
		case CC_COLDAIR:
		case CC_LADDER:
		case CC_EVACZONE:
			bIgnoreTints = LTTRUE;
	}


	g_pLTServer->WriteToMessageByte(hMessage, m_nSfxMsgId);

	if(bIgnoreTints)
	{
		g_pLTServer->WriteToMessageByte(hMessage, (DBYTE)LTFALSE);
	}
	else
	{
		g_pLTServer->WriteToMessageByte(hMessage, (DBYTE)m_bFogEnable);
	}

	g_pLTServer->WriteToMessageFloat(hMessage, m_fMinFogFarZ);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fMinFogNearZ);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fFogFarZ);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fFogNearZ);
	g_pLTServer->WriteToMessageVector(hMessage, &m_vFogColor);

	if(bIgnoreTints)
	{
		g_pLTServer->WriteToMessageVector(hMessage, &vTintColor);
		g_pLTServer->WriteToMessageVector(hMessage, &vLightAdd);
	}
	else
	{
		g_pLTServer->WriteToMessageVector(hMessage, &m_vTintColor);
		g_pLTServer->WriteToMessageVector(hMessage, &m_vLightAdd);
	}

	g_pLTServer->WriteToMessageFloat(hMessage, m_fViscosity);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fFriction);
	g_pLTServer->WriteToMessageVector(hMessage, &m_vCurrent);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Update()
//
//	PURPOSE:	Update the brush
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Update()
{
	if (m_bHidden) return;

	// Only do updates if we have a surface...

	if (m_hSurfaceObj)
	{
		g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);
	}
	else
	{
		g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
		g_pLTServer->SetDeactivationTime(m_hObject, 0.001f);
		g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);
	}


	// See if we have moved...

	DVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (!Equal(m_vLastPos, vPos))
	{
		VEC_COPY(m_vLastPos, vPos);

		// Set the surface to its new position...

		DVector vDims;
		g_pLTServer->GetObjectDims(m_hObject, &vDims);

		vPos.y += vDims.y - (m_fSurfaceHeight/2.0f); 

		g_pLTServer->SetObjectPos(m_hSurfaceObj, &vPos);		
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CreateSurface()
//
//	PURPOSE:	Create the poly grid surface
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CreateSurface()
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-PolyGrid", g_pLTServer->GetObjectName(m_hObject) );
#endif

	DVector vPos, vDims, vScale;
	VEC_INIT(vScale);

	g_pLTServer->GetObjectDims(m_hObject, &vDims);
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	DRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	VEC_COPY(m_vLastPos, vPos);


	vPos.y += vDims.y - (m_fSurfaceHeight/2.0f); 
	VEC_COPY(theStruct.m_Pos, vPos);
	ROT_COPY(theStruct.m_Rotation, rRot);

	HCLASS hClass = g_pLTServer->GetClass("PolyGrid");

	PolyGrid* pSurface = LTNULL;

	if (hClass)
	{
		pSurface = (PolyGrid *)g_pLTServer->CreateObject(hClass, &theStruct);
	}

	if (pSurface)
	{
		m_hSurfaceObj = pSurface->m_hObject;
		vDims.y		  = m_fSurfaceHeight;

		DFLOAT fXPan = m_bPanWithCurrent ? m_vCurrent.x : m_fXPan;
		DFLOAT fYPan = m_bPanWithCurrent ? m_vCurrent.z : m_fYPan;

		if (!m_bHidden)
		{
			g_pLTServer->SetObjectUserFlags(m_hSurfaceObj, USRFLG_VISIBLE);
		}

		pSurface->Setup(&vDims, &m_vSurfaceColor1, &m_vSurfaceColor2,
						m_hstrSurfaceSprite, m_fXScaleMin, m_fXScaleMax, 
						m_fYScaleMin, m_fYScaleMax, m_fXScaleDuration, 
						m_fYScaleDuration, fXPan, fYPan, m_fFrequency, m_fSurfAlpha,
						m_dwNumSurfPolies, m_bAdditive, m_bMultiply);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	if (m_bHidden || !pCPStruct || !pCPStruct->m_hObject) return;

	DFLOAT fUpdateDelta = g_pLTServer->GetFrameTime();


	// Character container physics is handled by the character movement class
	
	HCLASS hCharacter = g_pServerDE->GetClass("CCharacter");
	HCLASS hPlayer = g_pServerDE->GetClass("CPlayerObj");
	HCLASS hClass = g_pServerDE->GetObjectClass(pCPStruct->m_hObject);

	if (!g_pLTServer->IsKindOf(hClass, hCharacter))
	{
		// Dampen velocity based on the viscosity of the container...

		DVector vVel, vCurVel;
		vVel = vCurVel = pCPStruct->m_Velocity;

		if (m_fViscosity > 0.0f && VEC_MAG(vCurVel) > 1.0f)
		{
			DVector vDir;
			VEC_COPY(vDir, vCurVel);
			vDir.Norm();

			DFLOAT fAdjust = MAX_CONTAINER_VISCOSITY * m_fViscosity * fUpdateDelta;

			vVel = (vDir * fAdjust);

			if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
			{
				VEC_SUB(vVel, vCurVel, vVel);
			}
			else
			{
				VEC_INIT(vVel);
			}

			vVel += (m_vCurrent * fUpdateDelta);

			pCPStruct->m_Velocity = vVel;
		}

		
		// Do special liquid handling...

		if (IsLiquid(m_eContainerCode))
		{
			UpdateLiquidPhysics(pCPStruct);
		}
	}


	// Update damage...

	// Make damage relative to update delta...

	DFLOAT fDamage = 0.0f;
	if (m_fDamage > 0.0f) 
	{
		fDamage = m_fDamage * fUpdateDelta;
	}

	if (fDamage)
	{
		// Make sure we only continue to the damage stuff for the correct types of objects

		if(m_bDamageCharactersOnly && !g_pLTServer->IsKindOf(hClass, hCharacter))
			return;

		if(m_bDamagePlayersOnly && !g_pLTServer->IsKindOf(hClass, hPlayer))
			return;


		DamageStruct damage;

		damage.eType	= m_eDamageType;
		damage.fDamage	= fDamage;
		damage.hDamager = m_hObject;

		damage.DoDamage(this, pCPStruct->m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdateLiquidPhysics()
//
//	PURPOSE:	Update liquid specific physics of the passed in object
//				(really, under liquid physics)
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdateLiquidPhysics(ContainerPhysics* pCPStruct)
{
	if (!pCPStruct || !pCPStruct->m_hObject) return;

	// Apply liquid gravity to object...	
	
	if (pCPStruct->m_Flags & FLAG_GRAVITY)
	{
		pCPStruct->m_Flags &= ~FLAG_GRAVITY;
		pCPStruct->m_Acceleration.y += m_fLiquidGravity;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	hWrite->WriteHString(m_hstrSurfaceSprite);
	*hWrite << m_hSurfaceObj;

	*hWrite << m_vCurrent;
	*hWrite << m_vSurfaceColor1;
	*hWrite << m_vSurfaceColor2;
	*hWrite << m_vLastPos;
	*hWrite << m_vFogColor;
	*hWrite << m_vTintColor;
	*hWrite << m_vLightAdd;
	*hWrite << m_fViscosity;
	*hWrite << m_fFriction;
	*hWrite << m_fDamage;
	*hWrite << m_fSurfaceHeight;
	*hWrite << m_fLiquidGravity;
	*hWrite << m_fXScaleMin;
	*hWrite << m_fXScaleMax;
	*hWrite << m_fYScaleMin;
	*hWrite << m_fYScaleMax;
	*hWrite << m_fXScaleDuration;
	*hWrite << m_fYScaleDuration;
	*hWrite << m_bPanWithCurrent;
	*hWrite << m_fXPan;
	*hWrite << m_fYPan;
	*hWrite << m_fFrequency;
	*hWrite << m_fMinFogFarZ;
	*hWrite << m_fMinFogNearZ;
	*hWrite << m_fFogFarZ;
	*hWrite << m_fFogNearZ;
	*hWrite << m_fSurfAlpha;
	*hWrite << m_dwNumSurfPolies;
	*hWrite << m_dwFlags;
	*hWrite << m_dwSaveFlags;

	*hWrite << m_eDamageType;
	hWrite->WriteDWord(m_eContainerCode);
	*hWrite << m_bShowSurface;
	*hWrite << m_bFogEnable;
	*hWrite << m_bHidden;
	*hWrite << m_bAdditive;
	*hWrite << m_bMultiply;

	*hWrite << m_nSfxMsgId;

	*hWrite << m_bStartOn;
	*hWrite << m_bDamageCharactersOnly;
	*hWrite << m_bDamagePlayersOnly;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	m_hstrSurfaceSprite = hRead->ReadHString();
	*hRead >> m_hSurfaceObj;

	*hRead >> m_vCurrent;
	*hRead >> m_vSurfaceColor1;
	*hRead >> m_vSurfaceColor2;
	*hRead >> m_vLastPos;
	*hRead >> m_vFogColor;
	*hRead >> m_vTintColor;
	*hRead >> m_vLightAdd;
	*hRead >> m_fViscosity;
	*hRead >> m_fFriction;
	*hRead >> m_fDamage;
	*hRead >> m_fSurfaceHeight;
	*hRead >> m_fLiquidGravity;
	*hRead >> m_fXScaleMin;
	*hRead >> m_fXScaleMax;
	*hRead >> m_fYScaleMin;
	*hRead >> m_fYScaleMax;
	*hRead >> m_fXScaleDuration;
	*hRead >> m_fYScaleDuration;
	*hRead >> m_bPanWithCurrent;
	*hRead >> m_fXPan;
	*hRead >> m_fYPan;
	*hRead >> m_fFrequency;
	*hRead >> m_fMinFogFarZ;
	*hRead >> m_fMinFogNearZ;
	*hRead >> m_fFogFarZ;
	*hRead >> m_fFogNearZ;
	*hRead >> m_fSurfAlpha;
	*hRead >> m_dwNumSurfPolies;
	*hRead >> m_dwFlags;
	*hRead >> m_dwSaveFlags;

	*hRead >> m_eDamageType;
	m_eContainerCode = ContainerCode( hRead->ReadDWord() );
	*hRead >> m_bShowSurface;
	*hRead >> m_bFogEnable;
	*hRead >> m_bHidden;
	*hRead >> m_bAdditive;
	*hRead >> m_bMultiply;

	*hRead >> m_nSfxMsgId;

	*hRead >> m_bStartOn;
	*hRead >> m_bDamageCharactersOnly;
	*hRead >> m_bDamagePlayersOnly;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CacheFiles
//
//	PURPOSE:	Cache resources used by this the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CacheFiles()
{
	char* pFile = LTNULL;
	if (m_hstrSurfaceSprite)
	{
		pFile = g_pLTServer->GetStringData(m_hstrSurfaceSprite);
		if (pFile)
		{
			g_pLTServer->CacheFile(FT_SPRITE, pFile);
		}
	}
}
