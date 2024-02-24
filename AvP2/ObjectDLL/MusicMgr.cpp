// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "MusicMgr.h"
#include "MsgIDs.h"
#include "GameServerShell.h"
#include "ServerSoundMgr.h"

CMusicMgr* g_pMusicMgr = LTNULL;
static CMusicMgr s_MusicMgr;

CMusicButeMgr* g_pMusicButeMgr = LTNULL;
static CMusicButeMgr s_MusicButeMgr;

#ifdef __LINUX
	#include "dirutils.h"
#endif


#ifndef _FINAL

namespace /*unnamed*/
{
	LTFLOAT ShowMusicLevel()
	{
		ASSERT( g_pLTServer );

		static CVarTrack vtShowMusic(g_pLTServer,"ShowMusic",0.0f);

		return vtShowMusic.GetFloat();
	}

	const char * k_aszMoodDescriptions[] =
	{
		"None", // eMoodNone
		"Ambient", // eMoodAmbient
		"Warning", // eMoodWarning
		"Hostile"  // eMoodHostile
	};


} //namespace /*unnamed*/

#endif

CMusicMgr::CMusicMgr()
{
	// Set the globally accessor to this object.
	g_pMusicMgr = this;

	// Start off with no mood at all.
	m_eDefaultMood = eMoodNone;
	m_eModifiedMood = eMoodNone;
	m_eLastMood = eMoodNone;

	// Initialize the frame counters.
	m_fModifiedMoodTime = 0;

	// Clear out the wavetrack info.
	m_hWaveTrack = NULL;
	m_nWaveTrackIntensity = -1;
	m_nWaveTrackFollowIntensity = -1;
	m_fWaveTrackFadeTime = 2.0f;
	m_bWaveTrackLock = FALSE;

	// Not initialized yet.
	m_bInitialized = LTFALSE;
}

CMusicMgr::~CMusicMgr()
{
	// Uninitialize data.
	Term();

	// Global accessor no longer valid.
	g_pMusicMgr = LTNULL;
}

BOOL CMusicMgr::Init( char const* pszSong )
{
	// Start fresh.
	Term( );
	_ASSERT( !m_bInitialized );

	// Initialize the music butes.
	if( g_pMusicButeMgr && !g_pMusicButeMgr->IsValid( ))
	{
		if( !g_pMusicButeMgr->Init( g_pLTServer ))
		{
			_ASSERT( FALSE );
			Term( );
			return FALSE;
		}
	}

	// Make sure the song tag exists.
	if( !g_pMusicButeMgr->GetButeMgr( )->Exist( pszSong ))
	{
		_ASSERT( FALSE );
		Term( );
		return FALSE;
	}

	// Set the song to use.
	m_sSong = pszSong;

	// Set the control file game variable.
	CString sControlFile = g_pMusicButeMgr->GetButeMgr( )->GetString( m_sSong, "ControlFile", "" );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}
	g_pLTServer->SetGameConVar( "MusicControlFile", ( char* )( char const* )sControlFile );

	// Initialize the timer.
	m_fModifiedMoodTime = 0;

	// We have successfully initialized.
	m_bInitialized = LTTRUE;


	// Pre-cache all the wave files listed in the wave tracks for this song.
	g_pMusicButeMgr->CacheWaveTracks(pszSong);

	return TRUE;
}

void CMusicMgr::Term()
{
	// If initialized, then we need to call
	// the full featured functions so that the client is informed.
	if( m_bInitialized && g_pMusicButeMgr )
	{
		// Tell client to go to no mood.
		SendMood( eMoodNone, "" );
	}

	// Clear the song.
	m_sSong = "";

	// Clear the mood info.
	m_eDefaultMood = eMoodNone;
	m_sDefaultMoodTrackset = "";
	m_eModifiedMood = eMoodNone;
	m_sModifiedMoodTrackset = "";

	// Clear the last information.
	m_eLastMood = eMoodNone;
	m_sLastTrackset = "";

	m_bInitialized = LTFALSE;
}

void CMusicMgr::Save( ILTMessage& msg )
{
	msg << m_sSong;
	msg << m_eDefaultMood;
	msg << m_sDefaultMoodTrackset;
}

