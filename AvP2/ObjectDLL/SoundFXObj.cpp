// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.cpp
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 09/11/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SoundFXObj.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ServerSoundMgr.h"

BEGIN_CLASS(SoundFXObj)

    ADD_BOOLPROP_FLAG_HELP(StartOn, LTTRUE, 0, "Should this object be on when created?")
	ADD_STRINGPROP_FLAG_HELP(Sound, "None", PF_STATICLIST, "The SoundFX from the bute file.")
	ADD_LONGINTPROP_FLAG_HELP(NumPlays, -1, 0, "The number of times this sound should play before removing this object. If this is -1, the sound will play infinite times.")
	ADD_REALPROP_FLAG_HELP(MinDelay, 1.0f, 0, "The minimum delay to wait until playing the sound again.")
	ADD_REALPROP_FLAG_HELP(MaxDelay, 1.0f, 0, "The maximum delay to wait until playing the sound again.")
	ADD_REALPROP_FLAG_HELP(InnerRadius, -1.0f, PF_RADIUS, "The inner radius of the sound. If this is -1, it will use the radius from the attributes.")
	ADD_REALPROP_FLAG_HELP(OuterRadius, -1.0f, PF_RADIUS, "The outer radius of the sound. If this is -1, it will use the radius from the attributes.")
	ADD_BOOLPROP_FLAG_HELP(WaitUntilFinished, LTTRUE, 0, "Waits until the current sound is done before playing another one.")

END_CLASS_DEFAULT_FLAGS_PLUGIN(SoundFXObj, GameBase, NULL, NULL, 0, CSoundFXObjPlugin)


