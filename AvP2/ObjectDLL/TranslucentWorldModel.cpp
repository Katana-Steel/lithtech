// ----------------------------------------------------------------------- //
//
// MODULE  : TranslucentWorldModel.cpp
//
// PURPOSE : TranslucentWorldModel implementation
//
// CREATED : 4/12/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TranslucentWorldModel.h"
#include "cpp_server_de.h"
#include "ObjectMsgs.h"
#include "ClientServerShared.h"
#include "SFXFuncs.h"

BEGIN_CLASS(TranslucentWorldModel)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, 0)
	ADD_REALPROP_FLAG(Mass, 30000.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_LENSFLARE_WORLDMODEL_PROPERTIES(PF_GROUP4)
	ADD_VISIBLE_FLAG(DTRUE, 0)
	ADD_SOLID_FLAG(DTRUE, 0)
	ADD_GRAVITY_FLAG(DFALSE, 0)
	ADD_CHROMAKEY_FLAG(DFALSE, 0)
	ADD_BOOLPROP(BlockLight, DFALSE) // Used by pre-processor
	ADD_BOOLPROP(BoxPhysics, DTRUE)
	ADD_BOOLPROP(FogDisable, FALSE)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_BOOLPROP(LensFlare, DFALSE)
    ADD_BOOLPROP(AICanShootThrough, LTFALSE)
    ADD_BOOLPROP(AICanSeeThrough, LTFALSE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(TranslucentWorldModel, GameBase, NULL, NULL, 0, CTranslucentWorldModelPlugin)

DRESULT	CTranslucentWorldModelPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	DDWORD* pcStrings, 
	const DDWORD cMaxStrings, 
	const DDWORD cMaxStringLength)
{
	if (m_DestructibleModelPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::TranslucentWorldModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

TranslucentWorldModel::TranslucentWorldModel() : GameBase(OT_WORLDMODEL)
{
	AddAggregate(&m_damage);

	m_bLensFlare	= DFALSE;
	m_fInitialAlpha = 1.0f;
	m_fFinalAlpha	= 1.0f;
	m_fChangeTime	= 0.0f;
	m_fStartTime	= 0.0f;

	m_bCanShootThrough	= LTFALSE;
	m_bCanSeeThrough	= LTFALSE;
	m_bFirstUpdate		= LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::~TranslucentWorldModel()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

TranslucentWorldModel::~TranslucentWorldModel()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD TranslucentWorldModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if(!m_bFirstUpdate)
				Update();
			else
				FirstUpdate();
		}
		break;

		case MID_PRECREATE:
		{
			DDWORD dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}

				pStruct->m_ObjectType = OT_WORLDMODEL;
				SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
				pStruct->m_SkinName[0] = '\0';
				pStruct->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD | FLAG_DONTFOLLOWSTANDING;

				// Don't go through world if gravity is set...

				if (pStruct->m_Flags & FLAG_GRAVITY)
				{
					pStruct->m_Flags &= ~FLAG_GOTHRUWORLD;
				}
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			m_damage.SetMass(INFINITE_MASS);
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


	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD TranslucentWorldModel::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			HandleTrigger(hSender, hMsg);
			g_pServerDE->FreeString(hMsg);
		}
		break;
	}
	
	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL TranslucentWorldModel::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp gProp;

	if (!pStruct) return DFALSE;

	if (g_pServerDE->GetPropGeneric("LensFlare", &gProp) == LT_OK)
	{
		m_bLensFlare = gProp.m_Bool;
	}

	if (m_bLensFlare)
	{
		::GetLensFlareProperties(m_LensInfo);
	}

	if (g_pServerDE->GetPropGeneric("Alpha", &gProp) == LT_OK)
	{
		m_fInitialAlpha = gProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("BoxPhysics", &gProp) == LT_OK)
	{
//		pStruct->m_Flags |= (gProp.m_Bool ? FLAG_BOXPHYSICS : 0);
	}

	if (g_pServerDE->GetPropGeneric("FogDisable", &gProp) == LT_OK)
	{
		pStruct->m_Flags |= (gProp.m_Bool ? FLAG_FOGDISABLE : 0);
	}

    if (g_pLTServer->GetPropGeneric("AICanShootThrough", &gProp) == LT_OK)
	{
		 m_bCanShootThrough = gProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("AICanSeeThrough", &gProp) == LT_OK)
	{
		 m_bCanSeeThrough = gProp.m_Bool;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::InitialUpdate()
{
	// Make sure this object uses server marks...

    uint32 dwUsrFlgs = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_MOVEABLE);


	// Set object translucency...

	DFLOAT r, g, b, a;
	g_pServerDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	g_pServerDE->SetObjectColor(m_hObject, r, g, b, m_fInitialAlpha);

	SetNextUpdate(0.01f);

	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);

	if (dwFlags & FLAG_VISIBLE) 
	{
		dwUsrFlags |= USRFLG_VISIBLE;
	}
	else 
	{
		dwUsrFlags &= ~USRFLG_VISIBLE;
	}
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	// If we are not solid, be sure we are not rayhit either...
	if(!(dwFlags & FLAG_SOLID))
	{
		dwFlags &= ~FLAG_RAYHIT;
	}
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	if (m_bLensFlare)
	{
		::BuildLensFlareSFXMessage(m_LensInfo, this);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::FirstUpdate()
//
//	PURPOSE:	Do first update...
//
// --------------------------------------------------------------------------- //

void TranslucentWorldModel::FirstUpdate()
{
	// If we are not solid, be sure we are not rayhit either...
	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if(!(dwFlags & FLAG_SOLID))
	{
		dwFlags &= ~FLAG_RAYHIT;
		g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
	}

    m_bFirstUpdate = LTFALSE;

	SetNextUpdate(0.0f);

	// Don't need to be active or even get autoactivated.
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::Update()
//
//	PURPOSE:	Update the model
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::Update()
{
	DFLOAT fAlpha;
	DVector vColor;
	g_pServerDE->GetObjectColor(m_hObject, &vColor.x, &vColor.y, &vColor.z, &fAlpha);

	// See if we are at the target alpha...

	if (fabs(fAlpha - m_fFinalAlpha) < 0.01f)
	{
		g_pServerDE->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, m_fFinalAlpha);
		SetNextUpdate(0.0f);
		// Don't need to be active or even get autoactivated.
		g_pLTServer->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
		return;
	}

	DFLOAT fTimeDelta = g_pServerDE->GetTime() - m_fStartTime;

	fAlpha = m_fInitialAlpha + (fTimeDelta * (m_fFinalAlpha - m_fInitialAlpha) / m_fChangeTime);
	fAlpha = fAlpha > 0.999f ? 1.0f : (fAlpha < 0.001f ? 0.0f : fAlpha);

	g_pServerDE->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, fAlpha);
	SetNextUpdate(0.001f);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::TriggerMsg()
//
//	PURPOSE:	Handler trigger messages
//
// --------------------------------------------------------------------------- //

void TranslucentWorldModel::HandleTrigger(HOBJECT hSender, HSTRING hMsg)
{
	if (!hMsg) return;

	char* pMsg = g_pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	CommonLT* pCommon = g_pServerDE->Common();
	if (!pCommon) return;

	ConParse parse;
	parse.Init(pMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "CHANGEALPHA") == 0)
			{
				if (parse.m_nArgs > 2 && parse.m_Args[1] && parse.m_Args[2])
				{
					m_fFinalAlpha	= (DFLOAT) atof(parse.m_Args[1]);
					m_fChangeTime	= (DFLOAT) atof(parse.m_Args[2]);

					DFLOAT r, g, b;
					g_pServerDE->GetObjectColor(m_hObject, &r, &g, &b, &m_fInitialAlpha);
				
					// See if we need to change the alpha...

					if (fabs(m_fInitialAlpha - m_fFinalAlpha) > 0.01f)
					{
						m_fStartTime = g_pServerDE->GetTime();
						SetNextUpdate(0.001f);

						// We need to be active now.
						g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);
					}
				}
			}
			else if (_stricmp(parse.m_Args[0], "ON") == 0)
			{
				DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
				g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_VISIBLE);

				//DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
				//g_pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
			}
			else if (_stricmp(parse.m_Args[0], "OFF") == 0)
			{
				DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
				g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);

				//DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
				//g_pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	g_pServerDE->WriteToMessageFloat(hWrite, m_fInitialAlpha);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fFinalAlpha);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fChangeTime);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fStartTime - g_pLTServer->GetTime());

	g_pLTServer->WriteToMessageByte(hWrite, m_bCanShootThrough);
	g_pLTServer->WriteToMessageByte(hWrite, m_bCanSeeThrough);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	m_fInitialAlpha	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fFinalAlpha	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fChangeTime	= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fStartTime	= g_pServerDE->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();

    m_bCanShootThrough     = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanSeeThrough       = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
}
