// ----------------------------------------------------------------------- //
//
// MODULE  : ShoulderCannon.cpp
//
// PURPOSE : Predator Shoulder Cannon object class implementation
//
// CREATED : 6/21/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PredShoulderCannon.h"
#include "GameServerShell.h"
#include "SFXMsgIds.h"

BEGIN_CLASS(ShoulderCannon)
	ADD_LENSFLARE_PROPERTIES(PF_GROUP1)
END_CLASS_DEFAULT_FLAGS(ShoulderCannon, GameBase, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShoulderCannon::ShoulderCannon()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ShoulderCannon::ShoulderCannon() : GameBase(OT_MODEL)
{ 
//	m_hShoulderCannonModel = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShoulderCannon::~ShoulderCannon()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ShoulderCannon::~ShoulderCannon() 
{/*
	if (m_hShoulderCannonModel)
	{
		// Remove the model...

		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hShoulderCannonModel, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hShoulderCannonModel);
		m_hShoulderCannonModel = DNULL;
	}*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShoulderCannon::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ShoulderCannon::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
//				DDWORD dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
//				dwFlags &= ~FLAG_VISIBLE;
//				g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

				CreateLensFlareEffect();
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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShoulderCannon::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ShoulderCannon::ReadProp(ObjectCreateStruct *pInfo)
{
	GetLensFlareProperties( m_lensProps );

#ifdef _WIN32
	m_lensProps.SetSpriteFile("Sprites\\sfx\\flares\\predflare01.spr");
#else
	m_lensProps.SetSpriteFile("Sprites/SFX/Flares/predflare01.spr");
#endif

	m_lensProps.bCreateSprite = LTTRUE;
//	m_lensProps.bUseObjectAngle = LTTRUE;
	m_lensProps.bBlindingFlare	= LTFALSE;
	m_lensProps.fMinSpriteScale = 0.0f;
	m_lensProps.fMaxSpriteScale = 0.175f;
	m_lensProps.fSpriteOffset = 5.0f;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShoulderCannon::CreateLensFlareEffect()
//
//	PURPOSE:	Create the lense flare effect
//
// ----------------------------------------------------------------------- //
	
void ShoulderCannon::CreateLensFlareEffect()
{
	HMESSAGEWRITE hMessage = g_pServerDE->StartSpecialEffectMessage(this);
	g_pServerDE->WriteToMessageByte(hMessage, SFX_LENSFLARE_ID);
	g_pServerDE->WriteToMessageByte(hMessage, m_lensProps.bInSkyBox);
	g_pServerDE->WriteToMessageByte(hMessage, m_lensProps.bCreateSprite);
	g_pServerDE->WriteToMessageByte(hMessage, m_lensProps.bSpriteOnly);
	g_pServerDE->WriteToMessageByte(hMessage, m_lensProps.bUseObjectAngle);
	g_pServerDE->WriteToMessageByte(hMessage, m_lensProps.bSpriteAdditive);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fSpriteOffset);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fMinAngle);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fMinSpriteAlpha);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fMaxSpriteAlpha);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fMinSpriteScale);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fMaxSpriteScale);
	g_pServerDE->WriteToMessageHString(hMessage, m_lensProps.hstrSpriteFile);
	g_pServerDE->WriteToMessageByte(hMessage, m_lensProps.bBlindingFlare);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fBlindObjectAngle);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fBlindCameraAngle);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fMinBlindScale);
	g_pServerDE->WriteToMessageFloat(hMessage, m_lensProps.fMaxBlindScale);
	g_pServerDE->EndMessage(hMessage);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShoulderCannon::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ShoulderCannon::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

//	g_pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hShoulderCannonModel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShoulderCannon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ShoulderCannon::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

//	g_pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hShoulderCannonModel);
}