void CMusicMgr::Load( ILTMessage& msg )
{
	CString sSong;
	EMood eDefaultMood;
	CString sDefaultMoodTrackset;

	// Load in the data.
 	msg >> sSong;
	msg >> eDefaultMood;
	msg >> sDefaultMoodTrackset;

	// Initialize the music.
	if( !Init( sSong ))
		return;

	// Setup our moods.
	SetDefaultMood( eDefaultMood, sDefaultMoodTrackset );
}

BOOL CMusicMgr::SetDefaultMood( EMood eMood, char const* pszTrackSet )
{
	// Make sure we've been initialized.
	if( !m_bInitialized )
	{
		return FALSE;
	}

	// Make sure there is a valid trackset specified.
	if( !pszTrackSet )
	{
		pszTrackSet = "Default";
	}

	// Check if the mood is the same.
	if( m_eDefaultMood == eMood && strcmp( pszTrackSet, m_sDefaultMoodTrackset ) == 0 )
	{
		return TRUE;
	}

	// Set the mood.
	m_eDefaultMood = eMood;

	// Set the trackset.
	m_sDefaultMoodTrackset = pszTrackSet;

#ifndef _FINAL
	if( ShowMusicLevel() > 1.0f )
	{
		g_pLTServer->CPrint( "%f Music : Set default mood to %s, \"%s\".", 
			g_pLTServer->GetTime(), 
			k_aszMoodDescriptions[m_eDefaultMood],
			pszTrackSet );
	}
#endif

	return TRUE;
}

void CMusicMgr::GetDefaultMood( EMood& eMood, char* pszTrackSet, int nSize )
{
	// Give them the mood.
	eMood = m_eDefaultMood;

	// Give them the trackset if they want.
	if( pszTrackSet )
	{
		strncpy( pszTrackSet, m_sDefaultMoodTrackset, nSize );
	}
}

BOOL CMusicMgr::SetModifiedMood( EMood eMood, char const* pszTrackSet )
{
	// Make sure we've been initialized.
	if( !m_bInitialized )
	{
		return FALSE;
	}

	// Make sure there is a valid trackset specified.
	if( !pszTrackSet )
	{
		pszTrackSet = "Default";
	}

	// Don't set a modified mood if it is less than the default mood.
	if( eMood <= m_eDefaultMood )
	{
		return FALSE;
	}

	// Check if the current mood overrides this new mood.  The current mood
	// must have a valid timer.
	float fCurrentTime = g_pLTServer->GetTime( );
	if( eMood < m_eModifiedMood && m_fModifiedMoodTime >= fCurrentTime )
	{
		return FALSE;
	}

	// Set the mood.
	m_eModifiedMood = eMood;

	// Set the trackset.
	m_sModifiedMoodTrackset = pszTrackSet;

	// Set our timer.  Give us at least 1 second with this modification, since
	// not all objects update every frame.
	m_fModifiedMoodTime = fCurrentTime + 1.0f;

#ifndef _FINAL
	if( ShowMusicLevel() > 1.0f )
	{
		g_pLTServer->CPrint( "%f Music : Modified mood to %s, \"%s\", until %f.", 
			g_pLTServer->GetTime(), 
			k_aszMoodDescriptions[m_eModifiedMood],
			pszTrackSet,
			m_fModifiedMoodTime );
	}
#endif

	return TRUE;
}

void CMusicMgr::GetModifiedMood( EMood& eMood, char* pszTrackSet, int nSize )
{
	// Give them the mood.
	eMood = m_eModifiedMood;

	// Give them the trackset if they want.
	if( pszTrackSet )
	{
		strncpy( pszTrackSet, m_sModifiedMoodTrackset, nSize );
	}
}

