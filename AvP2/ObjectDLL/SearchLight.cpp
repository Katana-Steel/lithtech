// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLight.cpp
//
// PURPOSE : Implementation of Search Light object.
//
// CREATED : 6/7/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SearchLight.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "SFXMsgIds.h"


// Defines

#ifdef _WIN32
	#define DEFAULT_FILENAME		"Props\\Models\\SpotLight.abc"
	#define DEFAULT_SKIN			"Props\\Skins\\SpotLight.dtx"
	#define DEFAULT_BROKE_FILENAME	"Props\\Models\\SpotLight.abc"
	#define DEFAULT_BROKE_SKIN		"Props\\Skins\\SpotLightBroke.dtx"
#else
	#define DEFAULT_FILENAME		"Props/Models/SpotLight.abc"
	#define DEFAULT_SKIN			"Props/Skins/SpotLight.dtx"
	#define DEFAULT_BROKE_FILENAME	"Props/Models/SpotLight.abc"
	#define DEFAULT_BROKE_SKIN		"Props/Skins/SpotLightBroke.dtx"
#endif

#define UPDATE_DELTA			0.001f

// ----------------------------------------------------------------------- //
//
//	CLASS:		SearchLight
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(SearchLight)
	ADD_STRINGPROP_FLAG(Filename, DEFAULT_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, DEFAULT_SKIN, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, DEFAULT_BROKE_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, DEFAULT_BROKE_SKIN, PF_FILENAME)
	ADD_BOOLPROP(StartOn, DTRUE)
	ADD_OBJECTPROP(Target, "")
	ADD_REALPROP(FOV, 20.0)
	ADD_REALPROP(Yaw1, -45.0)
	ADD_REALPROP(Yaw2, 45.0)
	ADD_REALPROP(YawTime, 5.0)
	ADD_REALPROP(Yaw1PauseTime, 0.0)
	ADD_REALPROP(Yaw2PauseTime, 0.0)

	PROP_DEFINEGROUP(SpecialFXStuff, PF_GROUP4)
		ADD_REALPROP_FLAG(BeamRadius, 25.0f, PF_GROUP4)
		ADD_REALPROP_FLAG(BeamAlpha, 0.6f, PF_GROUP4)
		ADD_REALPROP_FLAG(BeamRotationTime, 20.0f, PF_GROUP4)
		ADD_REALPROP_FLAG(LightRadius, 75.0f, PF_GROUP4)
		ADD_COLORPROP_FLAG(LightColor, 255, 255, 255, PF_GROUP4)
		ADD_BOOLPROP_FLAG(BeamAdditive, DTRUE, PF_GROUP4)
		ADD_LENSFLARE_PROPERTIES(PF_GROUP5)

#ifdef _WIN32
			ADD_STRINGPROP_FLAG(SpriteFile, "Spr\\SearchLightFlare.spr", PF_GROUP5 | PF_FILENAME)
#else
			ADD_STRINGPROP_FLAG(SpriteFile, "Spr/SearchLightFlare.spr", PF_GROUP5 | PF_FILENAME)
