// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileUtils.h
//
// PURPOSE : Profile utilities
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PROFILEUTILS_H
#define PROFILEUTILS_H

#include "ltbasetypes.h"
#include "NetDefs.h"
#include "TeamMgr.h"
#include <string>

class CButeMgr;

#define DEFAULT_CAMPAIGN	".default"
#define DEFAULT_CAMPAIGN_FILE	"default_campaign.txt"
#define PROFILE_DIR "Profiles\\"
#define PROFILE_EXT ".txt"

#define MAX_PROFILE_NAME	16

// Value passed back by server after initializing.  Only
// way to pass error back to client during startgame.
enum EServerStartResult
{
	eServerStartResult_None,
	eServerStartResult_Success,
	eServerStartResult_NetworkError,
	eServerStartResult_Failed,
};

class BaseGameOptions
{
	public:

		BaseGameOptions( );

		BaseGameOptions& Copy( BaseGameOptions const& other );

		bool LoadFromBute( CButeMgr& bute );
		bool SaveToBute( CButeMgr& bute );

	// Stuff to override.

		// Gets the tag to use when accessing bute.
		virtual char const* GetButeTagName( ) = 0;

		// Clears out variables to default state.
		virtual void Clear( );

		// for transmitting options that may change mid game
		virtual void Write( ILTMessage_Write *pMsg ) { };
		virtual void Read( ILTMessage_Read *pMsg ) { };

	// End stuff to override.

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		bool			m_bFriendlyFire;

		// Players can only pickup a weapon type once.  When they pickup,
		// the weaponitem stays in the world for another player to pickup.
		bool			m_bWeaponsStay;

		// Coop specific
		bool			m_bUseSkills;
		uint8			m_nDifficulty;
		float			m_fPlayerDiffFactor;

		// Adversarial specific
		uint8			m_nRunSpeed;
		uint8			m_nScoreLimit;
		uint8			m_nTimeLimit;
		uint8			m_nRounds;
		uint8			m_nFragScore;
		uint8			m_nRevivingScore;

		// Team specific
		uint8			m_nNumTeams;	// Will this ever be anything but 2?
		uint32			m_nTeamModel[MAX_TEAMS];
		std::string		m_sTeamName[MAX_TEAMS];

		// Doomsday specific.
		uint8			m_nDeviceCompletedScore;
		uint8			m_nLightPiecePlacedScore;
		uint8			m_nHeavyPiecePlacedScore;
		uint8			m_nPieceRemovedScore;
};

class SingleplayerGameOptions : public BaseGameOptions
{
	public:

		// Gets the tag to use when accessing bute.
		virtual char const* GetButeTagName( ) { return "Singleplayer"; }
};

class CoopGameOptions : public BaseGameOptions
{
	public:

		// Gets the tag to use when accessing bute.
		virtual char const* GetButeTagName( ) { return "Cooperative"; }

		// Clears out variables to default state.
		virtual void Clear( );
};

class DMGameOptions : public BaseGameOptions
{
	public:

		// Gets the tag to use when accessing bute.
		virtual char const* GetButeTagName( ) { return "Deathmatch"; }

		// Clears out variables to default state.
		virtual void Clear( );

		// for transmitting options that may change mid game
		virtual void Write( ILTMessage_Write *pMsg );
		virtual void Read( ILTMessage_Read *pMsg );
};

class TeamDMGameOptions : public BaseGameOptions
{
	public:

		// Gets the tag to use when accessing bute.
		virtual char const* GetButeTagName( ) { return "TeamDM"; }

		// Clears out variables to default state.
		virtual void Clear( );

		// for transmitting options that may change mid game
		virtual void Write( ILTMessage_Write *pMsg );
		virtual void Read( ILTMessage_Read *pMsg );
};

class DemolitionGameOptions : public BaseGameOptions
{
	public:

		// Gets the tag to use when accessing bute.
		virtual char const* GetButeTagName( ) { return "Demolition"; }