BOOL CMusicMgr::PlayEvent( char const* pszEventName )
{
	ILTMessage* pMsg = NULL;

	// Check inputs.
	if( !pszEventName || !m_bInitialized )
	{
		return FALSE;
	}

#ifndef _FINAL
	TRACE( "CMusicMgr::PlayEvent( \"%s\" )\n", pszEventName );

	if( ShowMusicLevel() > 0.0f )
	{
		g_pLTServer->CPrint( "%f Music : Playing event \"%s\".", 
			g_pLTServer->GetTime(), pszEventName );
	}
#endif

	// Get the Event tag from the song info.
	CString sEventTag = g_pMusicButeMgr->GetButeMgr( )->GetString( m_sSong, "Events" );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Get the motifs from the event.
	CString sMotifs = g_pMusicButeMgr->GetButeMgr( )->GetString( sEventTag, pszEventName );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Parse the motifs.
	ConParse parse;
	parse.Init(( char* )( char const* )sMotifs );
	if( g_pLTServer->Common( )->Parse( &parse ) != LT_OK )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Select a motifs from the selection of motifs.
	CString sMotif = parse.m_Args[GetRandom( 0, parse.m_nArgs - 1 )];

	// Check if the motif has a '.wav' extension, which means it's a wave file, rather
	// than a directmusic motif.
	char ext[_MAX_EXT];
	_splitpath( sMotif, NULL, NULL, NULL, ext );

	// Handle wave file case.
	if( stricmp( ext, ".wav" ) == 0 )
	{
		PlayWaveFile( sMotif, FALSE, FALSE );
	}
	// Handle directmusic motif case.
	else
	{
		// Create a message to send.
		if( g_pLTServer->Common( )->CreateMessage( pMsg ) != LT_OK || !pMsg )
		{
			_ASSERT( FALSE );
			return FALSE;
		}

		// Write out and send the message.
		CString sMessage;

#ifdef _WIN32
		sMessage.Format( "PM * %s", sMotif );
#else
		sMessage.Format( "PM * %s", sMotif.GetBuffer(0) );
#endif

		pMsg->WriteString(( char* )( char const* )sMessage );
		g_pLTServer->SendToClient( *pMsg, MID_MUSIC, NULL, MESSAGE_GUARANTEED | MESSAGE_NAGGLE );
		pMsg->Release( );
		pMsg = NULL;
	}

	return TRUE;
}

void CMusicMgr::Update( )
{
	// Check if we're initialized yet.
	if( !m_bInitialized )
		return;

	// Only update the mood if we haven't been wavetrack locked.
	if( !m_bWaveTrackLock )
	{
		// This will be the mood we send to the client.
		EMood eMood = eMoodNone;

		// This will be the trackset we send to the client.
		char const* pszTrackSet = "";

		// Check if the modified mood has been set in the last frame.
		float fCurrentTime = g_pLTServer->GetTime( );
		if( m_fModifiedMoodTime >= fCurrentTime )
		{
			// Make sure there's a mood set.
			if( m_eModifiedMood != eMoodNone )
			{
				// Set the mood.
				eMood = m_eModifiedMood;

				// Set the trackset.
				pszTrackSet = m_sModifiedMoodTrackset;
			}
		}
		// No current modified mood.
		else
		{
			// Clear out any old modified mood.
			m_eModifiedMood = eMoodNone;
		}

		// Check if there wasn't a modified mood, 
		// or if the modified mood is below our default mood.
		if( eMood == eMoodNone || m_eDefaultMood >= eMood )
		{
			// Check if the default mood has been set.
			if( m_eDefaultMood != eMoodNone )
			{
				// Set the mood.
				eMood = m_eDefaultMood;

				// Set the trackset.
				pszTrackSet = m_sDefaultMoodTrackset;
			}
		}

		// Check if the mood changed from the last mood we sent.
		if( eMood != m_eLastMood || ( pszTrackSet && m_sLastTrackset.Compare( pszTrackSet )))
		{
			// Send the mood.
			if( SendMood( eMood, pszTrackSet ))
			{
				// Record that we are sending this to the client.
				m_eLastMood = eMood;
				m_sLastTrackset = pszTrackSet;
			}
		}
	}

	// Poll our wavetrack.
	UpdateWaveTrack( );
}


void CMusicMgr::UpdateWaveTrack( )
{
	// Check if we're playing a wave.
	if( !m_hWaveTrack )
		return;

	// If there is no follow intensity, then the wave will
	// loop until the mood is changed.  Otherwise, we need to find
	// when the wave stops playing and then we need to switch intensities.
	if( m_nWaveTrackFollowIntensity < 0 )
		return;

	// Get the status of the wave.
	DBOOL bDone = FALSE;
	if( g_pLTServer->IsSoundDone( m_hWaveTrack, &bDone ) != LT_OK )
	{
		_ASSERT( FALSE );
		return;
	}

	// Check if it's not done playing.
	if( !bDone )
		return;

	// Kill the wavetrack.
	KillWaveTrack( );
	_ASSERT( !m_hWaveTrack );

	// Switch to the new intensity.
	SendIntensity( m_nWaveTrackFollowIntensity );
}


// Strings defining keys used to lookup mood tags in the SongTag.
static const char* s_pszMoodsTagKeys[] =
{
	"AmbientMoods", "WarningMoods", "HostileMoods"
};

