
#include "stdafx.h"
#include "TriggerSound.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(TriggerSound)
	ADD_STRINGPROP_FLAG(StartSound, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LoopSound, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(StopSound, "", PF_FILENAME)
	ADD_LONGINTPROP(Priority, 0 )
	ADD_REALPROP_FLAG(OuterRadius, 100.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(InnerRadius, 10.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(PitchShift, 0.0f, 0)
	ADD_LONGINTPROP(Volume, 100.0f)
	ADD_BOOLPROP(Reverb, 0)
	ADD_BOOLPROP(Ambient, 1)
	ADD_BOOLPROP(On, 0)
	ADD_BOOLPROP(OneTimeOnly, 0)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(TriggerSound, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::TriggerSound()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

TriggerSound::TriggerSound() : BaseClass(OT_NORMAL)
{
	m_hstrStartSoundFile	= DNULL;
	m_hstrLoopSoundFile		= DNULL;
	m_hstrStopSoundFile		= DNULL;
	m_nPriority				= 0;
	m_fOuterRadius			= 100.0f;
	m_fInnerRadius			= 10.0f;
	m_fPitchShift			= 0.0f;
	m_nVolume				= 100;
	m_bAmbient				= DTRUE;
	m_bReverb				= DTRUE;
	m_bOn					= DFALSE;
	m_bOneTimeOnly			= DFALSE;
	m_hStartSound			= DNULL;
	m_hLoopSound			= DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::~TriggerSound()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

TriggerSound::~TriggerSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrStartSoundFile)
	{
		pServerDE->FreeString(m_hstrStartSoundFile);
	}
	if (m_hstrLoopSoundFile)
	{
		pServerDE->FreeString(m_hstrLoopSoundFile);
	}
	if (m_hstrStopSoundFile)
	{
		pServerDE->FreeString(m_hstrStopSoundFile);
	}
	if (m_hStartSound)
	{
		pServerDE->KillSound(m_hStartSound);
	}
	if (m_hLoopSound)
	{
		pServerDE->KillSound(m_hLoopSound);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD TriggerSound::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if ( fData == PRECREATE_WORLDFILE )
			{
				ReadProp(( ObjectCreateStruct *)pData);
			}
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
//	ROUTINE:	TriggerSound::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void TriggerSound::ReadProp(ObjectCreateStruct *pStruct)
{
	char szData[MAX_CS_FILENAME_LEN+1];
	DDWORD dwVal;

	if (g_pServerDE->GetPropString("StartSound", szData, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		if (strlen(szData))
		{
			m_hstrStartSoundFile = g_pServerDE->CreateString( szData );
		}
	}

	if (g_pServerDE->GetPropString("LoopSound", szData, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		if (strlen(szData))
		{
			m_hstrLoopSoundFile = g_pServerDE->CreateString(szData);
		}
	}

	if (g_pServerDE->GetPropString("StopSound", szData, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		if (strlen(szData))
		{
			m_hstrStopSoundFile = g_pServerDE->CreateString(szData);
		}
	}

	if (g_pServerDE->GetPropLongInt("Priority", (long *)(&dwVal)) == DE_OK)
	{
		m_nPriority = (unsigned char) dwVal;
	}

	g_pServerDE->GetPropReal("OuterRadius", &m_fOuterRadius);
	g_pServerDE->GetPropReal("InnerRadius", &m_fInnerRadius);
	g_pServerDE->GetPropReal("PitchShift", &m_fPitchShift);
	
	if (g_pServerDE->GetPropLongInt("Volume", (long *)(&dwVal)) == DE_OK)
	{
		m_nVolume = (unsigned char) dwVal;
	}

	g_pServerDE->GetPropBool("Reverb", &m_bReverb);
	g_pServerDE->GetPropBool("Ambient", &m_bAmbient);
	g_pServerDE->GetPropBool("On", &m_bOn);
	g_pServerDE->GetPropBool("OneTimeOnly", &m_bOneTimeOnly);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //
void TriggerSound::InitialUpdate( )
{
	if (m_bOn)
	{
		HandleOn();
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //
void TriggerSound::Update( )
{
	DBOOL bDone;

	// No more updates...
	g_pServerDE->SetNextUpdate( m_hObject, 0.0f );

	// Only play the looping sound if we are playing the start sound...

	if (m_hStartSound)
	{
		// If we haven't started the loop sound, do it...

		if (!m_hLoopSound && m_hstrLoopSoundFile)
		{
			// Play the looping sound...

			m_hLoopSound = PlaySound(m_hstrLoopSoundFile, DTRUE, DTRUE, DFALSE);
		}

		bDone = DFALSE;
		g_pServerDE->IsSoundDone(m_hStartSound, &bDone);
		if (bDone)
		{
			g_pServerDE->KillSound(m_hStartSound);
			m_hStartSound = DNULL;

			// See if we should go away...

			if (m_bOneTimeOnly && !m_hLoopSound)
			{
				g_pServerDE->RemoveObject(m_hObject);
			}
		}
		else
		{
			// Keep checking for sound to be done...

			g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD TriggerSound::ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead )
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char *pMsg = g_pServerDE->GetStringData(hMsg);

			// Handle start message

			if ( stricmp( pMsg, "On" ) == 0 )
			{
				HandleOn();
			}
			// Handle the stop message...
			else if ( stricmp( pMsg, "Off" ) == 0 )
			{
				HandleOff();
			}

			g_pServerDE->FreeString(hMsg);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::HandleOn
//
//	PURPOSE:	Handle the On message
//
// ----------------------------------------------------------------------- //

void TriggerSound::HandleOn()
{
	// Only do something if the sound isn't already playing...

	if (!m_hStartSound && !m_hLoopSound)
	{
		// Try to play the start sound...

		if (m_hstrStartSoundFile)
		{
			m_hStartSound = PlaySound(m_hstrStartSoundFile, DFALSE, DTRUE, DTRUE);

			// Set update for the end of the start sound, so we can play the loop sound...

			DFLOAT fDuration = 0.0f;
			g_pServerDE->GetSoundDuration(m_hStartSound, &fDuration);
			
			DFLOAT fUpdateTime = fDuration;
			g_pServerDE->SetNextUpdate(m_hObject, fUpdateTime);
		}
		else if (m_hstrLoopSoundFile)
		{
			// The start sound didn't work, so just play the looping sound...
			
			m_hLoopSound = PlaySound(m_hstrLoopSoundFile, DTRUE, DTRUE, DFALSE);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::HandleOff
//
//	PURPOSE:	Handle the Off message
//
// ----------------------------------------------------------------------- //

void TriggerSound::HandleOff()
{
	// Only do something if we have had a start...

	if (m_hStartSound || m_hLoopSound)
	{
		// Start sound may still be playing...

		if (m_hStartSound)
		{
			g_pServerDE->KillSound(m_hStartSound);
			m_hStartSound = DNULL;
		}

		// We should always have a valid looping sound if we got this far...
		
		if (m_hLoopSound)
		{
			g_pServerDE->KillSound(m_hLoopSound);
			m_hLoopSound = DNULL;
		}

		// Play the stop sound...

		if (m_hstrStopSoundFile)
		{
			PlaySound(m_hstrStopSoundFile, DFALSE, DFALSE, DFALSE);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::PlaySound()
//
//	PURPOSE:	Play the sound
//
// ----------------------------------------------------------------------- //
HSOUNDDE TriggerSound::PlaySound( HSTRING hstrSoundFile, DBOOL bLoop, DBOOL bHandle, DBOOL bTime )
{
	// No file specified...

	if (!hstrSoundFile) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT(playSoundInfo);

	playSoundInfo.m_dwFlags = PLAYSOUND_ATTACHED;

	if (bLoop)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	}
	if (bHandle)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	}
	if (bTime)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_TIME | PLAYSOUND_TIMESYNC;
	}
	if (m_bReverb)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_REVERB;
	}

	strncpy(playSoundInfo.m_szSoundName, g_pServerDE->GetStringData(hstrSoundFile), sizeof(playSoundInfo.m_szSoundName) - 1);
	playSoundInfo.m_nPriority    = m_nPriority;
	playSoundInfo.m_fOuterRadius = m_fOuterRadius;
	playSoundInfo.m_fInnerRadius = m_fInnerRadius;

	if (m_fPitchShift > 0.0f)
	{
		playSoundInfo.m_fPitchShift = m_fPitchShift;
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_PITCH;
	}
		
	if (m_nVolume < 100)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
		playSoundInfo.m_nVolume = m_nVolume;
	}
	else
	{
		playSoundInfo.m_nVolume = 100;
	}

	playSoundInfo.m_hObject = m_hObject;
	
	if (m_bAmbient)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
	}
	else
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_3D;
	}

	g_pServerDE->PlaySound(&playSoundInfo);

	return playSoundInfo.m_hSound;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TriggerSound::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrStartSoundFile);
	pServerDE->WriteToMessageHString(hWrite, m_hstrLoopSoundFile);
	pServerDE->WriteToMessageHString(hWrite, m_hstrStopSoundFile);
	pServerDE->WriteToMessageFloat(hWrite, m_fOuterRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fInnerRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fPitchShift);
	pServerDE->WriteToMessageByte(hWrite, m_nVolume);
	pServerDE->WriteToMessageByte(hWrite, m_nPriority);
	pServerDE->WriteToMessageByte(hWrite, m_bReverb);
	pServerDE->WriteToMessageByte(hWrite, m_bAmbient);
	pServerDE->WriteToMessageByte(hWrite, m_bOn);

	// Just save whether these sounds were on or off...	

	pServerDE->WriteToMessageByte(hWrite, m_hStartSound ? DTRUE : DFALSE);
	pServerDE->WriteToMessageByte(hWrite, m_hLoopSound ? DTRUE : DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TriggerSound::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrStartSoundFile = pServerDE->ReadFromMessageHString(hRead);
	m_hstrLoopSoundFile  = pServerDE->ReadFromMessageHString(hRead);
	m_hstrStopSoundFile  = pServerDE->ReadFromMessageHString(hRead);
	m_fOuterRadius		 = pServerDE->ReadFromMessageFloat(hRead);
	m_fInnerRadius		 = pServerDE->ReadFromMessageFloat(hRead);
	m_fPitchShift		 = pServerDE->ReadFromMessageFloat(hRead);
	m_nVolume			 = pServerDE->ReadFromMessageByte(hRead);
	m_nPriority			 = pServerDE->ReadFromMessageByte(hRead);
	m_bReverb  			 = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bAmbient			 = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bOn				 = (DBOOL) pServerDE->ReadFromMessageByte(hRead);

	// See if our sounds were on or off...

	DBOOL bStatus = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	if (bStatus)
	{
		m_hStartSound = PlaySound(m_hstrStartSoundFile, DFALSE, DTRUE, DTRUE);
	}

	bStatus	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	if (bStatus)
	{
		m_hLoopSound = PlaySound(m_hstrLoopSoundFile, DTRUE, DTRUE, DFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::CacheFiles
//
//	PURPOSE:	Cache resources used by this the object
//
// ----------------------------------------------------------------------- //

void TriggerSound::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;
	if (m_hstrStartSoundFile)
	{
		pFile = pServerDE->GetStringData(m_hstrStartSoundFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrLoopSoundFile)
	{
		pFile = pServerDE->GetStringData(m_hstrLoopSoundFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrStopSoundFile)
	{
		pFile = pServerDE->GetStringData(m_hstrStopSoundFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}