#endif

			ADD_REALPROP_FLAG(MinAngle, 90.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(SpriteOffset, 15.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(MinSpriteScale, 0.5f, PF_GROUP5)
			ADD_REALPROP_FLAG(MaxSpriteScale, 1.0f, PF_GROUP5)
			ADD_BOOLPROP_FLAG(BlindingFlare, DTRUE, PF_GROUP5)
			ADD_REALPROP_FLAG(BlindObjectAngle, 5.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(BlindCameraAngle, 90.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(MinBlindScale, 1.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(MaxBlindScale, 10.0f, PF_GROUP5)

END_CLASS_DEFAULT_FLAGS(SearchLight, CScanner, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::SearchLight()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SearchLight::SearchLight() : CScanner()
{
	m_ePreviousState	= eStatePausingAt1;
	m_eState			= eStatePausingAt1;

	m_hTargetObject		= DNULL;
	m_hstrTargetName	= DNULL;

	m_bFirstUpdate		= DTRUE;
	m_bOn				= DTRUE;

	m_fYaw				= 0.0f;
	m_fYaw1				= -45.0f;
	m_fYaw2				= +45.0f;
	m_fYawSpeed			= 5.0f;
	m_fYaw1PauseTime	= 0.0f;
	m_fYaw2PauseTime	= 0.0f;
	m_fYawPauseTimer	= 0.0f;

	m_fBeamLength		= 500.0f;
	m_fBeamRadius		= 30.0f;
	m_fBeamAlpha		= 0.4f;
	m_fBeamRotTime		= 20.0f;
	m_fLightRadius		= 75.0f;
	m_bBeamAdditive		= DTRUE;
	m_vLightColor.Init(255, 255, 255);

#ifdef _WIN32
	m_LensInfo.SetSpriteFile("Spr\\Spr0022.spr");
#else
	m_LensInfo.SetSpriteFile("Spr/Spr0022.spr");
#endif

	m_LensInfo.fMinAngle = 90.0f;
	m_LensInfo.fSpriteOffset = 15.0f;
	m_LensInfo.fMinSpriteAlpha = 0.0f;
	m_LensInfo.fMaxSpriteAlpha = 1.0f;
	m_LensInfo.bBlindingFlare = DTRUE;
	m_LensInfo.fBlindObjectAngle = 5.0f;
	m_LensInfo.fBlindCameraAngle = 90.0f;
	m_LensInfo.fMinBlindScale = 1.0f;
	m_LensInfo.fMaxBlindScale = 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::~SearchLight()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

SearchLight::~SearchLight()
{
	FREE_HSTRING(m_hstrTargetName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SearchLight::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			DDWORD dwRet = CScanner::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
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
				InitialUpdate();
			}
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hTargetObject)
				{
					m_hTargetObject = DNULL;
				}
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return CScanner::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD SearchLight::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			CommonLT* pCommon = g_pServerDE->Common();
			if (!pCommon) return 0;

			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : DNULL;
			if (!szMsg) return 0;

			ConParse parse;
			parse.Init(szMsg);

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], "OFF") == 0)
					{
						m_bOn = DFALSE;
						g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
					}
					else if (_stricmp(parse.m_Args[0], "ON") == 0)
					{
						m_bOn = DTRUE;
						g_pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
					}
				}
			}

			g_pServerDE->FreeString(hMsg);
		}
		break;

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

			DDWORD dwRet = CScanner::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if (m_damage.IsDead())
			{
				SetState(eStateDestroyed);
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return CScanner::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL SearchLight::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return DFALSE;

	GenericProp genProp;

	::GetLensFlareProperties(m_LensInfo);

	if (g_pServerDE->GetPropGeneric("VisualRange", &genProp) == DE_OK)
	{
		m_fBeamLength = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("StartOn", &genProp) == DE_OK)
	{
		m_bOn = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("Target", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrTargetName = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("Yaw1", &genProp) == DE_OK)
	{
		m_fYaw1 = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Yaw2", &genProp) == DE_OK)
	{
		m_fYaw2 = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("YawTime", &genProp) == DE_OK)
	{
		m_fYawSpeed = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Yaw1PauseTime", &genProp) == DE_OK)
	{
		m_fYaw1PauseTime = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Yaw2PauseTime", &genProp) == DE_OK)
	{
		m_fYaw2PauseTime = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("BeamRadius", &genProp) == DE_OK)
	{
		m_fBeamRadius = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("BeamAlpha", &genProp) == DE_OK)
	{
		m_fBeamAlpha = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("BeamRotationTime", &genProp) == DE_OK)
	{
		m_fBeamRotTime = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("LightRadius", &genProp) == DE_OK)
	{
		m_fLightRadius = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("LightColor", &genProp) == DE_OK)
	{
		m_vLightColor = genProp.m_Vec;
	}

	if (g_pServerDE->GetPropGeneric("BeamAdditive", &genProp) == DE_OK)
	{
		m_bBeamAdditive = genProp.m_Bool;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void SearchLight::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Convert all our stuff into radians

	m_fYaw1 = MATH_DEGREES_TO_RADIANS(m_fYaw1);
	m_fYaw2 = MATH_DEGREES_TO_RADIANS(m_fYaw2);

	// YawSpeed was actually specified as time, so make it a rate

	m_fYawSpeed = (m_fYaw2 - m_fYaw1)/m_fYawSpeed;

	// Adjust yaws based on initial pitch yaw roll

	m_fYaw1 += GetInitialPitchYawRoll().y;
	m_fYaw2 += GetInitialPitchYawRoll().y;

	m_fYaw = GetInitialPitchYawRoll().y;

	if (m_fYaw > m_fYaw1)
	{
		SetState(eStateTurningTo2);
	}
	else
	{
		SetState(eStatePausingAt1);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL SearchLight::InitialUpdate()
{
	g_pServerDE->SetNextUpdate(m_hObject, m_bOn ? UPDATE_DELTA : 0.0f);

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_VISIBLE);

	HMESSAGEWRITE hMessage = g_pServerDE->StartSpecialEffectMessage(this);
	g_pServerDE->WriteToMessageByte(hMessage, SFX_SEARCHLIGHT_ID);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fBeamLength);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fBeamRadius);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fBeamAlpha);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fBeamRotTime);
	g_pServerDE->WriteToMessageFloat(hMessage, m_fLightRadius);
	g_pServerDE->WriteToMessageByte(hMessage, m_bBeamAdditive);
	g_pServerDE->WriteToMessageVector(hMessage, &m_vLightColor);
	::AddLensFlareInfoToMessage(m_LensInfo, hMessage);
	g_pServerDE->EndMessage(hMessage);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL SearchLight::Update()
{
	if (m_bFirstUpdate)
	{
		FirstUpdate();
		m_bFirstUpdate = DFALSE;
	}

	UpdateRotation();
	
	if (!GetLastDetectedEnemy())
	{
		UpdateDetect();
	}
	
	g_pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::FirstUpdate()
//
//	PURPOSE:	First Update
//
// ----------------------------------------------------------------------- //

void SearchLight::FirstUpdate()
{
	if (m_hstrTargetName)
	{
		// Kai Martin - 10/26/99
		// replaced linked list of objects in favor of new
		// static array of objects. 
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

		g_pServerDE->FindNamedObjects(g_pServerDE->GetStringData(m_hstrTargetName),objArray);

		if(objArray.NumObjects())
		{
			m_hTargetObject = objArray.GetObject(0);
			g_pServerDE->CreateInterObjectLink(m_hObject, m_hTargetObject);
		}

		FREE_HSTRING(m_hstrTargetName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::UpdateDetect()
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

DBOOL SearchLight::UpdateDetect()
{
	if (CScanner::UpdateDetect())
	{
		SetState(eStateDetected);
		return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::UpdateRotation()
//
//	PURPOSE:	Handles updating the camera's rotation
//
// ----------------------------------------------------------------------- //

void SearchLight::UpdateRotation()
{
	// Follow our target object or our follow object if applicable...

	if (m_hTargetObject || GetLastDetectedEnemy())
	{
		HOBJECT hObj = GetLastDetectedEnemy() ? GetLastDetectedEnemy() : m_hTargetObject;

		DVector vTargetPos, vDir;
		g_pServerDE->GetObjectPos(hObj, &vTargetPos);
		vDir = vTargetPos - GetPos();

		if (vDir.MagSqr() < GetVisualRange())
		{
			DRotation rRot;
			g_pServerDE->AlignRotation(&rRot, &vDir, DNULL);
			g_pServerDE->SetObjectRotation(m_hObject, &rRot);
		}	
		else if (GetLastDetectedEnemy())
		{
			// Handle lost enemy...wait for a second, search a little, then
			// go back to normal scanning...

			// SetLastDetectedEnemy(DNULL);
		}

		return;
	}

	// If we've gotten to this point, we should just do normal (security
	// camera-like) scanning...

	if (m_eState == eStateTurningTo1 || m_eState == eStateTurningTo2)
	{
		DFLOAT fYaw = g_pLTServer->GetFrameTime()*m_fYawSpeed;

		if (m_eState == eStateTurningTo1)
		{
			fYaw = -fYaw;
			m_fYaw += fYaw;

			if (m_fYaw < m_fYaw1)
			{
				if (m_fYaw1PauseTime)
				{
					SetState(eStatePausingAt1);
				}
				else
				{
					SetState(eStateTurningTo2);
				}
			}
		}
		else 
		{
			m_fYaw += fYaw;

			if (m_fYaw > m_fYaw2)
			{
				if (m_fYaw2PauseTime)
				{
					SetState(eStatePausingAt2);
				}
				else
				{
					SetState(eStateTurningTo1);
				}
			}
		}

		DVector vU, vR, vF;
		DRotation rRot;
		g_pServerDE->GetObjectRotation(m_hObject, &rRot);
		g_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		g_pServerDE->RotateAroundAxis(&rRot, &vU, fYaw);
		g_pServerDE->SetObjectRotation(m_hObject, &rRot);
	}

	if (m_eState == eStatePausingAt1)
	{
		m_fYawPauseTimer += g_pLTServer->GetFrameTime();

		if (m_fYawPauseTimer > m_fYaw1PauseTime)
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo2);
		}
	}

	if (m_eState == eStatePausingAt2)
	{
		m_fYawPauseTimer += g_pLTServer->GetFrameTime();

		if (m_fYawPauseTimer > m_fYaw2PauseTime)
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo1);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::SetState
//
//	PURPOSE:	Change our current state
//
// ----------------------------------------------------------------------- //

void SearchLight::SetState(State eNewState)
{
	if (m_eState == eNewState) return;

	// Handle switching to the new state...
	
	switch (eNewState)
	{
		case eStateDestroyed:
		{
			SetDestroyedModel();
			DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
			g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);

			g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		}
		break;

		default : break;
	}

	m_ePreviousState = m_eState;
	m_eState = eNewState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SearchLight::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	g_pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hTargetObject);

	g_pServerDE->WriteToMessageDWord(hWrite, m_eState);
	g_pServerDE->WriteToMessageDWord(hWrite, m_ePreviousState);

	g_pServerDE->WriteToMessageByte(hWrite, m_bOn);
	g_pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);

	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw1);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw2);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYawSpeed);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw1PauseTime);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYaw2PauseTime);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fYawPauseTimer);

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrTargetName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SearchLight::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	g_pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hTargetObject);

	m_eState			= (State)g_pServerDE->ReadFromMessageDWord(hRead);
	m_ePreviousState	= (State)g_pServerDE->ReadFromMessageDWord(hRead);

	m_bOn				= (DBOOL)g_pServerDE->ReadFromMessageByte(hRead);
	m_bFirstUpdate		= (DBOOL)g_pServerDE->ReadFromMessageByte(hRead);

	m_fYaw				= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw1				= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw2				= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYawSpeed			= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw1PauseTime	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw2PauseTime	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fYawPauseTimer	= g_pServerDE->ReadFromMessageFloat(hRead);

	m_hstrTargetName	= g_pServerDE->ReadFromMessageHString(hRead);
}