BOOL CMusicMgr::SendMood( EMood eMood, char const* pszTrackSet )
{
	// Make sure we're initialized.
	if( !m_bInitialized )
	{
		return FALSE;
	}

	// Use "Default" trackset if not specified.
	if( !pszTrackSet || !pszTrackSet[0] )
		pszTrackSet = "Default";

	TRACE( "CMusicMgr::SendMood( %i, \"%s\" )\n", eMood, ( pszTrackSet ) ? pszTrackSet : "" );

#ifndef _FINAL
	if( ShowMusicLevel() > 0.0f )
	{
		g_pLTServer->CPrint( "%f Music : Setting mood %s, \"%s\"", 
			g_pLTServer->GetTime(), k_aszMoodDescriptions[eMood], ( pszTrackSet ) ? pszTrackSet : "" );
	}
#endif

	// If the mood is set to none, then send silence.
	if( eMood == eMoodNone )
	{
		// Get the silence intensity index.
		int nSilenceIntensity = g_pMusicButeMgr->GetButeMgr( )->GetInt( m_sSong, "SilenceIntensity", 0 );
		if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
		{
			_ASSERT( FALSE );
			return FALSE;
		}

		// Kill any wavetrack playing.
		KillWaveTrack( );

		// Set the intesity to silence.
		return SendIntensity( nSilenceIntensity );
	}

	// Get the mood tag from the song definition.
	CString sMoodTag = g_pMusicButeMgr->GetButeMgr( )->GetString( m_sSong, s_pszMoodsTagKeys[eMood - eMoodAmbient], "" );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Get the trackset from the mood tag.
	CString sTrackSet = g_pMusicButeMgr->GetButeMgr( )->GetString( sMoodTag, pszTrackSet, "" );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Parse the trackset.
	ConParse parse;
	parse.Init(( char* )( char const* )sTrackSet );
	if( g_pLTServer->Common( )->Parse( &parse ) != LT_OK )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Select a track randomly from the trackset.
	CString sTrack = parse.m_Args[GetRandom( 0, parse.m_nArgs - 1 )];

	// Check if the track is an intensity index or a wavetrack.
	if( isdigit( sTrack[0] ))
	{
		// Kill any wavetrack playing.
		KillWaveTrack( );

		// Set the intensity.
		return SendIntensity( atoi( sTrack ));
	}

	// The track is a wavetrack.  Get the tag for the wavetracks.
	CString sWaveTrackTag = g_pMusicButeMgr->GetButeMgr( )->GetString( m_sSong, "WaveTracks", "" );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Get the wavetrack info.
	CString sWaveTrack = g_pMusicButeMgr->GetButeMgr( )->GetString( sWaveTrackTag, sTrack, "" );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Set the wavetrack.
	return PlayWaveTrack( sWaveTrack );
}