#ifdef _DEBUG
//#define _SOUNDFXOBJ_DEBUG_
#endif


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::SoundFXObj()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoundFXObj::SoundFXObj() : GameBase(OT_NORMAL)
{
	m_bOn				= LTTRUE;
	m_nSoundFX			= -1;
	m_hsndSound			= LTNULL;
	m_nNumPlays			= -1;
	m_fMinDelay			= 0.0f;
	m_fMaxDelay			= 0.0f;
	m_fInnerRadius		= 0.0f;
	m_fOuterRadius		= 0.0f;
	m_bWait				= LTTRUE;
	m_fCurDelay			= 0.0f;
	m_fLastOnUpdate		= 0.0f;
	m_fLastPlayTime		= 0.0f;
	m_fCurPlayLength	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::~SoundFXObj()	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoundFXObj::~SoundFXObj()
{
	if(m_hsndSound)
	{
		g_pLTServer->KillSound(m_hsndSound);
		m_hsndSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SoundFXObj::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

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

			CacheFiles();
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

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



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 SoundFXObj::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
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

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //

void SoundFXObj::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hstr = g_pLTServer->ReadFromMessageHString(hRead);
	if (!hstr) return;

	char *pszMessage = g_pLTServer->GetStringData(hstr);

	if(!pszMessage || !pszMessage[0])
	{
		g_pLTServer->FreeString(hstr);
		return;
	}

    if(stricmp(pszMessage, "TOGGLE") == 0)
    {
		m_bOn = !m_bOn;
    } 
    else if(stricmp(pszMessage, "ON") == 0 || stricmp(pszMessage, "TRIGGER") == 0)
    {
		m_bOn = LTTRUE;
    }            
    else if(stricmp(pszMessage, "OFF") == 0)
    {
		m_bOn = LTFALSE;
    }

	// Kill the sound if we need to
	if(!m_bOn && !m_bWait)
	{
		// Free the sound if it exists already
		if(m_hsndSound)
		{
			g_pLTServer->KillSound(m_hsndSound);
			m_hsndSound = LTNULL;
		}
	}

	if(hstr) g_pLTServer->FreeString(hstr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL SoundFXObj::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

	if(g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bOn = genProp.m_Bool;
	}

	if(g_pLTServer->GetPropGeneric("Sound", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
		{
			m_nSoundFX = g_pSoundButeMgr->GetSoundSetFromName(genProp.m_String);
		}
	}

	if(g_pLTServer->GetPropGeneric("NumPlays", &genProp) == LT_OK)
	{
		m_nNumPlays = genProp.m_Long;
	}

	if(g_pLTServer->GetPropGeneric("MinDelay", &genProp) == LT_OK)
	{
		m_fMinDelay = genProp.m_Float;
	}

	if(g_pLTServer->GetPropGeneric("MaxDelay", &genProp) == LT_OK)
	{
		m_fMaxDelay = genProp.m_Float;
	}

	if(g_pLTServer->GetPropGeneric("InnerRadius", &genProp) == LT_OK)
	{
		m_fInnerRadius = genProp.m_Float;
	}

	if(g_pLTServer->GetPropGeneric("OuterRadius", &genProp) == LT_OK)
	{
		m_fOuterRadius = genProp.m_Float;
	}

	if(g_pLTServer->GetPropGeneric("WaitUntilFinished", &genProp) == LT_OK)
	{
		m_bWait = genProp.m_Bool;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SoundFXObj::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

LTBOOL SoundFXObj::InitialUpdate()
{
	SetNextUpdate(0.01f);
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::Update()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

LTBOOL SoundFXObj::Update()
{
	// Get the current time
	LTFLOAT fTime = g_pLTServer->GetTime();


	// Handle being turned on... whoo hoo! Yeah baby!
	if(m_bOn)
	{
		// See if we need to play the sound
		if((fTime - m_fLastPlayTime) >= m_fCurDelay)
		{
			if(!m_bWait || !m_hsndSound)
			{
				// Check to see if we should go away
				if(m_nNumPlays != -1)
				{
					if(m_nNumPlays <= 0)
					{
#ifdef _SOUNDFXOBJ_DEBUG_
				g_pLTServer->CPrint("SoundFXObj %s: Number of plays expired! Removing object...", g_pLTServer->GetObjectName(m_hObject));
#endif
						g_pLTServer->RemoveObject(m_hObject);
						return LTFALSE;
					}

					m_nNumPlays--;
				}

#ifdef _SOUNDFXOBJ_DEBUG_
				g_pLTServer->CPrint("SoundFXObj %s: Playing sound after a delay of %f seconds!", g_pLTServer->GetObjectName(m_hObject), m_fCurDelay);
#endif

				// Reset our times
				m_fLastPlayTime = fTime;
				m_fCurDelay = GetRandom(m_fMinDelay, m_fMaxDelay);

				// Free the sound if it exists already
				if(m_hsndSound)
				{
					g_pLTServer->KillSound(m_hsndSound);
					m_hsndSound = LTNULL;
				}

				// Play the sound
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				m_hsndSound = g_pServerSoundMgr->PlaySoundFromPos(vPos, m_nSoundFX, m_fInnerRadius, m_fOuterRadius);

				// Get the length of the sound
				if(m_hsndSound)
					g_pLTServer->GetSoundDuration(m_hsndSound, &m_fCurPlayLength);
				else
					m_fCurPlayLength = 0.0f;
			}
		}

		// Set our last update
		m_fLastOnUpdate = fTime;
	}


	// Check the sound
	if(m_hsndSound)
	{
		LTBOOL bDone;

		if(g_pLTServer->IsSoundDone(m_hsndSound, &bDone) != LT_OK)
		{
			if(!bDone && (m_fCurPlayLength > 0.0f))
				bDone = ((fTime - m_fLastPlayTime) >= m_fCurPlayLength);
		}

		if(bDone)
		{
			g_pLTServer->KillSound(m_hsndSound);
			m_hsndSound = LTNULL;
		}
	}


	// Make sure we get another update
	SetNextUpdate(0.01f);
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SoundFXObj::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if(!hWrite) return;

	g_pLTServer->WriteToMessageByte(hWrite, (uint8)m_bOn);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nSoundFX);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nNumPlays);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMinDelay);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxDelay);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fCurDelay);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLastOnUpdate - g_pLTServer->GetTime());
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLastPlayTime - g_pLTServer->GetTime());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SoundFXObj::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if(!hRead) return;

	m_hsndSound			= DNULL;

	m_bOn				= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
	m_nSoundFX			= g_pLTServer->ReadFromMessageDWord(hRead);
	m_nNumPlays			= (int)g_pLTServer->ReadFromMessageDWord(hRead);
	m_fMinDelay			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fMaxDelay			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fCurDelay			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fLastOnUpdate		= g_pLTServer->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();
	m_fLastPlayTime		= g_pLTServer->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFXObj::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void SoundFXObj::CacheFiles()
{
	if (!(g_pLTServer->GetServerFlags() & SS_CACHING)) return;

	g_pServerSoundMgr->CacheButeSounds(m_nSoundFX);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFXObjPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT	CSoundFXObjPlugin::PreHook_EditStringList(const char* szRezPath, 
												 const char* szPropName, 
												 char* const * aszStrings, 
												 uint32* pcStrings, 
												 const uint32 cMaxStrings, 
												 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if(_stricmp("Sound", szPropName) == 0)
	{
		m_SoundButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength, "BaseSFXObjSound");

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

