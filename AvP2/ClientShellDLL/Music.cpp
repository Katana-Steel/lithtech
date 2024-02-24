// ----------------------------------------------------------------------- //
//
// MODULE  : Music.cpp
//
// PURPOSE : LithTech DirectMusic helper class.  Handles all music commands for game.
//
// CREATED : Apr-13-2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "iltdirectmusic.h"
#include "iltclient.h"
#include "ClientServerShared.h"
#include "MsgIDs.h"
#include "CommonUtilities.h"
#include "Music.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

#define MUSIC_MIN_VOL		-5000
#define MUSIC_MAX_VOL		0


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::CMusic
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CMusic::CMusic( )
{
    m_pClientDE = LTNULL;
	m_pMusicMgr = NULL;
	m_bMusicEnabled = TRUE;
	m_bLevelInitialized = FALSE;

	m_nMenuVolume	= 0;
	m_nTriggerVolume	= 0;
	m_bPlaying = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Init
//
//	PURPOSE:	Initialize the music
//
// ----------------------------------------------------------------------- //
LTBOOL CMusic::Init( ILTClient *pClientDE)
{
	// make sure we are not already initialized
    if (IsInitialized()) return LTTRUE;

	m_pClientDE = pClientDE;

	m_bMusicEnabled = TRUE;
	m_bLevelInitialized = FALSE;
	m_pMusicMgr = NULL;

	// get the musicenable console var
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("disablemusic");
	if (hVar)
	{
		// check if music is disabled
        if (((int)g_pLTClient->GetVarValueFloat(hVar)) != 0)
		{
			m_bMusicEnabled = FALSE;
		}
	}

	// if music is enabled
	if (MusicEnabled())
	{
		// get a pointer to the music mgr
        m_pMusicMgr = g_pLTClient->GetDirectMusicMgr();

		// make sure directmusic mgr was available
		if (m_pMusicMgr != NULL)
		{
			// initialize the music mgr
			m_pMusicMgr->Init();
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Term
//
//	PURPOSE:	Terminate the music
//
// ----------------------------------------------------------------------- //
void CMusic::Term( )
{
	if (m_pMusicMgr != NULL)
	{
		m_pMusicMgr->Term();
		m_pMusicMgr = NULL;
	}

	m_pClientDE = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::InitMusicLevel
//
//	PURPOSE:	Initialize Music for Current Game Level
//
// ----------------------------------------------------------------------- //

LTBOOL CMusic::InitLevel(char* pDirectory, char* pControlFile)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return LTFALSE;
	if (m_pMusicMgr == NULL) return LTFALSE;
	if (!pDirectory || !pControlFile) return LTFALSE;

	// Make sure we are not trying to initialize the level again
	//if (IsLevelInitialized()) return LTTRUE;
	if (IsLevelInitialized())
	{
		TermLevel();
	}

	strcpy(m_State.szDirectory, pDirectory);
	strcpy(m_State.szControlFile, pControlFile);

	// initialize the music mgr
	if (m_pMusicMgr->InitLevel(m_State.szDirectory, m_State.szControlFile) == LT_OK)
	{
		// set level initialized flag
		m_bLevelInitialized = TRUE;

		// set the initial volume
//		SetTriggerVolume(m_pMusicMgr->GetInitialVolume());
//		SetMenuVolume( 100 );

		CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
		SetMenuVolume(( long )pSettings->MusicVolume( ));

		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::TermMusicLevel
//
//	PURPOSE:	Terminate Music for Current Game Level
//
// ----------------------------------------------------------------------- //
void CMusic::TermLevel()
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	// make sure directmusic mgr was available
	if (m_pMusicMgr != NULL)
	{
		// initialize the music mgr
		m_pMusicMgr->TermLevel();
	}

	{ // BL 09/26/00
		// clear the state

		m_State.Clear();
	}

	// set level initialized flag
	m_bLevelInitialized = FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::ProcessMusicMessage
//
//	PURPOSE:	Process a music trigger message from the server.
//
// ----------------------------------------------------------------------- //

void CMusic::ProcessMusicMessage(HMESSAGEREAD hMessage)
{
	// Convert to LTMessage.
	ILTMessage* pMsg = ( ILTMessage* )hMessage;

	// get the data from the message
    char const* pszMessage = pMsg->ReadString( );

	// get the message string data
	ProcessMusicMessage( pszMessage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::ProcessMusicMessage
//
//	PURPOSE:	Process a music trigger message from the server.
//
// ----------------------------------------------------------------------- //

void CMusic::ProcessMusicMessage( char const* pMessage )
{
	// Make sure mgrs are initialized
	if (!pMessage || !pMessage[0]) return;

	// if the mgrs are not initialized, queue up the message
	// for processing when they are.
	if ( !IsInitialized() || !IsLevelInitialized() || m_pMusicMgr == NULL ) 
	{
		if( !m_strQueuedMessages.empty() )
		{
			m_strQueuedMessages += "; ";
		}

		m_strQueuedMessages += pMessage;

		return;
	}

    ILTCommon* pCommon = g_pLTClient->Common();
	ConParse parse;
	parse.Init(( char* )pMessage);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0)
		{
			if (parse.m_Args[0] != NULL)
			{
				// change intensity command
				if ((_stricmp(parse.m_Args[0], "intensity") == 0) ||
					(_stricmp(parse.m_Args[0], "i") == 0))
				{
					if (parse.m_nArgs > 1)
					{
						if (parse.m_Args[1] != NULL)
						{
							// convert intensity value to an int
							int nIntensity = atoi(parse.m_Args[1]);

							// figure out the enact time (optional parameter)
							LTDMEnactTypes nEnact = LTDMEnactInvalid;
							if (parse.m_nArgs > 2) nEnact = StringToEnactType(parse.m_Args[2]);

							// change the music intensity
							ChangeIntensity(nIntensity, nEnact);
						}
					}
				}
				// play secondary segment command
				else if ((_stricmp(parse.m_Args[0], "secondary") == 0) ||
					(_stricmp(parse.m_Args[0], "ps") == 0))

				{
					if (parse.m_nArgs > 1)
					{
						if (parse.m_Args[1] != NULL)
						{
							strcpy(m_State.szSecondary, parse.m_Args[0]);

							// figure out the enact time (optional parameter)
							m_State.nSecondaryEnact = LTDMEnactInvalid;
							if (parse.m_nArgs > 2) m_State.nSecondaryEnact = StringToEnactType(parse.m_Args[2]);

							// play a secondary segment
							m_pMusicMgr->PlaySecondary(m_State.szSecondary, m_State.nSecondaryEnact);
						}
					}
				}
				// play motif command
				else if ((_stricmp(parse.m_Args[0], "motif") == 0) ||
					(_stricmp(parse.m_Args[0], "pm") == 0))
				{
					if (parse.m_nArgs > 2)
					{
						if ((parse.m_Args[1] != NULL) && (parse.m_Args[2] != NULL))
						{
							strcpy(m_State.szMotifStyle, parse.m_Args[1]);
							strcpy(m_State.szMotifName, parse.m_Args[2]);

							// figure out the enact time (optional parameter)
							m_State.nMotifEnact = LTDMEnactInvalid;
							if (parse.m_nArgs > 3)
							{
								m_State.nMotifEnact = StringToEnactType(parse.m_Args[3]);
							}

							// play a motif
							if ((stricmp(m_State.szMotifStyle,"*") == 0))
							{
								m_pMusicMgr->PlayMotif(NULL, m_State.szMotifName, m_State.nMotifEnact);
							}
							else
							{
								m_pMusicMgr->PlayMotif(m_State.szMotifStyle, m_State.szMotifName, m_State.nMotifEnact);
							}
						}
					}
				}
				// set volume command
				else if ((_stricmp(parse.m_Args[0], "volume") == 0) ||
					(_stricmp(parse.m_Args[0], "v") == 0))
				{
					if (parse.m_nArgs > 1)
					{
						if (parse.m_Args[1] != NULL)
						{
							// set the new volume
							SetTriggerVolume(atol(parse.m_Args[1]));
						}
					}
				}
				// stop secondary segment command
				else if ((_stricmp(parse.m_Args[0], "stopsecondary") == 0) ||
					(_stricmp(parse.m_Args[0], "ss") == 0))

				{
					if (parse.m_nArgs > 1)
					{
						if (parse.m_Args[1] != NULL)
						{
							// No secondary...
							m_State.szSecondary[0] = '\0';

							// figure out the enact time (optional parameter)
							LTDMEnactTypes nEnact = LTDMEnactInvalid;
							if (parse.m_nArgs > 2) nEnact = StringToEnactType(parse.m_Args[2]);

							// stop a secondary segment if name is * stop all
							if ((stricmp(parse.m_Args[1],"*") == 0)) m_pMusicMgr->StopSecondary(NULL, nEnact);
							else m_pMusicMgr->StopSecondary(parse.m_Args[1], nEnact);
						}
					}
				}
				// stop motif command
				else if ((_stricmp(parse.m_Args[0], "stopmotif") == 0) ||
					(_stricmp(parse.m_Args[0], "sm") == 0))
				{
					if (parse.m_nArgs > 2)
					{
						if ((parse.m_Args[1] != NULL) && (parse.m_Args[2] != NULL))
						{
							// No motifs...
							m_State.szMotifStyle[0] = '\0';
							m_State.szMotifName[0]  = '\0';

							// figure out the enact time (optional parameter)
							LTDMEnactTypes nEnact = LTDMEnactInvalid;
							if (parse.m_nArgs > 3) nEnact = StringToEnactType(parse.m_Args[3]);

							// if name or style are * pass in NULL for all/any
							if ((stricmp(parse.m_Args[1],"*") == 0))
							{
								if ((stricmp(parse.m_Args[2],"*") == 0)) m_pMusicMgr->StopMotif(NULL, NULL, nEnact);
								else m_pMusicMgr->StopMotif(NULL, parse.m_Args[2], nEnact);
							}
							else
							{
								if ((stricmp(parse.m_Args[2],"*") == 0)) m_pMusicMgr->StopMotif(parse.m_Args[1], NULL, nEnact);
								else m_pMusicMgr->StopMotif(parse.m_Args[1], parse.m_Args[2], nEnact);
							}
						}
					}
				}
				// play command
				else if ((_stricmp(parse.m_Args[0], "play") == 0) ||
					(_stricmp(parse.m_Args[0], "p") == 0))

				{
					// begin playing music
					Play( );
				}
				// stop command
				else if ((_stricmp(parse.m_Args[0], "stop") == 0) ||
					(_stricmp(parse.m_Args[0], "s") == 0))

				{
					// enact time to stop at
					LTDMEnactTypes nEnact = LTDMEnactInvalid;

					// see if there is another parameter for the enact time
					if (parse.m_nArgs > 1)
					{
						if (parse.m_Args[1] != NULL)
						{
							// figure out the enact time (optional parameter)
							if (parse.m_nArgs > 2) nEnact = StringToEnactType(parse.m_Args[2]);
						}
					}

					// stop music from playing
					Stop(nEnact);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::StringToEnactType
//
//	PURPOSE:	Converts a string to an enact type for LithTech DirectMusic.
//
// ----------------------------------------------------------------------- //
LTDMEnactTypes CMusic::StringToEnactType(const char* sName)
{
	if (sName == NULL) return LTDMEnactInvalid;
	if (stricmp(sName, "Invalid") == 0) return LTDMEnactInvalid;
	if (stricmp(sName, "Default") == 0) return LTDMEnactDefault;
	if (stricmp(sName, "Immediatly") == 0) return LTDMEnactImmediatly;
	if (stricmp(sName, "Immediate") == 0) return LTDMEnactImmediatly;
	if (stricmp(sName, "NextBeat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "NextMeasure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "NextGrid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "NextSegment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "Beat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "Measure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "Grid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "Segment") == 0) return LTDMEnactNextSegment;
	return LTDMEnactInvalid;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Play
//
//	PURPOSE:	Begin playing music
//
// ----------------------------------------------------------------------- //
// Begin playing music
void CMusic::Play(int32 nIntensity /* = -1 */, int32 nStart /* = LTDMEnactDefault */)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	_ASSERT( m_strQueuedMessages.empty() );

	if ( !m_bPlaying )
	{
		m_pMusicMgr->Play();
	}

	m_State.bPlaying = LTTRUE;
	m_bPlaying = LTTRUE;

	if ( nIntensity == -1 )
	{
		m_pMusicMgr->ChangeIntensity(m_State.nIntensity, m_State.nIntensityEnact);
	}
	else
	{
		m_pMusicMgr->ChangeIntensity(nIntensity, (LTDMEnactTypes)nStart);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Stop
//
//	PURPOSE:	Stop playing music
//
// ----------------------------------------------------------------------- //
void CMusic::Stop(const LTDMEnactTypes nStart)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	if (!m_bPlaying) return;
	_ASSERT( m_strQueuedMessages.empty() );

	// Stop playing music
	m_pMusicMgr->Stop(nStart);
    m_State.bPlaying = LTFALSE;

	m_bPlaying = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Pause
//
//	PURPOSE:	Pause playing music
//
// ----------------------------------------------------------------------- //
void CMusic::Pause(const LTDMEnactTypes nStart)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	_ASSERT( m_strQueuedMessages.empty() );

	// Pause music playing
	m_pMusicMgr->Pause();
    m_State.bPaused = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::UnPause
//
//	PURPOSE:	Un pause the music
//
// ----------------------------------------------------------------------- //
void CMusic::UnPause()
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	_ASSERT( m_strQueuedMessages.empty() );

	// UnPause music playing
	m_pMusicMgr->UnPause();
    m_State.bPaused = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::SetMenuVolume
//
//	PURPOSE:	Adjust music volume
//
// ----------------------------------------------------------------------- //
void CMusic::SetMenuVolume(const long nVolume)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	_ASSERT( m_strQueuedMessages.empty() );

	// Set current volume
	m_nMenuVolume = nVolume;

	// Add the volumes together to get the rendered volume.
	long nFinalVolume = m_nMenuVolume+m_nTriggerVolume;

	// Clamp the range.
	nFinalVolume = Clamp( nFinalVolume, ( long )0, ( long )100 );

	// DM takes volume in db.  Convert the linear 0-100 to log10 scale.
	float fVolumeLog = ( float )log10(( double )nFinalVolume + 1 ) / 2.0f;

	// Scale logrithmic value between min max db.
	long nScaledVolume = ( long )( fVolumeLog * ( MUSIC_MAX_VOL - MUSIC_MIN_VOL ) + MUSIC_MIN_VOL );
	m_pMusicMgr->SetVolume(nScaledVolume);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::SetTriggerVolume
//
//	PURPOSE:	Adjust music volume
//
// ----------------------------------------------------------------------- //
void CMusic::SetTriggerVolume(const long nVolume)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	_ASSERT( m_strQueuedMessages.empty() );

	// Set current volume
	m_nTriggerVolume = nVolume;

	// Add the volumes together to get the rendered volume.
	long nFinalVolume = m_nMenuVolume+m_nTriggerVolume;

	// Clamp the range.
	nFinalVolume = Clamp( nFinalVolume, ( long )0, ( long )100 );

	// DM takes volume in db.  Convert the linear 0-100 to log10 scale.
	float fVolumeLog = ( float )log10(( double )nFinalVolume + 1 ) / 2.0f;

	// Scale logrithmic value between min max db.
	long nScaledVolume = ( long )( fVolumeLog * ( MUSIC_MAX_VOL - MUSIC_MIN_VOL ) + MUSIC_MIN_VOL );
	m_pMusicMgr->SetVolume(nScaledVolume);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::ChangeIntensity
//
//	PURPOSE:	Set the music intensity
//
// ----------------------------------------------------------------------- //
void CMusic::ChangeIntensity(const int nIntensity, const LTDMEnactTypes nStart)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	_ASSERT( m_strQueuedMessages.empty() );

	// Just set our intensity back to whatever it was...

	Play(nIntensity, nStart);
/*
	m_pMusicMgr->ChangeIntensity(nIntensity, nStart);
*/
	m_State.nIntensity		= nIntensity;
	m_State.nIntensityEnact = nStart;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::RestoreMusicState
//
//	PURPOSE:	Restore the music to the given state
//
// ----------------------------------------------------------------------- //

LTBOOL CMusic::RestoreMusicState(const CMusicState & newState)
{
    if (!IsInitialized()) return LTFALSE;

	// See if the new state is valid...

	if (!strlen(newState.szDirectory) || !strlen(newState.szControlFile))
	{
		return LTFALSE;
	}

	// See if anything has changed...

	if ((stricmp(m_State.szDirectory, newState.szDirectory) != 0) ||
		(stricmp(m_State.szControlFile, newState.szControlFile) != 0))
	{
		// Initialize (or re-initialize) the level.
		if (!InitLevel((char*)newState.szDirectory, (char*)newState.szControlFile))
		{
            return LTFALSE;
		}

		// Process any queued messages while waiting for the above initialization.
		if( !m_strQueuedMessages.empty() )
		{
			ProcessMusicMessage( m_strQueuedMessages.c_str() );

			m_strQueuedMessages.clear();
		}

	}

    return LTTRUE;
}