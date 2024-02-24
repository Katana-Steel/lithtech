	// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef MUSICMGR_H
#define MUSICMGR_H

#include "GameButeMgr.h"

extern class CMusicMgr* g_pMusicMgr;
extern class CMusicButeMgr* g_pMusicButeMgr;

class CMusicButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CMusicButeMgr( )
		{
			g_pMusicButeMgr = this;
			m_bInitted = FALSE;
		}

		~CMusicButeMgr( )
		{
			g_pMusicButeMgr = NULL;
		}

#ifdef _WIN32
        LTBOOL	Init(ILTCSBase *pInterface, const char* szAttributeFile = "Attributes\\Music.txt")
#else
        LTBOOL	Init(ILTCSBase *pInterface, const char* szAttributeFile = "Attributes/music.txt")
#endif
		{
			m_bInitted = Parse(pInterface, szAttributeFile);
			return m_bInitted;
		}

		// Check if object has been initialized yet.
		LTBOOL IsValid( ) { return m_bInitted; }

		CButeMgr* GetButeMgr() { return &m_buteMgr; }

		// Type used in GetTrackSetsListForUI
		typedef std::vector<std::string> TrackSetList;

		// Gets a list of tracksets available in the attribute file.
		BOOL GetTrackSetsListForUI( TrackSetList& lstTrackSets );

		// Cache the *.wav files in the song's list of wave tracks.
		void CacheWaveTracks(const char * pszSong);


	protected:

		// Data structure used in callback with GetTrackSetsListForUI
		struct TrackSetCBData
		{
			char const*					m_pszSong;
			char const*					m_pszMood;
			TrackSetList*				m_pTrackSetList;
		};

		// Callback function used with GetTrackSetsListForUI
		static bool	GetMoodKeysCB( const char* pszKeyName, CButeMgr::CSymTabItem* pItem, void* pContext );

		// Callback function used to cache wavetracks.
		static bool	CacheWaveTrackCB( const char* pszKeyName, CButeMgr::CSymTabItem* pItem, void* pContext );

		// Gets the tracksets for a particular song and a particular mood.
		BOOL GetTrackSetsFromMood( char const* pszSongTag, char const* pszMood, TrackSetCBData& trackSetCBData );

		BOOL m_bInitted;
};

class CMusicMgr
{
	public :

		enum EMood
		{
			eMoodNone,
			eMoodAmbient,
			eMoodWarning,
			eMoodHostile
		};

	public :

		CMusicMgr();
		virtual ~CMusicMgr();

		// Initializes music to a song.
		BOOL Init( char const* pszSong );

		// Disposes of music data.
		void Term();

		// Checks if object initialized.
		BOOL IsValid( ) { return m_bInitialized; }

		// Saves current music state.
		void Save( ILTMessage& msg );

		// Loads music state.
		void Load( ILTMessage& msg );

		// Updates music.
		void Update();

		// Set the default mood.
		BOOL SetDefaultMood( EMood eMood, char const* pszTrackSet = "Default" );

		// Get the current default mood.
		void GetDefaultMood( EMood& eMood, char* pszTrackSet = NULL, int nSize = 0 );

		// Set the modified mood.
		BOOL SetModifiedMood( EMood eMood, char const* pszTrackSet = "Default" );

		// Get the current default mood.
		void GetModifiedMood( EMood& eMood, char* pszTrackSet = NULL, int nSize = 0 );

		// Plays a named event.
		BOOL PlayEvent( char const* pszEventName );

		// Plays the playerdeath event.
		BOOL PlayPlayerDeathEvent( ) { return PlayEvent( "PlayerDeath" ); }

		// Plays the AI death event.
		BOOL PlayAIDeathEvent( ) { return PlayEvent( "AIDeath" ); }

		// Call functions based on string command
		BOOL HandleStringCommand( char const* pszCommand );

		// Call functions based on a parsed string command.
		BOOL HandleCommand( char const * const * pszTokens, int nArgs );

		// Converts strings to EMood enumeration.
		static EMood		ConvertStringToMood( char const* pszMood );

	protected :

		// Sends the mood to client.
		BOOL		SendMood( EMood eMood, char const* pszTrackSet );

		// Send silence intensity.
		BOOL		SendIntensity( int nIntensity );

		// Play a wavetrack.
		BOOL		PlayWaveTrack( char const* pszWaveTrack );

		// Kill any existing wavetrack.
		void		KillWaveTrack( );

		// Update any playing wavetrack.
		void		UpdateWaveTrack( );

		// Plays a wave locally.  Returns NULL on failure.
		HLTSOUND	PlayWaveFile( char const* pszWaveFile, BOOL bLoop, BOOL bGetHandle );

		// Set to true if successful Init() called.
		LTBOOL		m_bInitialized;

		// Song to use.
		CString		m_sSong;

		// Current default mood.
		EMood		m_eDefaultMood;

		// Current default mood trackset.
		CString		m_sDefaultMoodTrackset;

		// Current modified mood.
		EMood		m_eModifiedMood;

		// Current modified mood trackset.
		CString		m_sModifiedMoodTrackset;

		// Frame when modified mood last set.
		float		m_fModifiedMoodTime;

		// The last mood sent to the client.
		EMood		m_eLastMood;

		// The last trackset sent to the client.
		CString		m_sLastTrackset;

		// Wave playing for wavetrack.
		HLTSOUND	m_hWaveTrack;

		// Intensity to play during wavetrack.
		int			m_nWaveTrackIntensity;

		// Intensity to follow wavetrack.
		int			m_nWaveTrackFollowIntensity;

		// Play this event at the end of the wavetrack.
		CString		m_sWaveTrackEndEvent;

		// WaveTrack fade time.
		float		m_fWaveTrackFadeTime;

		// Wave will play to the end before switching moods.
		BOOL		m_bWaveTrackLock;
};

inline ILTMessage & operator<<(ILTMessage & out, CMusicMgr::EMood & mood)
{
	_ASSERT( mood < 256 );
	out.WriteByte( mood );
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CMusicMgr::EMood & mood)
{
	mood = CMusicMgr::EMood( in.ReadByte() );
	return in;
}

#endif // MUSICMGR_H
