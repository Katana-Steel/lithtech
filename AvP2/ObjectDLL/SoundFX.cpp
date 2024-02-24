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
#include "SoundFX.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "SoundTypes.h"

BEGIN_CLASS(SoundFX)
    ADD_BOOLPROP(StartOn, DTRUE)                
	ADD_STRINGPROP_FLAG(Sound, "", PF_FILENAME)
	ADD_LONGINTPROP(Priority, 0.0f)
	ADD_REALPROP_FLAG(OuterRadius, 500.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(InnerRadius, 100.0f, PF_RADIUS)
	ADD_LONGINTPROP(Volume, 100)
	ADD_REALPROP(PitchShift, 1.0f)
	ADD_BOOLPROP(Ambient, 1)
	ADD_BOOLPROP(Loop, 1)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(SoundFX, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::TriggerSound2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoundFX::SoundFX() : BaseClass(OT_NORMAL)
{
	m_bStartOn				= DTRUE;

	m_hstrSound				= DNULL;
	m_hsndSound				= DNULL;

	m_fOuterRadius			= 0.0f;
	m_fInnerRadius			= 0.0f;
	m_nVolume				= 0;
	m_fPitchShift			= 1.0f;
	m_bAmbient				= DFALSE;
	m_bLooping				= DTRUE;
	m_nPriority				= SOUNDPRIORITY_MISC_LOW;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::~SoundFX()	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoundFX::~SoundFX()
{
	if (m_hstrSound)
	{
		g_pServerDE->FreeString(m_hstrSound);
	}

	if (m_hsndSound)
	{
		g_pServerDE->KillSound(m_hsndSound);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SoundFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

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
			if (m_hsndSound)
			{
				g_pServerDE->KillSound(m_hsndSound);
				m_hsndSound = NULL;
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
//	ROUTINE:	SoundFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD SoundFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	SoundFX:HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //

void SoundFX::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hstr = g_pServerDE->ReadFromMessageHString(hRead);
	if (!hstr) return;

	char *pszMessage = g_pServerDE->GetStringData(hstr);
	if (!pszMessage || !pszMessage[0])
	{
		g_pServerDE->FreeString(hstr);
		return;
	}

    if (stricmp(pszMessage, "TOGGLE") == 0)
    {
		if (m_hsndSound)
		{
			if (m_bLooping)
			{
				g_pServerDE->KillSoundLoop(m_hsndSound);
			}
			else
			{
				g_pServerDE->KillSound(m_hsndSound);
			}

			m_hsndSound = NULL;
		}
		else
		{
			PlaySound( );
		}
    } 
    else if (stricmp(pszMessage, "ON") == 0 || stricmp(pszMessage, "TRIGGER") == 0)
    {
		PlaySound();
    }            
    else if (stricmp(pszMessage, "OFF") == 0)
    {
		if (m_hsndSound)
		{
			if (m_bLooping)
			{
				g_pServerDE->KillSoundLoop(m_hsndSound);
			}
			else
			{
				g_pServerDE->KillSound(m_hsndSound);
			}

			m_hsndSound = NULL;
		}
    }
    else if (stricmp(pszMessage, "KILLSOUND") == 0)
    {
		// Just nuke the sound, looping or otherwise...
		if (m_hsndSound)
		{
			g_pServerDE->KillSound(m_hsndSound);
			m_hsndSound = NULL;
		}
    }
	
	if (hstr)
	{
		g_pServerDE->FreeString(hstr);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL SoundFX::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bStartOn = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("Sound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			 m_hstrSound = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("Priority", &genProp) == LT_OK)
	{
		m_nPriority = (unsigned char) genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("OuterRadius", &genProp) == LT_OK)
	{
		m_fOuterRadius = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("InnerRadius", &genProp) == LT_OK)
	{
		m_fInnerRadius = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Volume", &genProp) == LT_OK)
	{
		m_nVolume = (DBYTE) genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("PitchShift", &genProp) == LT_OK)
	{
		m_fPitchShift = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Ambient", &genProp) == LT_OK)
	{
		m_bAmbient = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("Loop", &genProp) == LT_OK)
	{
		m_bLooping = genProp.m_Bool;
	}

	return DTRUE;
}

      
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SoundFX::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

    // Set the Update!

	pStruct->m_NextUpdate = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL SoundFX::InitialUpdate()
{
	if (m_bStartOn)
	{
		PlaySound();
	}

	g_pServerDE->SetNextUpdate(m_hObject, 0);
	g_pServerDE->SetDeactivationTime(m_hObject, 1.0f);
	g_pServerDE->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::PlaySound
//
//	PURPOSE:	Plays the requested sound file.
//
// ----------------------------------------------------------------------- //

void SoundFX::PlaySound()
{
	// Kill the current sound

	if (m_hsndSound) 
	{
		g_pServerDE->KillSound(m_hsndSound);
		m_hsndSound = DNULL;
	}

	char *pSoundFile = g_pServerDE->GetStringData(m_hstrSound);

	// Play the sound...

	if (pSoundFile)
	{
		PlaySoundInfo playSoundInfo;
		
		PLAYSOUNDINFO_INIT(playSoundInfo);
		
		playSoundInfo.m_dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;
		if (m_bLooping)
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
		}

		SAFE_STRCPY(playSoundInfo.m_szSoundName, pSoundFile);
		playSoundInfo.m_nPriority = m_nPriority;
		playSoundInfo.m_fOuterRadius = m_fOuterRadius;
		playSoundInfo.m_fInnerRadius = m_fInnerRadius;
		if (m_nVolume < 100)
		{
			playSoundInfo.m_nVolume = m_nVolume;
			playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
		}
		
		if (m_fPitchShift != 1.0f)
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_PITCH;
			playSoundInfo.m_fPitchShift = m_fPitchShift;
		}

		g_pServerDE->GetObjectPos(m_hObject, &playSoundInfo.m_vPosition);
		if (m_bAmbient)
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
		}
		else
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_3D;
		}

 		if (g_pServerDE->PlaySound(&playSoundInfo) == LT_OK)
		{
			m_hsndSound = playSoundInfo.m_hSound;
		}

		if (!m_bLooping && m_hsndSound)
		{
			DFLOAT fDuration;
			g_pServerDE->GetSoundDuration(m_hsndSound, &fDuration);
			g_pServerDE->SetNextUpdate(m_hObject, fDuration);
			g_pServerDE->SetDeactivationTime(m_hObject, fDuration + 1.0f);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SoundFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSound);

	g_pServerDE->WriteToMessageFloat(hWrite, m_fOuterRadius);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fInnerRadius);
	g_pServerDE->WriteToMessageByte(hWrite, m_nVolume);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fPitchShift);
	g_pServerDE->WriteToMessageByte(hWrite, m_bAmbient);
	g_pServerDE->WriteToMessageByte(hWrite, m_bLooping);
	g_pServerDE->WriteToMessageByte(hWrite, m_nPriority);
	g_pServerDE->WriteToMessageByte(hWrite, (m_hsndSound) ? 1 : 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SoundFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	m_hsndSound			= DNULL;
	m_hstrSound			= g_pServerDE->ReadFromMessageHString(hRead);
	
	m_fOuterRadius		= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fInnerRadius		= g_pServerDE->ReadFromMessageFloat(hRead);
	m_nVolume			= g_pServerDE->ReadFromMessageByte(hRead);
	m_fPitchShift		= g_pServerDE->ReadFromMessageFloat(hRead);
	m_bAmbient			= g_pServerDE->ReadFromMessageByte(hRead);
	m_bLooping			= g_pServerDE->ReadFromMessageByte(hRead);
	m_nPriority			= g_pServerDE->ReadFromMessageByte(hRead);

	if (g_pServerDE->ReadFromMessageByte(hRead))
	{
		PlaySound();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void SoundFX::CacheFiles()
{
	if (!(g_pServerDE->GetServerFlags() & SS_CACHING)) return;

	char* pFile = DNULL;

	if (m_hstrSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrSound);

		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}
}