		// Clears out variables to default state.
		virtual void Clear( );

		// for transmitting options that may change mid game
		virtual void Write( ILTMessage_Write *pMsg );
		virtual void Read( ILTMessage_Read *pMsg );
};

class DoomsdayGameOptions : public BaseGameOptions
{
	public:

		// Gets the tag to use when accessing bute.
		virtual char const* GetButeTagName( ) { return "Doomsday"; }

		// Clears out variables to default state.
		virtual void Clear( );

		// for transmitting options that may change mid game
		virtual void Write( ILTMessage_Write *pMsg );
		virtual void Read( ILTMessage_Read *pMsg );
};

class ServerGameOptions
{
	public:

		ServerGameOptions( );

		ServerGameOptions( ServerGameOptions const& other )
		{
			Copy( other );
		}

		ServerGameOptions& operator=( ServerGameOptions const& other )
		{
			return Copy( other );
		}

		ServerGameOptions& Copy( ServerGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// Saved data: all game types
		GameType		m_eGameType;
		bool			m_bUsePassword;
		std::string		m_sPassword;
		std::string		m_sScmdPassword;
		bool			m_bAllowScmdCommands;
		uint16			m_nPort;
		uint8			m_nBandwidthServer;
		uint16			m_nBandwidthServerCustom;
		bool			m_bLANOnly;
		bool			m_bDedicated;
		StringSet		m_setRestrictedWeapons;
		StringSet		m_setRestrictedAmmo;
		StringSet		m_setRestrictedGear;
		std::string		m_sModName;

		//for the current game type
		bool		IsDefaultCampaign() const;
		const char*	GetCampaignName() const;
		void		SetCampaignName(const char*	pszName);
		const char*	GetSessionName() const;
		void		SetSessionName(const char*	pszName);

		// Check if the current gametype is using weaponsstay.
		bool		IsWeaponsStay( ) { return GetCurrentGameOptions( ).m_bWeaponsStay; }
		
		uint8		GetMaxPlayers() const;
		int			GetGameTypeStringID() const;

		// Unsaved data.
		std::string		m_sProfileName;
		bool			m_bPreCacheAssets;
		bool			m_bPerformanceTest;

		CoopGameOptions&		GetCoop() { return m_Coop;	}
		DMGameOptions&			GetDeathmatch() { return m_DM; }
		TeamDMGameOptions&		GetTeamDeathmatch() { return m_TeamDM; }
		DemolitionGameOptions&	GetDemolition() { return m_DE; }
		DoomsdayGameOptions&	GetDoomsday() { return m_DD; }

		// Gets the GameOptions for the current gametype.
		BaseGameOptions const&	GetCurrentGameOptions_const( ) const;
		BaseGameOptions&		GetCurrentGameOptions( ) 
		{ 
			BaseGameOptions const& bgo = GetCurrentGameOptions_const( );
			return const_cast< BaseGameOptions& >( bgo );
		}

		// Server fills in with start result.
		EServerStartResult m_eServerStartResult;

	private:

		//Saved data: cooperative
		SingleplayerGameOptions	m_Singleplayer;

		//Saved data: cooperative
		CoopGameOptions		m_Coop;

		//Saved data: deathmatch
		DMGameOptions		m_DM;

		//Saved data:  team deathmatch
		TeamDMGameOptions	m_TeamDM;
		
		//Saved data:  demolition
		DemolitionGameOptions	m_DE;
		
		//Saved data:  doomsday
		DoomsdayGameOptions	m_DD;

};



const char*	GetProfileDir( char const* pszProfileName );
const char*	GetProfileFile( char const* pszProfileName );
const char*	GetCampaignDir( char const* pszProfileName, GameType eGameType );
const char*	GetCampaignFile( ServerGameOptions const& serverGameOptions );

//Fix player name... may return a null string if no valid name is possible
const char* FixPlayerName(char const* pszPlayerName);


#endif // PROFILEUTILS_H