BOOL CMusicMgr::PlayWaveTrack( char const* pszWaveTrack )
{
	// Check inputs.
	if( !pszWaveTrack )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	TRACE( "CMusicMgr::PlayWaveTrack( \"%s\" )\n", pszWaveTrack );

#ifndef _FINAL
	if( ShowMusicLevel() > 0.0f )
	{
		g_pLTServer->CPrint( "%f Music : Playing wave track \"%s\".", 
			g_pLTServer->GetTime(), pszWaveTrack );
	}
#endif
	// Kill any existing wavetrack.
	KillWaveTrack( );
	_ASSERT( !m_hWaveTrack );

	// Parse the wavetrack string.
	ConParse parse;
	parse.Init(( char* )( char const* )pszWaveTrack );
	if( g_pLTServer->Common( )->Parse( &parse ) != LT_OK )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Make sure the minimum number of parameters exists.
	if( parse.m_nArgs < 1 )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Uninitialize variables we will be looking for.
	m_nWaveTrackIntensity = -1;
	m_nWaveTrackFollowIntensity = -1;
	m_sWaveTrackEndEvent = "";
	m_fWaveTrackFadeTime = 2.0f;
	m_bWaveTrackLock = FALSE;

	int nArgIndex;
	for( nArgIndex = 1; nArgIndex < parse.m_nArgs; nArgIndex++ )
	{
		// Check for overlapping intensity
		if( stricmp( "-o", parse.m_Args[nArgIndex] ) == 0 )
		{
			// Make sure the intensity was specified.
			if( nArgIndex + 1 < parse.m_nArgs )
			{
				// Get the intensity to play during the wavetrack.
				m_nWaveTrackIntensity = atoi( parse.m_Args[nArgIndex + 1] );

				// Skip the intensity value.
				nArgIndex++;
			}
		}
		// Check for following intensity
		else if( stricmp( "-f", parse.m_Args[nArgIndex] ) == 0 )
		{
			// Make sure the intensity was specified.
			if( nArgIndex + 1 < parse.m_nArgs )
			{
				// Get the intensity to play after the wavetrack.
				m_nWaveTrackFollowIntensity = atoi( parse.m_Args[nArgIndex + 1] );

				// Skip the intensity value.
				nArgIndex++;
			}
		}
		// Check for start event.
		else if( stricmp( "-s", parse.m_Args[nArgIndex] ) == 0 )
		{
			// Make sure the event was specified.
			if( nArgIndex + 1 < parse.m_nArgs )
			{
				// Play the start event.
				PlayEvent( parse.m_Args[nArgIndex + 1] );

				// Skip the intensity value.
				nArgIndex++;
			}
		}
		// Check for end event.
		else if( stricmp( "-e", parse.m_Args[nArgIndex] ) == 0 )
		{
			// Make sure the event was specified.
			if( nArgIndex + 1 < parse.m_nArgs )
			{
				// Record the end event.
				m_sWaveTrackEndEvent = parse.m_Args[nArgIndex + 1];

				// Skip the intensity value.
				nArgIndex++;
			}
		}
		// Check for fade time.  When wave is killed it will be faded out
		// by this time.
		else if( stricmp( "-t", parse.m_Args[nArgIndex] ) == 0 )
		{
			// Make sure the fade time was specified.
			if( nArgIndex + 1 < parse.m_nArgs )
			{
				// Record the fade time.
				m_fWaveTrackFadeTime = ( float )atof( parse.m_Args[nArgIndex + 1] );

				// Skip the fadetime value.
				nArgIndex++;
			}
		}
		// Check for play lock.  Wave will play to the end if this is set.  Mood
		// changes will be delayed until the wave reaches the end.
		else if( stricmp( "-l", parse.m_Args[nArgIndex] ) == 0 )
		{
			m_bWaveTrackLock = TRUE;
		}
	}

	// Used to indicate we want the wave file to loop.
	BOOL bLoop = FALSE;
	
	// If there is a following intensity, don't loop.
	if( m_nWaveTrackFollowIntensity >= 0 )
	{
		// Don't loop.
		bLoop = FALSE;
	}
	// Since we're looping, set the looping flag and dissallow wavetrack locking.
	else
	{
		bLoop = TRUE;
		m_bWaveTrackLock = FALSE;
	}

	// Set the intensity during the wave.
	if( !SendIntensity( m_nWaveTrackIntensity ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Play the wave.
	m_hWaveTrack = PlayWaveFile( parse.m_Args[0], bLoop, TRUE );
	if( !m_hWaveTrack )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	return TRUE;
}

void CMusicMgr::KillWaveTrack( )
{
	// Check if there is a wavetrack.
	if( !m_hWaveTrack )
		return;

	TRACE( "CMusicMgr::KillWaveTrack( )\n" );

#ifndef _FINAL
	if( ShowMusicLevel() > 0.0f )
	{
		g_pLTServer->CPrint( "%f Music : Stopping wavetrack.", 
			g_pLTServer->GetTime() );
	}
#endif

	// Stop the wave.
	g_pLTServer->SoundMgr( )->KillSoundFade( m_hWaveTrack, m_fWaveTrackFadeTime );
	m_hWaveTrack = NULL;

	// If there is an end event, play it now.
	if( !m_sWaveTrackEndEvent.IsEmpty( ))
	{
		PlayEvent( m_sWaveTrackEndEvent );

		// Clear it out, since we just played it.
		m_sWaveTrackEndEvent = "";
	}

	// Clear the wavetrack lock so that new wavetracks can modify the music.
	m_bWaveTrackLock = FALSE;
}


BOOL CMusicMgr::SendIntensity( int nIntensity )
{
	TRACE( "CMusicMgr::SendIntensity( %i )\n", nIntensity );

#ifndef _FINAL
	if( ShowMusicLevel() > 0.0f )
	{
		g_pLTServer->CPrint( "%f Music : Setting intensity to %i.", 
			g_pLTServer->GetTime(), nIntensity );
	}
#endif

	// Create a message to send.
	ILTMessage* pMsg = NULL;
	if( g_pLTServer->Common( )->CreateMessage( pMsg ) != LT_OK || !pMsg )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Fill in the message and send it.
	CString sMessage;
	sMessage.Format( "I %i", nIntensity );
	pMsg->WriteString(( char* )( char const* )sMessage );
	g_pLTServer->SendToClient( *pMsg, MID_MUSIC, NULL, MESSAGE_GUARANTEED | MESSAGE_NAGGLE );
	pMsg->Release( );
	pMsg = NULL;

	return TRUE;
}


BOOL CMusicMgr::HandleStringCommand( char const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	TRACE( "CMusicMgr::HandleStringCommand( \"%s\" )\n", pszCommand );

#ifndef _FINAL
	if( ShowMusicLevel() > 0.0f )
	{
		g_pLTServer->CPrint( "%f Music : Handling string command \"%s\".", 
			g_pLTServer->GetTime(), pszCommand );
	}
#endif

	// Parse the input.
	ConParse parse;
	parse.Init(( char* )( char const* )pszCommand );
	if( g_pLTServer->Common( )->Parse( &parse ) != LT_OK )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Make sure we got the minimum number of tokens.
	if( parse.m_nArgs < 2 )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Make sure the first token is "Music".
	if( stricmp( "Music", parse.m_Args[0] ) != 0 )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	return HandleCommand( parse.m_Args + 1, parse.m_nArgs - 1 );
}


BOOL CMusicMgr::HandleCommand( char const* const * pszArgs, int nArgs )
{
	// Sanity checks.
	if( nArgs < 1 || !pszArgs || !pszArgs[0] )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Handle the modified mood command.
	if( stricmp( "ModifiedMood", pszArgs[0] ) == 0 )
	{
		if( nArgs >= 2 )
		{
			EMood eMood = ConvertStringToMood( pszArgs[1] );
			char const* pszTrackSet = ( nArgs == 3 ) ? pszArgs[2] : NULL;
			SetModifiedMood( eMood, pszTrackSet );
			return LTTRUE;
		}
	}
	// Handle the default mood command.
	else if( stricmp( "DefaultMood", pszArgs[0] ) == 0 )
	{
		if( nArgs >= 2 )
		{
			EMood eMood = ConvertStringToMood( pszArgs[1] );
			char const* pszTrackSet = ( nArgs == 3 ) ? pszArgs[2] : NULL;
			SetDefaultMood( eMood, pszTrackSet );
			return LTTRUE;
		}
	}
	// Handle the modified mood command.
	else if( stricmp( "PlayEvent", pszArgs[0] ) == 0 )
	{
		if( nArgs == 2 )
		{
			return PlayEvent( pszArgs[1] );
		}
	}

	// Invalid message.
	_ASSERT( FALSE );
	return FALSE;
}

CMusicMgr::EMood CMusicMgr::ConvertStringToMood( char const* pszMood )
{
	// Check inputs.
	if( !pszMood || !pszMood[0] )
	{
		_ASSERT( FALSE );
		return eMoodNone;
	}

	// Find the mood based on the string.
	if( stricmp( "None", pszMood ) == 0 )
		return eMoodNone;
	else if( stricmp( "Ambient", pszMood ) == 0 )
		return eMoodAmbient;
	else if( stricmp( "Warning", pszMood ) == 0 )
		return eMoodWarning;
	else if( stricmp( "Hostile", pszMood ) == 0 )
		return eMoodHostile;

	// None found.
	return eMoodNone;
}


HLTSOUND CMusicMgr::PlayWaveFile( char const* pszWaveFile, BOOL bLoop, BOOL bGetHandle )
{
	// Check if they want to loop the wave.
	uint32 nLoopFlag = ( bLoop ) ? PLAYSOUND_LOOP : 0;

	// Check if they need the handle back.
	uint32 nGetHandleFlags = ( bGetHandle ) ? ( PLAYSOUND_GETHANDLE | PLAYSOUND_TIME ) : 0;

	// Play the wave.
	HLTSOUND hWave = g_pServerSoundMgr->PlaySoundLocal( pszWaveFile, SOUNDPRIORITY_MISC_HIGH, 
		nLoopFlag | nGetHandleFlags, 100, 1.0f, USERSOUNDTYPE_MUSIC );

	return hWave;
}

bool CMusicButeMgr::GetMoodKeysCB( const char* pszKeyName, CButeMgr::CSymTabItem* pItem, void* pContext )
{
	// This will be the string we add to the list.
	std::string sEntry;

	// Get the trackset list we're adding to.
	TrackSetCBData* pTrackSetCBData = ( TrackSetCBData* )pContext;

	// Format the string to add to the list.  This will show the song, the mood and the trackset
	sEntry = pTrackSetCBData->m_pszSong;
	sEntry += " ";
	sEntry += pTrackSetCBData->m_pszMood;
	sEntry += " ";
	sEntry += pszKeyName;

	// Add it to the list.
	pTrackSetCBData->m_pTrackSetList->push_back( std::string( sEntry ));

	// Keep iterating.
	return true;
}

BOOL CMusicButeMgr::GetTrackSetsFromMood( char const* pszSongTag, char const* pszMood, TrackSetCBData& trackSetCBData )
{
	// Check inputs.
	if( !pszSongTag || !pszMood )
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Get the ambient mood tag.
	CString sMoodTag = GetButeMgr( )->GetString( pszSongTag, pszMood, "" );
	if( !GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return FALSE;
	}

	// Set the mood in the trackset callback datastructure.
	trackSetCBData.m_pszMood = sMoodTag;

	// Add the tracksets for this song's mood.
	GetButeMgr( )->GetKeys( sMoodTag, GetMoodKeysCB, &trackSetCBData );

	return TRUE;
}

bool CMusicButeMgr::CacheWaveTrackCB( const char* pszKeyName, CButeMgr::CSymTabItem* pItem, void* pContext )
{
	_ASSERT( pItem->SymType == CButeMgr::StringType );

	const char * szTrackData = *pItem->data.s;
	if( szTrackData && szTrackData[0] )
	{
		ConParse parse;
		parse.Init( const_cast<char*>(szTrackData) );
		if( g_pLTServer->Common( )->Parse( &parse ) != LT_OK )
		{
			_ASSERT( FALSE );

			// just keep iterating.
			return true;
		}

		// Make sure the minimum number of parameters exists.
		if( parse.m_nArgs < 1 )
		{
			_ASSERT( FALSE );

			// just keep iterating.
			return true;
		}

		g_pLTServer->CacheFile( FT_SOUND, parse.m_Args[0] );

	}

	// Keep iterating.
	return true;
}

void CMusicButeMgr::CacheWaveTracks(const char * pszSongTag )
{
	CString sWaveTrackTag = g_pMusicButeMgr->GetButeMgr( )->GetString( pszSongTag, "WaveTracks", "" );
	if( !g_pMusicButeMgr->GetButeMgr( )->Success( ))
	{
		_ASSERT( FALSE );
		return;
	}

	if( !sWaveTrackTag.IsEmpty() )
	{
		// Cache the wavetracks.
		GetButeMgr( )->GetKeys( sWaveTrackTag, CacheWaveTrackCB, LTNULL );
	}
}

BOOL CMusicButeMgr::GetTrackSetsListForUI( TrackSetList& lstTrackSets )
{
	// Initialize the list.
	lstTrackSets.clear( );

	int nSongIndex = 0;
	CString sSongIndexKey;
	CString sSongTag;
	CString sMoodTag;
	TrackSetCBData trackSetCBData;

	// Iterate through the songs listed in the "Songs" tag.
	while( 1 )
	{
		// Create the songindex key.  They are labeled sequencially as in Song0, Song1, etc.
		sSongIndexKey.Format( "Song%i", nSongIndex );

		// Increment the song index for the next iteration.
		nSongIndex++;

		// Get the song for this index key.
		sSongTag = GetButeMgr( )->GetString( "Songs", sSongIndexKey, "" );

		// If we didn't find the song, then there are no more in the list.
		if( !GetButeMgr( )->Success( ))
			break;

		// Set the song in the trackset callback datastructure.
		trackSetCBData.m_pszSong = sSongTag;
		trackSetCBData.m_pTrackSetList = &lstTrackSets;

		// Add the tracksets for the 3 moods.
		GetTrackSetsFromMood( sSongTag, "AmbientMoods", trackSetCBData );
		GetTrackSetsFromMood( sSongTag, "WarningMoods", trackSetCBData );
		GetTrackSetsFromMood( sSongTag, "HostileMoods", trackSetCBData );
	}	

	return TRUE;
}

